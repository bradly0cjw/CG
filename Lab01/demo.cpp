#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
/*** freeglut ***/
#include ".\GL\freeglut.h"

void ChangeSize(int, int);
void RenderScene(void);
void TimerFunc(int value);

int _mode = GL_FLAT; // 體素風格必須用 FLAT 邊緣才會俐落
float _animTime = 0.0f;

void Menu(int index) {
    switch (index) {
    case 1: _mode = GL_SMOOTH; break;
    case 2: _mode = GL_FLAT; break;
    }
    glutPostRedisplay();
}

void SetupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // 打亮一點，讓方塊看起來像明亮的玩具積木
    GLfloat ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat lightPos[] = { -5.0f, 10.0f, 10.0f, 1.0f }; // 光源從左上前方打來

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(400, 80);
    glutCreateWindow("Cute Voxel Nick");

    glutCreateMenu(Menu);
    glutAddMenuEntry("smooth", 1);
    glutAddMenuEntry("flat", 2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glEnable(GL_DEPTH_TEST);
    SetupLighting();

    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutTimerFunc(16, TimerFunc, 0);
    glutMainLoop();
    return 0;
}

void ChangeSize(int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)w / (float)h;
    gluPerspective(40.0, aspect, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RenderScene(void)
{
    // 溫暖的淺黃色背景，襯托可愛感
    glClearColor(0.9f, 0.85f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glShadeModel(_mode);

    // 攝影機拉遠一點，看全身
    gluLookAt(0, 1.5f, 12.0f,
        0, 0, 0,
        0, 1, 0);

    // 輕微的上下浮動動畫
    float bobbing = sin(_animTime * 4.0f) * 0.1f;

    // 讓整隻狐狸微微側身，看起來比較呆萌
    glRotatef(15.0f, 0.0f, 1.0f, 0.0f);

    // ==========================================
    // 體素積木建構 (全部使用 glutSolidCube)
    // ==========================================

    // -- 身體 (小小的，襯衫綠色) --
    glPushMatrix();
    glTranslatef(0.0f, -1.5f + bobbing, 0.0f);
    glColor3f(0.18f, 0.55f, 0.34f);
    glPushMatrix();
    glScalef(1.2f, 1.2f, 0.8f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 領帶 (貼在胸前的深色小方塊)
    glColor3f(0.0f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.45f);
    glScalef(0.3f, 0.8f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPopMatrix();

    // -- 頭部 (超大比例，橘色) --
    glPushMatrix();
    glTranslatef(0.0f, 0.0f + bobbing * 1.5f, 0.0f);

    // 1. 大方塊頭
    glColor3f(0.85f, 0.45f, 0.15f); // 亮橘色
    glPushMatrix();
    glScalef(2.4f, 1.8f, 1.8f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 2. 吻部 (米色方塊，往前凸)
    glColor3f(1.00f, 0.90f, 0.70f);
    glPushMatrix();
    glTranslatef(0.0f, -0.4f, 1.0f);
    glScalef(1.2f, 0.6f, 0.8f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 3. 鼻頭 (黑色小方塊)
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.2f, 1.45f);
    glScalef(0.4f, 0.3f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 4. 左耳 (橘色方塊)
    glColor3f(0.85f, 0.45f, 0.15f);
    glPushMatrix();
    glTranslatef(-0.8f, 1.3f, 0.0f);
    glRotatef(-15.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.6f, 0.8f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 5. 右耳 (橘色方塊)
    glPushMatrix();
    glTranslatef(0.8f, 1.3f, 0.0f);
    glRotatef(15.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.6f, 0.8f, 0.4f);
    glutSolidCube(1.0);
    glPopMatrix();

    // 6. 眼睛 (黑色小方塊，眼距拉寬)
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(-0.7f, 0.2f, 0.95f);
    glScalef(0.25f, 0.25f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.7f, 0.2f, 0.95f);
    glScalef(0.25f, 0.25f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();

    glutSwapBuffers();
}

void TimerFunc(int value) {
    _animTime += 0.03f;
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunc, 0);
}
