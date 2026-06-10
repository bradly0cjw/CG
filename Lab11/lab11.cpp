#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/freeglut.h>
#include <iostream>

// Render modes: 1-mesh, 2-mesh with culling, 3-color fill, 4-texture fill
int renderMode = 4;
GLuint textures[5];
float rotation = 0.0f;
bool useAlternativeFloor = false;

// Texture loading helper
void loadTexture(const char* filename, GLuint& texID) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (data) {
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        std::cout << "Loaded texture: " << filename << std::endl;
    } else {
        std::cerr << "Failed to load texture: " << filename << std::endl;
    }
}

void setupRC() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // Load 5 textures from the current folder
    loadTexture("table.jpg", textures[0]);     // Floor
    loadTexture("table2.jpg", textures[5]);    // Alternative floor
    loadTexture("material1.jpg", textures[1]); // Walls
    loadTexture("material2.jpg", textures[2]); // Roof
    loadTexture("material3.jpg", textures[3]); // Door
    loadTexture("material4.jpg", textures[4]); // Window
    

    // Setup Lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[] = { 5.0f, 10.0f, 5.0f, 1.0f };
    GLfloat ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void drawQuad(float x1, float y1, float z1,
              float x2, float y2, float z2,
              float x3, float y3, float z3,
              float x4, float y4, float z4,
              float nx, float ny, float nz) {
    glNormal3f(nx, ny, nz);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, y1, z1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x2, y2, z2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x3, y3, z3);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x4, y4, z4);
}

void drawSmallHouse() {
    // Walls (4 quads)
    if (renderMode == 4) {
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glColor3f(1.0f, 1.0f, 1.0f); // Texture only, no tint
    } else {
        glColor3f(0.8f, 0.8f, 0.8f); // Light Grey
    }
    glBegin(GL_QUADS);
    // Front Wall
    drawQuad(-1.0, 0.0, 1.0,  1.0, 0.0, 1.0,  1.0, 1.5, 1.0, -1.0, 1.5, 1.0, 0, 0, 1);
    // Back Wall
    drawQuad(-1.0, 0.0, -1.0, -1.0, 1.5, -1.0, 1.0, 1.5, -1.0, 1.0, 0.0, -1.0, 0, 0, -1);
    // Left Wall
    drawQuad(-1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 1.5, 1.0, -1.0, 1.5, -1.0, -1, 0, 0);
    // Right Wall
    drawQuad(1.0, 0.0, -1.0, 1.0, 1.5, -1.0, 1.0, 1.5, 1.0, 1.0, 0.0, 1.0, 1, 0, 0);
    glEnd();

    // Gable Ends (2 quads, degenerate to triangles)
    glBegin(GL_QUADS);
    // Front Gable
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, 1.5, 1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, 1.5, 1.0);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0, 2.2, 1.0);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0, 2.2, 1.0);
    // Back Gable
    glNormal3f(0, 0, -1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0, 1.5, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0, 1.5, -1.0);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0, 2.2, -1.0);
    glTexCoord2f(0.5f, 1.0f); glVertex3f(0.0, 2.2, -1.0);
    glEnd();

    // Roof (2 quads)
    if (renderMode == 4) {
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glColor3f(0.6f, 0.1f, 0.1f); // Dark Red
    }
    glBegin(GL_QUADS);
    // Left Slope (calculated normals for slope 0.7)
    drawQuad(-1.1, 1.43, 1.1, 0.0, 2.2, 1.1, 0.0, 2.2, -1.1, -1.1, 1.43, -1.1, -0.573, 0.819, 0);
    // Right Slope
    drawQuad(1.1, 1.43, -1.1, 0.0, 2.2, -1.1, 0.0, 2.2, 1.1, 1.1, 1.43, 1.1, 0.573, 0.819, 0);
    glEnd();

    // Door (1 quad)
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    if (renderMode == 4) {
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glColor3f(0.4f, 0.2f, 0.0f); // Brown
    }
    glBegin(GL_QUADS);
    drawQuad(-0.3, 0.0, 1.0, 0.3, 0.0, 1.0, 0.3, 0.9, 1.0, -0.3, 0.9, 1.0, 0, 0, 1);
    glEnd();

    // Window (1 quad)
    if (renderMode == 4) {
        glBindTexture(GL_TEXTURE_2D, textures[4]);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glColor3f(0.0f, 0.8f, 0.8f); // Cyan
    }
    glBegin(GL_QUADS);
    drawQuad(0.4, 0.6, 1.0, 0.8, 0.6, 1.0, 0.8, 1.1, 1.0, 0.4, 1.1, 1.0, 0, 0, 1);
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void drawFloor() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Always fill floor
    if (renderMode == 4) {
        glEnable(GL_TEXTURE_2D);
        if (useAlternativeFloor) {
            glBindTexture(GL_TEXTURE_2D, textures[5]);
        } else {
            glBindTexture(GL_TEXTURE_2D, textures[0]);
        }
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.0f, 0.0f, 0.5f); // Dark Blue
    }
    
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, -10.0f);
    glTexCoord2f(10.0f, 0.0f); glVertex3f(10.0f, 0.0f, -10.0f);
    glTexCoord2f(10.0f, 10.0f); glVertex3f(10.0f, 0.0f, 10.0f);
    glTexCoord2f(0.0f, 10.0f); glVertex3f(-10.0f, 0.0f, 10.0f);
    glEnd();
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void drawHUD() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f); // White text
    renderBitmapString(10, 20, GLUT_BITMAP_9_BY_15, "Modes: 1-Mesh | 2-Culling | 3-Color | 4-Texture");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 3, 6, 0, 1, 0, 0, 1, 0);

    // Default states
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    drawFloor();

    // Set house polygon mode
    if (renderMode == 1 || renderMode == 2) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);
        if (renderMode == 2) glEnable(GL_CULL_FACE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (renderMode == 4) glEnable(GL_TEXTURE_2D);
    }

    glPushMatrix();
    glRotatef(rotation, 0, 1, 0);
    drawSmallHouse();
    glPopMatrix();

    drawHUD();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key >= '1' && key <= '4') {
        renderMode = key - '0';
        std::cout << "Render Mode: " << renderMode << std::endl;
    }else{
        useAlternativeFloor = !useAlternativeFloor;
    }
    glutPostRedisplay();
}

void timer(int value) {
    rotation += 0.5f;
    if (rotation > 360.0f) rotation -= 360.0f;
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Lab 11 - Textured Creative Object");

    setupRC();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
