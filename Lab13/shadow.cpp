#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <math.h>
#include <stdio.h>

// Math types and functions (to make it self-contained)
typedef float M3DVector3f[3];
typedef float M3DVector4f[4];
typedef float M3DMatrix44f[16];

void m3dCrossProduct(M3DVector3f result, const M3DVector3f u, const M3DVector3f v) {
    result[0] = u[1] * v[2] - v[1] * u[2];
    result[1] = u[2] * v[0] - v[2] * u[0];
    result[2] = u[0] * v[1] - v[0] * u[1];
}

void m3dNormalizeVector(M3DVector3f v) {
    float mag = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (mag > 0.0f) {
        v[0] /= mag; v[1] /= mag; v[2] /= mag;
    }
}

void m3dGetPlaneEquation(M3DVector4f planeEq, const M3DVector3f p1, const M3DVector3f p2, const M3DVector3f p3) {
    M3DVector3f v1, v2;
    v1[0] = p3[0] - p1[0]; v1[1] = p3[1] - p1[1]; v1[2] = p3[2] - p1[2];
    v2[0] = p2[0] - p1[0]; v2[1] = p2[1] - p1[1]; v2[2] = p2[2] - p1[2];
    m3dCrossProduct(planeEq, v1, v2);
    m3dNormalizeVector(planeEq);
    planeEq[3] = -(planeEq[0] * p3[0] + planeEq[1] * p3[1] + planeEq[2] * p3[2]);
}

void m3dMakePlanarShadowMatrix(M3DMatrix44f proj, const M3DVector4f planeEq, const M3DVector3f vLightPos) {
    float a = planeEq[0]; float b = planeEq[1]; float c = planeEq[2]; float d = planeEq[3];
    float dx = -vLightPos[0]; float dy = -vLightPos[1]; float dz = -vLightPos[2];
    proj[0] = b * dy + c * dz; proj[4] = -b * dx; proj[8] = -c * dx; proj[12] = -d * dx;
    proj[1] = -a * dy; proj[5] = a * dx + c * dz; proj[9] = -c * dy; proj[13] = -d * dy;
    proj[2] = -a * dz; proj[6] = -b * dz; proj[10] = a * dx + b * dy; proj[14] = -d * dz;
    proj[3] = 0.0; proj[7] = 0.0; proj[11] = 0.0; proj[15] = a * dx + b * dy + c * dz;
}

// Global variables
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;
static int windowWidth, windowHeight;

// Light settings
GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f };

GLfloat lightPositions[3][4] = {
    { -75.0f, 150.0f, -50.0f, 1.0f },
    { 75.0f, 150.0f, -50.0f, 1.0f },
    { 0.0f, 150.0f, 75.0f, 1.0f }
};
int activeLight = 0;

M3DMatrix44f shadowMat;
M3DVector4f vPlaneEquation;

// Robot animation variables
float fHeadRot = 0.0f;
float fUpperArmRotL = 0.0f;
float fLowerArmRotL = 0.0f;
float fUpperArmRotR = 0.0f;
float fLowerArmRotR = 0.0f;
float fUpperLegRotL = 0.0f;
float fLowerLegRotL = 0.0f;
float fUpperLegRotR = 0.0f;
float fLowerLegRotR = 0.0f;
bool bAnimate = true;
float fTime = 0.0f;

void DrawCube(float x, float y, float z) {
    glPushMatrix();
    glScalef(x, y, z);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void DrawRobot(int nShadow) {
    if (nShadow == 0)
        glColor3ub(0, 0, 255); // Blue robot
    else
        glColor3ub(0, 0, 0); // Black shadow

    // Torso
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    DrawCube(20.0f, 40.0f, 10.0f);

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 25.0f, 0.0f);
    glRotatef(fHeadRot, 0.0f, 1.0f, 0.0f);
    if (nShadow == 0) glColor3ub(255, 200, 150);
    glutSolidSphere(8.0f, 15, 15);
    glPopMatrix();

    // Left Arm
    if (nShadow == 0) glColor3ub(0, 0, 255);
    glPushMatrix();
    glTranslatef(-15.0f, 15.0f, 0.0f);
    glRotatef(fUpperArmRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(6.0f, 20.0f, 6.0f); // Upper
    
    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(fLowerArmRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(5.0f, 20.0f, 5.0f); // Lower
    glPopMatrix();

    // Right Arm
    glPushMatrix();
    glTranslatef(15.0f, 15.0f, 0.0f);
    glRotatef(fUpperArmRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(6.0f, 20.0f, 6.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(fLowerArmRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(5.0f, 20.0f, 5.0f); // Lower
    glPopMatrix();

    // Left Leg
    if (nShadow == 0) glColor3ub(255, 0, 0); // Red legs
    glPushMatrix();
    glTranslatef(-7.0f, -20.0f, 0.0f);
    glRotatef(fUpperLegRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(8.0f, 20.0f, 8.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(fLowerLegRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(7.0f, 20.0f, 7.0f); // Lower
    glPopMatrix();

    // Right Leg
    glPushMatrix();
    glTranslatef(7.0f, -20.0f, 0.0f);
    glRotatef(fUpperLegRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(8.0f, 20.0f, 8.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(fLowerLegRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawCube(7.0f, 20.0f, 7.0f); // Lower
    glPopMatrix();

    glPopMatrix();
}

void RenderText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void DrawUI() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3ub(255, 255, 255);
    float lineSpacing = 15.0f;
    float startY = 160.0f;
    float startX = 10.0f;

    RenderText(startX, startY, "Keybindings:");
    RenderText(startX, startY - lineSpacing,     "Arrows: Rotate Plane");
    RenderText(startX, startY - 2 * lineSpacing, "1, 2, 3: Switch Light");
    RenderText(startX, startY - 3 * lineSpacing, "R: Reset Plane");
    RenderText(startX, startY - 4 * lineSpacing, "P: Pause/Resume");
    RenderText(startX, startY - 5 * lineSpacing, "Q/A, W/S: Upper Arms");
    RenderText(startX, startY - 6 * lineSpacing, "T/V, Y/H: Lower Arms");
    RenderText(startX, startY - 7 * lineSpacing, "E/D, F/G: Upper Legs");
    RenderText(startX, startY - 8 * lineSpacing, "U/J, I/K: Lower Legs");
    RenderText(startX, startY - 9 * lineSpacing, "Z/X: Head");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void RenderScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Apply scene rotation (plane rotation)
    glPushMatrix();
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot, 0.0f, 1.0f, 0.0f);

    // Draw ground
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3ub(0, 32, 0);
    glVertex3f(400.0f, -150.0f, -200.0f);
    glVertex3f(-400.0f, -150.0f, -200.0f);
    glColor3ub(0, 255, 0);
    glVertex3f(-400.0f, -150.0f, 200.0f);
    glVertex3f(400.0f, -150.0f, 200.0f);
    glEnd();

    // Robot position on ground
    // Total height of leg: 20 (upper) + 20 (lower) = 40.
    // Origin of leg is at -20 from torso center.
    // Torso center is at (0, robotY).
    // Leg bottom is at robotY - 60.
    // We want robotY - 60 = -149.9 (slightly above ground at -150)
    float robotY = -89.9f; 

    // Draw Shadow
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // Recalculate shadow matrix for current light
    m3dMakePlanarShadowMatrix(shadowMat, vPlaneEquation, lightPositions[activeLight]);
    
    glPushMatrix();
    glMultMatrixf((GLfloat*)shadowMat);
    glTranslatef(0.0f, robotY, 0.0f);
    DrawRobot(1);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);

    // Draw Robot
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPositions[activeLight]);
    glPushMatrix();
    glTranslatef(0.0f, robotY, 0.0f);
    DrawRobot(0);
    glPopMatrix();

    // Visualize active light source
    glPushMatrix();
    glTranslatef(lightPositions[activeLight][0], lightPositions[activeLight][1], lightPositions[activeLight][2]);
    glColor3ub(255, 255, 0);
    glutSolidSphere(5.0f, 10, 10);
    glPopMatrix();

    glPopMatrix(); // End scene rotation

    DrawUI();

    glutSwapBuffers();
}

void SetupRC() {
    M3DVector3f points[3] = { { -30.0f, -149.9f, -20.0f },
                            { -30.0f, -149.9f, 20.0f },
                            { 40.0f, -149.9f, 20.0f } };

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    m3dGetPlaneEquation(vPlaneEquation, points[0], points[1], points[2]);
    glEnable(GL_NORMALIZE);
}

void TimerFunc(int value) {
    if (bAnimate) {
        fTime += 0.05f;
        fHeadRot = 30.0f * sinf(fTime);
        
        // Arms
        fUpperArmRotL = 45.0f * sinf(fTime);
        fLowerArmRotL = -30.0f * (sinf(fTime) + 1.0f); // Bent
        fUpperArmRotR = 45.0f * sinf(fTime + 3.14f);
        fLowerArmRotR = -30.0f * (sinf(fTime + 3.14f) + 1.0f);

        // Legs
        fUpperLegRotL = 30.0f * sinf(fTime + 3.14f);
        fLowerLegRotL = 20.0f * (sinf(fTime + 3.14f) + 1.0f); // Bent at knee
        fUpperLegRotR = 30.0f * sinf(fTime);
        fLowerLegRotR = 20.0f * (sinf(fTime) + 1.0f);
    }
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunc, 1);
}

void SpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) xRot -= 5.0f;
    if (key == GLUT_KEY_DOWN) xRot += 5.0f;
    if (key == GLUT_KEY_LEFT) yRot -= 5.0f;
    if (key == GLUT_KEY_RIGHT) yRot += 5.0f;
    glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '1': activeLight = 0; break;
    case '2': activeLight = 1; break;
    case '3': activeLight = 2; break;
    case 'r': case 'R': xRot = 0.0f; yRot = 0.0f; break;
    case 'p': case 'P': bAnimate = !bAnimate; break;
    
    // Manual controls for robot parts
    case 'q': fUpperArmRotL += 5.0f; break;
    case 'a': fUpperArmRotL -= 5.0f; break;
    case 'w': fUpperArmRotR += 5.0f; break;
    case 's': fUpperArmRotR -= 5.0f; break;
    
    case 't': fLowerArmRotL += 5.0f; break;
    case 'v': fLowerArmRotL -= 5.0f; break; // V instead of G because G is used
    case 'y': fLowerArmRotR += 5.0f; break;
    case 'h': fLowerArmRotR -= 5.0f; break;

    case 'e': fUpperLegRotL += 5.0f; break;
    case 'd': fUpperLegRotL -= 5.0f; break;
    case 'f': fUpperLegRotR += 5.0f; break;
    case 'g': fUpperLegRotR -= 5.0f; break;

    case 'u': fLowerLegRotL += 5.0f; break;
    case 'j': fLowerLegRotL -= 5.0f; break;
    case 'i': fLowerLegRotR += 5.0f; break;
    case 'k': fLowerLegRotR -= 5.0f; break;

    case 'z': fHeadRot += 5.0f; break;
    case 'x': fHeadRot -= 5.0f; break;
    }
    glutPostRedisplay();
}

void ChangeSize(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -400.0f);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Robot Shadow");
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutKeyboardFunc(Keyboard);
    glutDisplayFunc(RenderScene);
    glutTimerFunc(16, TimerFunc, 1);
    SetupRC();
    glutMainLoop();
    return 0;
}
