#include <GL/freeglut.h>
#include <iostream>

// Light modes: 0-All, 1-Directional Only, 2-Point Only, 3-Spotlight Only
int lightMode = 0;
float rotation = 0.0f;

void setupRC() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Pure Black background
    glEnable(GL_DEPTH_TEST);
    
    // Setup Lighting
    glEnable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    
    // Global Ambient is 0
    GLfloat global_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // Setup Material Specular Properties
    GLfloat mat_specular[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 100.0f }; 
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // Basic Colors for lights (Static properties)
    GLfloat red[]   = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLfloat blue[]  = { 0.0f, 0.0f, 1.0f, 1.0f };
    GLfloat green[] = { 0.0f, 1.0f, 0.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_DIFFUSE, red);
    glLightfv(GL_LIGHT0, GL_SPECULAR, red);

    glLightfv(GL_LIGHT1, GL_DIFFUSE, blue);
    glLightfv(GL_LIGHT1, GL_SPECULAR, blue);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.1f);

    glLightfv(GL_LIGHT2, GL_DIFFUSE, green);
    glLightfv(GL_LIGHT2, GL_SPECULAR, green);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 45.0f);   // Wider cone
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 10.0f); // Softer edge

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void drawQuad(float x1, float y1, float z1,
              float x2, float y2, float z2,
              float x3, float y3, float z3,
              float x4, float y4, float z4,
              float nx, float ny, float nz) {
    glNormal3f(nx, ny, nz);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
}

void drawSmallHouse() {
    // Using a darker gray color (0.4) for the object makes the colored lights
    // much more apparent because it prevents the additive mixing from reaching
    // pure white too quickly. It still looks like a "white-ish" object in shadows.
    glColor3f(0.4f, 0.4f, 0.4f);

    // Walls (4 quads)
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
    glVertex3f(-1.0, 1.5, 1.0);
    glVertex3f(1.0, 1.5, 1.0);
    glVertex3f(0.0, 2.2, 1.0);
    glVertex3f(0.0, 2.2, 1.0);
    // Back Gable
    glNormal3f(0, 0, -1);
    glVertex3f(1.0, 1.5, -1.0);
    glVertex3f(-1.0, 1.5, -1.0);
    glVertex3f(0.0, 2.2, -1.0);
    glVertex3f(0.0, 2.2, -1.0);
    glEnd();

    // Roof (2 quads)
    glBegin(GL_QUADS);
    // Left Slope (calculated normals for slope 0.7)
    drawQuad(-1.1, 1.43, 1.1, 0.0, 2.2, 1.1, 0.0, 2.2, -1.1, -1.1, 1.43, -1.1, -0.573, 0.819, 0);
    // Right Slope
    drawQuad(1.1, 1.43, -1.1, 0.0, 2.2, -1.1, 0.0, 2.2, 1.1, 1.1, 1.43, 1.1, 0.573, 0.819, 0);
    glEnd();

    // Door (1 quad)
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0, -1.0);
    glBegin(GL_QUADS);
    drawQuad(-0.3, 0.0, 1.0, 0.3, 0.0, 1.0, 0.3, 0.9, 1.0, -0.3, 0.9, 1.0, 0, 0, 1);
    glEnd();

    // Window (1 quad)
    glBegin(GL_QUADS);
    drawQuad(0.4, 0.6, 1.0, 0.8, 0.6, 1.0, 0.8, 1.1, 1.0, 0.4, 1.1, 1.0, 0, 0, 1);
    glEnd();
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 3, 6, 0, 1, 0, 0, 1, 0);

    // Update light positions and directions in world space (relative to camera)
    GLfloat light0_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f }; // Directional
    GLfloat light1_pos[] = { -3.0f, 3.0f, 3.0f, 1.0f }; // Point
    GLfloat light2_pos[] = { 0.0f, 5.0f, 0.0f, 1.0f }; // Spotlight Pos
    GLfloat light2_dir[] = { 0.0f, -1.0f, 0.0f };      // Spotlight Dir

    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light2_dir);

    // Default states
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    // Set light states based on lightMode
    if (lightMode == 0) {
        glEnable(GL_LIGHT0); glEnable(GL_LIGHT1); glEnable(GL_LIGHT2);
    } else {
        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2);
        if (lightMode == 1) glEnable(GL_LIGHT0);
        else if (lightMode == 2) glEnable(GL_LIGHT1);
        else if (lightMode == 3) glEnable(GL_LIGHT2);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glPushMatrix();
    glRotatef(rotation, 1.0, 1.0, 0.5); // Spin in multiple axes
    drawSmallHouse();
    glPopMatrix();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key >= '0' && key <= '3') {
        lightMode = key - '0';
        std::cout << "Light Mode: " << lightMode << " (0:All, 1:Dir, 2:Point, 3:Spot)" << std::endl;
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
    glutCreateWindow("Lab 12 - Advanced Lighting on House");

    setupRC();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
