#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// --- VARIÁVEIS GERAIS ---
float tempoGlobal = 0.0f;
int multiplicadorTempo = 1;

// --- VARIÁVEIS DA CÂMERA E FOCO ---
char foco = '0'; // '0' = Sol/Geral, letras para os planetas
float camOffsetY = 150.0f;
float camOffsetZ = 250.0f;

// --- TAMANHOS E DISTÂNCIAS ESCALONADOS ---
// O Sol agora é GIGANTE, e os planetas gasosos são proporcionais à Terra
float raioSol = 15.0f;
float distMerc = 20.0f,    raioMerc = 0.2f;
float distVenus = 26.0f,   raioVenus = 0.38f;
float distEarth = 34.0f,   raioEarth = 0.4f;
float distMars = 42.0f,    raioMars = 0.21f;
float distJupiter = 70.0f, raioJupiter = 4.4f;
float distSaturn = 105.0f, raioSaturn = 3.6f;
float distUranus = 145.0f, raioUranus = 1.6f;
float distNeptune = 180.0f,raioNeptune = 1.5f;

GLuint texSun, texMercury, texVenus, texEarth, texMoon;
GLuint texMars, texJupiter, texSaturn, texSaturnRing, texUranus, texNeptune;

GLfloat luzBranca[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat luzAmbienteFraca[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLUquadric* quadrico; 

// --- CARREGADOR DE IMAGENS ---
GLuint loadBMP_custom(const char * imagepath) {
    unsigned char header[54];
    unsigned int dataPos, width, height, imageSize;
    unsigned char * data;

    FILE * file = fopen(imagepath, "rb");
    if (!file) return 0;
    if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M') { fclose(file); return 0; }

    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);

    if (imageSize == 0)    imageSize = width * height * 3;
    if (dataPos == 0)      dataPos = 54;

    data = new unsigned char[imageSize];
    fread(data, 1, imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return textureID;
}

float calculaAnguloPlaneta(float velocidade) {
    return fmod(tempoGlobal * velocidade, 360.0f);
}

// Converte o ângulo que a função acima gera para Radianos (necessário para trigonometria)
float getAnguloRad(float velocidade) {
    return calculaAnguloPlaneta(velocidade) * 3.14159265f / 180.0f;
}

void carregaTexturas() {
    texSun        = loadBMP_custom("2k_sun.bmp");
    texMercury    = loadBMP_custom("2k_mercury.bmp");
    texVenus      = loadBMP_custom("2k_venus_surface.bmp");
    texEarth      = loadBMP_custom("2k_earth_daymap.bmp");
    texMoon       = loadBMP_custom("2k_moon.bmp");
    texMars       = loadBMP_custom("2k_mars.bmp");
    texJupiter    = loadBMP_custom("2k_jupiter.bmp");
    texSaturn     = loadBMP_custom("2k_saturn.bmp");
    texSaturnRing = loadBMP_custom("2k_saturn_ring_alpha.bmp");
    texUranus     = loadBMP_custom("2k_uranus.bmp");
    texNeptune    = loadBMP_custom("2k_neptune.bmp");
}

void configuraLuz() {
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbienteFraca);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzBranca);
    GLfloat posPontual[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posPontual);
}

void desenhaPlanetaTexturizado(GLuint textura, float raio) {
    glBindTexture(GL_TEXTURE_2D, textura);
    glColor3f(1.0f, 1.0f, 1.0f); 
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluSphere(quadrico, raio, 32, 32);
    glPopMatrix();
}

void desenhaOrbita(float raio) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.2f, 0.2f, 0.2f); 
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / 100.0f;
        glVertex3f(raio * cos(theta), 0.0f, raio * sin(theta)); 
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// --- FUNÇÃO PARA DESENHAR TEXTOS NA TELA (HUD) ---
void renderTexto(float x, float y, const char* string) {
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void desenhaInterface() {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h); 
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Desativa luz e 3D para desenhar o texto 2D em cima de tudo
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glColor3f(0.0f, 1.0f, 0.0f); // Cor Verde estilo Terminal
    
    float startY = h - 20;
    renderTexto(10, startY, "[ CONTROLES DE CAMERA ]"); startY -= 20;
    glColor3f(1.0f, 1.0f, 1.0f); // Volta para branco
    renderTexto(10, startY, "0 - Voltar pro Sol (Geral)"); startY -= 15;
    renderTexto(10, startY, "ESPACO - Visao Panoramica (Topo)"); startY -= 25;
    
    glColor3f(0.0f, 1.0f, 1.0f);
    renderTexto(10, startY, "[ FOCAR EM PLANETAS ]"); startY -= 20;
    glColor3f(1.0f, 1.0f, 1.0f);
    renderTexto(10, startY, "M - Mercurio"); startY -= 15;
    renderTexto(10, startY, "V - Venus"); startY -= 15;
    renderTexto(10, startY, "T - Terra"); startY -= 15;
    renderTexto(10, startY, "R - Marte"); startY -= 15;
    renderTexto(10, startY, "J - Jupiter"); startY -= 15;
    renderTexto(10, startY, "S - Saturno"); startY -= 15;
    renderTexto(10, startY, "U - Urano"); startY -= 15;
    renderTexto(10, startY, "N - Netuno"); startY -= 25;

    glColor3f(1.0f, 1.0f, 0.0f);
    renderTexto(10, startY, "[ TEMPO & ZOOM ]"); startY -= 20;
    glColor3f(1.0f, 1.0f, 1.0f);
    renderTexto(10, startY, "Teclas 1 a 9 = Acelerar Orbitas"); startY -= 15;
    renderTexto(10, startY, "Teclas + / - = Zoom In / Zoom Out");

    // Restaura o estado 3D
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // LÓGICA MATEMÁTICA DE ACOMPANHAMENTO DE CÂMERA
    float alvoX = 0.0f, alvoZ = 0.0f;
    
    // Calcula a coordenada exata (X, Z) do planeta selecionado usando Seno e Cosseno
    switch(foco) {
        case 'm': alvoX = distMerc * cos(getAnguloRad(8.3f)); alvoZ = -distMerc * sin(getAnguloRad(8.3f)); break;
        case 'v': alvoX = distVenus * cos(getAnguloRad(3.2f)); alvoZ = -distVenus * sin(getAnguloRad(3.2f)); break;
        case 't': alvoX = distEarth * cos(getAnguloRad(2.0f)); alvoZ = -distEarth * sin(getAnguloRad(2.0f)); break;
        case 'r': alvoX = distMars * cos(getAnguloRad(1.0f)); alvoZ = -distMars * sin(getAnguloRad(1.0f)); break;
        case 'j': alvoX = distJupiter * cos(getAnguloRad(0.17f)); alvoZ = -distJupiter * sin(getAnguloRad(0.17f)); break;
        case 's': alvoX = distSaturn * cos(getAnguloRad(0.06f)); alvoZ = -distSaturn * sin(getAnguloRad(0.06f)); break;
        case 'u': alvoX = distUranus * cos(getAnguloRad(0.02f)); alvoZ = -distUranus * sin(getAnguloRad(0.02f)); break;
        case 'n': alvoX = distNeptune * cos(getAnguloRad(0.01f)); alvoZ = -distNeptune * sin(getAnguloRad(0.01f)); break;
    }

    // Posiciona a câmera usando a posição do planeta selecionado (alvoX, alvoZ) como base
    gluLookAt(alvoX, camOffsetY, alvoZ + camOffsetZ, // Onde o Olho da câmera está
              alvoX, 0.0, alvoZ,                     // Para onde a câmera está olhando
              0.0, 1.0, 0.0);
              
    configuraLuz();

    // Desenha Orbitas Centrais
    desenhaOrbita(distMerc);
    desenhaOrbita(distVenus);
    desenhaOrbita(distEarth);
    desenhaOrbita(distMars);
    desenhaOrbita(distJupiter);
    desenhaOrbita(distSaturn);
    desenhaOrbita(distUranus);
    desenhaOrbita(distNeptune);

    // 1. Sol Gigante
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(1.85f), 0.0f, 1.0f, 0.0f); 
    GLfloat luzAmarelaSol[] = { 1.0f, 1.0f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, luzAmarelaSol);
    desenhaPlanetaTexturizado(texSun, raioSol);
    glMaterialfv(GL_FRONT, GL_EMISSION, luzAmbienteFraca); 
    glPopMatrix();

    // 2. Mercúrio
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(8.3f), 0.0f, 1.0f, 0.0f); 
    glTranslatef(distMerc, 0.0f, 0.0f);
    glRotatef(calculaAnguloPlaneta(0.85f), 0.0f, 1.0f, 0.0f); 
    desenhaPlanetaTexturizado(texMercury, raioMerc);
    glPopMatrix();

    // 3. Vênus 
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(3.2f), 0.0f, 1.0f, 0.0f); 
    glTranslatef(distVenus, 0.0f, 0.0f);
    glRotatef(calculaAnguloPlaneta(-0.2f), 0.0f, 1.0f, 0.0f); 
    desenhaPlanetaTexturizado(texVenus, raioVenus);
    glPopMatrix();

    // 4. Terra
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(2.0f), 0.0f, 1.0f, 0.0f); 
    glTranslatef(distEarth, 0.0f, 0.0f);
        glPushMatrix();
        glRotatef(calculaAnguloPlaneta(50.0f), 0.0f, 1.0f, 0.0f); 
        desenhaPlanetaTexturizado(texEarth, raioEarth);
        glPopMatrix();
        
        // Lua
        glPushMatrix();
        glRotatef(calculaAnguloPlaneta(26.6f), 0.0f, 1.0f, 0.0f); 
        glTranslatef(0.8f, 0.0f, 0.0f);
        glRotatef(calculaAnguloPlaneta(1.83f), 0.0f, 1.0f, 0.0f); 
        desenhaPlanetaTexturizado(texMoon, 0.10f); // Lua um pouco menor
        glPopMatrix();
    glPopMatrix();

    // 5. Marte
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(1.0f), 0.0f, 1.0f, 0.0f);
    glTranslatef(distMars, 0.0f, 0.0f);
    glRotatef(calculaAnguloPlaneta(48.5f), 0.0f, 1.0f, 0.0f); 
    desenhaPlanetaTexturizado(texMars, raioMars);
    glPopMatrix();

    // 6. Júpiter GIGANTE
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.17f), 0.0f, 1.0f, 0.0f);
    glTranslatef(distJupiter, 0.0f, 0.0f);
    glRotatef(calculaAnguloPlaneta(121.9f), 0.0f, 1.0f, 0.0f);
    desenhaPlanetaTexturizado(texJupiter, raioJupiter);
    glPopMatrix();

    // 7. Saturno GIGANTE
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.06f), 0.0f, 1.0f, 0.0f);
    glTranslatef(distSaturn, 0.0f, 0.0f);
        
        glPushMatrix();
        glRotatef(calculaAnguloPlaneta(111.1f), 0.0f, 1.0f, 0.0f);
        desenhaPlanetaTexturizado(texSaturn, raioSaturn);
        glPopMatrix();
    
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, texSaturnRing);
        glRotatef(-70.0f, 1.0f, 0.0f, 0.0f); 
        glRotatef(calculaAnguloPlaneta(2.0f), 0.0f, 0.0f, 1.0f); 
        glColor3f(1.0f, 1.0f, 1.0f); 
        
        // Ajuste dos Anéis para o novo tamanho de Saturno
        gluDisk(quadrico, 4.2f, 5.2f, 32, 32); 
        gluDisk(quadrico, 5.5f, 7.2f, 32, 32); 
        gluDisk(quadrico, 7.7f, 9.0f, 32, 32); 
        
        glPopMatrix();
    glPopMatrix();

    // 8. Urano 
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.02f), 0.0f, 1.0f, 0.0f);
    glTranslatef(distUranus, 0.0f, 0.0f);
    glRotatef(calculaAnguloPlaneta(-69.4f), 0.0f, 1.0f, 0.0f);
    desenhaPlanetaTexturizado(texUranus, raioUranus);
    glPopMatrix();

    // 9. Netuno
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.01f), 0.0f, 1.0f, 0.0f);
    glTranslatef(distNeptune, 0.0f, 0.0f);
    glRotatef(calculaAnguloPlaneta(74.6f), 0.0f, 1.0f, 0.0f);
    desenhaPlanetaTexturizado(texNeptune, raioNeptune);
    glPopMatrix();

    // Renderiza a Legenda no final para sobrepor os planetas
    desenhaInterface();

    glutSwapBuffers();
}

// --- FUNÇÃO PARA GERENCIAR A MUDANÇA DE FOCO E OS CONTROLES ---
void focarPlaneta(char novoFoco, float zoomY, float zoomZ) {
    foco = novoFoco;
    camOffsetY = zoomY;
    camOffsetZ = zoomZ;
}

void teclado(unsigned char key, int x, int y) {
    // Teclas de Velocidade
    if (key >= '1' && key <= '9') {
        multiplicadorTempo = key - '0';
    }
    // Lógica de Zoom Suave (Multiplicando para acelerar o zoom em visões distantes)
    else if (key == '+' || key == '=') {
        camOffsetY *= 0.85f;
        camOffsetZ *= 0.85f;
    }
    else if (key == '-' || key == '_') {
        camOffsetY *= 1.15f;
        camOffsetZ *= 1.15f;
    }
    // Visões de Câmera e Foco (Usando minúsculas e maiúsculas)
    else if (key == '0') focarPlaneta('0', 150.0f, 250.0f); // Geral
    else if (key == ' ') focarPlaneta('0', 250.0f, 1.0f);   // Topo (Panorâmica)
    
    // Focos específicos - As distâncias de câmera variam pelo tamanho do planeta
    else if (key == 'm' || key == 'M') focarPlaneta('m', 1.0f, 2.0f);
    else if (key == 'v' || key == 'V') focarPlaneta('v', 1.5f, 3.0f);
    else if (key == 't' || key == 'T') focarPlaneta('t', 1.5f, 3.0f);
    else if (key == 'r' || key == 'R') focarPlaneta('r', 1.0f, 2.0f);
    else if (key == 'j' || key == 'J') focarPlaneta('j', 15.0f, 25.0f);
    else if (key == 's' || key == 'S') focarPlaneta('s', 15.0f, 25.0f);
    else if (key == 'u' || key == 'U') focarPlaneta('u', 5.0f, 12.0f);
    else if (key == 'n' || key == 'N') focarPlaneta('n', 5.0f, 12.0f);
}

void inicializa() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL); 
    glEnable(GL_TEXTURE_2D);
    
    quadrico = gluNewQuadric();
    gluQuadricTexture(quadrico, GL_TRUE); 
    gluQuadricNormals(quadrico, GLU_SMOOTH);
    
    carregaTexturas();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Aumentei absurdamente o campo de visão distante para comportar o sistema inteiro
    gluPerspective(60.0, (double)w / (double)h, 1.0, 5000.0);
    glMatrixMode(GL_MODELVIEW);
}

void atualizaAnimacao(int value) {
    tempoGlobal += 0.2f * multiplicadorTempo; 
    glutPostRedisplay();
    glutTimerFunc(16, atualizaAnimacao, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800); 
    glutCreateWindow("Sistema Solar Dinamico");

    inicializa();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutTimerFunc(16, atualizaAnimacao, 0);

    glutMainLoop();
    return 0;
}
