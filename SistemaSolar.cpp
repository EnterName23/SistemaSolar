#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Relógio Global
float tempoGlobal = 0.0f;

// Variáveis para armazenar os IDs das texturas
GLuint texSun, texMercury, texVenus, texEarth, texMoon;
GLuint texMars, texJupiter, texSaturn, texSaturnRing, texUranus, texNeptune;

GLfloat luzBranca[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat luzAmbienteFraca[] = { 0.2f, 0.2f, 0.2f, 1.0f };
// Objeto quádrico para desenhar esferas com suporte a texturas
GLUquadric* quadrico; 

// --- FUNÇÃO PARA CARREGAR IMAGENS .BMP (24-bits) ---
GLuint loadBMP_custom(const char * imagepath) {
    printf("Carregando imagem %s\n", imagepath);
    unsigned char header[54];
    unsigned int dataPos, width, height, imageSize;
    unsigned char * data;

    FILE * file = fopen(imagepath, "rb");
    if (!file) { printf("Imagem nao encontrada: %s\n", imagepath); return 0; }
    if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M') {
        printf("Nao e um arquivo BMP valido: %s\n", imagepath);
        fclose(file); return 0;
    }

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
    
    // Configura os parâmetros da textura (Filtros de interpolação)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    delete[] data;
    return textureID;
}

float calculaAnguloPlaneta(float velocidade) {
    return fmod(tempoGlobal * velocidade, 360.0f);
}

void carregaTexturas() {
    // Certifique-se de que os nomes batem exatamente com os arquivos da sua pasta
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

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, luzBranca);
    GLfloat posPontual1[] = { 0.0f, 1.0f, 2.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, posPontual1);
}

// Função auxiliar para desenhar planetas texturizados
void desenhaPlanetaTexturizado(GLuint textura, float raio) {
    glBindTexture(GL_TEXTURE_2D, textura);
    // IMPORTANTE: Deixe a cor branca para não "tingir" a textura
    glColor3f(1.0f, 1.0f, 1.0f); 
    
    // Rotaciona 90 graus no eixo X para que os polos das texturas fiquem em cima/embaixo
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluSphere(quadrico, raio, 32, 32);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Afastei a câmera (eixo Z = 25.0) para caberem todos os planetas
    gluLookAt(0.0, 8.0, 25.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
              
    configuraLuz();

    // 1. Sol
    glPushMatrix();
    // O Sol emite luz, não reage à sombra
    GLfloat luzAmarelaSol[] = { 1.0f, 1.0f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, luzAmarelaSol);
    desenhaPlanetaTexturizado(texSun, 1.5f);
    glMaterialfv(GL_FRONT, GL_EMISSION, luzAmbienteFraca); // Reseta material
    glPopMatrix();

    // 2. Mercúrio
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(8.3f), 0.0f, 1.0f, 0.0f);
    glTranslatef(3.0f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texMercury, 0.2f);
    glPopMatrix();

    // 3. Vênus
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(3.2f), 0.0f, 1.0f, 0.0f);
    glTranslatef(4.5f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texVenus, 0.3f);
    glPopMatrix();

    // 4. Terra
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(2.0f), 0.0f, 1.0f, 0.0f);
    glTranslatef(6.0f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texEarth, 0.4f);

        // 4.1. Lua
        glPushMatrix();
        glRotatef(calculaAnguloPlaneta(26.6f), 0.0f, 1.0f, 0.0f);
        glTranslatef(0.8f, 0.0f, 0.0f);
        desenhaPlanetaTexturizado(texMoon, 0.15f);
        glPopMatrix();

    glPopMatrix();

    // 5. Marte
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(1.0f), 0.0f, 1.0f, 0.0f);
    glTranslatef(7.5f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texMars, 0.25f);
    glPopMatrix();

    // 6. Júpiter
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.17f), 0.0f, 1.0f, 0.0f);
    glTranslatef(10.0f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texJupiter, 0.9f);
    glPopMatrix();

    // 7. Saturno
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.06f), 0.0f, 1.0f, 0.0f);
    glTranslatef(13.0f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texSaturn, 0.7f);
    
        // Anel de Saturno (Desenhado como um disco ao redor)
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, texSaturnRing);
        glRotatef(-70.0f, 1.0f, 0.0f, 0.0f); // Inclinação do anel
        gluDisk(quadrico, 0.8f, 1.5f, 32, 32);
        glPopMatrix();

    glPopMatrix();

    // 8. Urano
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.02f), 0.0f, 1.0f, 0.0f);
    glTranslatef(16.0f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texUranus, 0.5f);
    glPopMatrix();

    // 9. Netuno
    glPushMatrix();
    glRotatef(calculaAnguloPlaneta(0.01f), 0.0f, 1.0f, 0.0f);
    glTranslatef(18.5f, 0.0f, 0.0f);
    desenhaPlanetaTexturizado(texNeptune, 0.45f);
    glPopMatrix();

    glutSwapBuffers();
}

void inicializa() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL); 
    
    // Ativa suporte a texturas
    glEnable(GL_TEXTURE_2D);
    
    // Inicializa e configura o objeto quádrico para gerar coordenadas de textura
    quadrico = gluNewQuadric();
    gluQuadricTexture(quadrico, GL_TRUE); 
    gluQuadricNormals(quadrico, GLU_SMOOTH);
    
    carregaTexturas();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void atualizaAnimacao(int value) {
    tempoGlobal += 0.2f; 
    glutPostRedisplay();
    glutTimerFunc(16, atualizaAnimacao, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800); // Aumentei a janela para ver melhor a expansão
    glutCreateWindow("Sistema Solar Texturizado");

    inicializa();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, atualizaAnimacao, 0);

    glutMainLoop();
    return 0;
}