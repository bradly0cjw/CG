#include <GL/glut.h>
#include <cmath>

// 旋轉角度
float angle = 0.0f;

// 輔助函式：給定三個頂點，計算出該面的單位法向量
void calculateNormal(float p1[3], float p2[3], float p3[3], float normal[3]) {
    float ax = p2[0] - p1[0];
    float ay = p2[1] - p1[1];
    float az = p2[2] - p1[2];

    float bx = p3[0] - p1[0];
    float by = p3[1] - p1[1];
    float bz = p3[2] - p1[2];

    float nx = ay * bz - az * by;
    float ny = az * bx - ax * bz;
    float nz = ax * by - ay * bx;

    float length = sqrt(nx * nx + ny * ny + nz * nz);
    if (length == 0.0f) length = 1.0f;

    normal[0] = nx / length;
    normal[1] = ny / length;
    normal[2] = nz / length;
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // ==========================================
    // 設定材質 (與前一個範例相同的金黃色材質)
    // ==========================================
    GLfloat mat_ambient[]   = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat mat_diffuse[]   = { 0.8f, 0.6f, 0.2f, 1.0f };
    GLfloat mat_specular[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // ==========================================
    // 設定三種光源 (平行光、點光源、聚光燈)
    // ==========================================
    // 1. 平行光 (紅光)
    GLfloat light0_pos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    GLfloat light0_diffuse[] = { 0.8f, 0.2f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glEnable(GL_LIGHT0);

    // 2. 點光源 (藍光)
    GLfloat light1_pos[] = { -2.0f, 2.0f, 2.0f, 1.0f };
    GLfloat light1_diffuse[] = { 0.2f, 0.2f, 0.8f, 1.0f };
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.2f);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glEnable(GL_LIGHT1);

    // 3. 聚光燈 (綠光)
    GLfloat light2_pos[] = { 0.0f, 5.0f, 0.0f, 1.0f };
    GLfloat light2_dir[] = { 0.0f, -1.0f, 0.0f };
    GLfloat light2_diffuse[] = { 0.2f, 0.8f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light2_dir);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0f);
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 10.0f);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
    glEnable(GL_LIGHT2);
}

// 繪製自訂三角柱的函式
void drawTriangularPrism() {
    // 定義三角柱的 6 個頂點位置
    // 頂面三角形 (Y = 1.0)
    float t0[3] = {  0.0f,  1.0f, -0.8f }; // 上後
    float t1[3] = { -0.8f,  1.0f,  0.6f }; // 上左前
    float t2[3] = {  0.8f,  1.0f,  0.6f }; // 上右前

    // 底面三角形 (Y = -1.0)
    float b0[3] = {  0.0f, -1.0f, -0.8f }; // 下後
    float b1[3] = { -0.8f, -1.0f,  0.6f }; // 下左前
    float b2[3] = {  0.8f, -1.0f,  0.6f }; // 下右前

    float normal[3];

    // --- 1. 頂面三角形 (逆時針: t0 -> t1 -> t2) ---
    calculateNormal(t0, t1, t2, normal);
    glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        glVertex3fv(t0); glVertex3fv(t1); glVertex3fv(t2);
    glEnd();

    // --- 2. 底面三角形 (從下面看要逆時針: b0 -> b2 -> b1) ---
    calculateNormal(b0, b2, b1, normal);
    glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        glVertex3fv(b0); glVertex3fv(b2); glVertex3fv(b1);
    glEnd();

    // --- 3. 側面 1: 正前方矩形 (t1 -> b1 -> b2 -> t2) ---
    calculateNormal(t1, b1, b2, normal);
    glBegin(GL_QUADS);
        glNormal3fv(normal);
        glVertex3fv(t1); glVertex3fv(b1); glVertex3fv(b2); glVertex3fv(t2);
    glEnd();

    // --- 4. 側面 2: 左後方矩形 (t0 -> t1 -> b1 -> b0) ---
    calculateNormal(t0, t1, b1, normal);
    glBegin(GL_QUADS);
        glNormal3fv(normal);
        glVertex3fv(t0); glVertex3fv(t1); glVertex3fv(b1); glVertex3fv(b0);
    glEnd();

    // --- 5. 側面 3: 右後方矩形 (t2 -> b2 -> b0 -> t0) ---
    calculateNormal(t2, b2, b0, normal);
    glBegin(GL_QUADS);
        glNormal3fv(normal);
        glVertex3fv(t2); glVertex3fv(b2); glVertex3fv(b0); glVertex3fv(t0);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(angle, 1.0f, 1.0f, 0.5f); // 旋轉它以觀察各個面的光影

    drawTriangularPrism(); // 呼叫我們自己畫的三角柱

    glutSwapBuffers();
}

void update(int value) {
    angle += 1.0f;
    if (angle > 360.0f) angle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Fixed Pipeline - 3 Lights on Custom Prism");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}