//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
//#include ".\GL\freeglut.h"
//
//float angleX = 30.0f;
//float angleY = 60.0f;
//
//void ChangeSize(int, int);
//void RenderScene(void);
//void Idle(void);
//
//int main(int argc, char** argv)
//{
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//    glutInitWindowSize(800, 600);
//    glutInitWindowPosition(400, 80);
//    glutCreateWindow("Spinning Triangular Prism - Lab02");
//
//    glEnable(GL_DEPTH_TEST);
//
//    glutReshapeFunc(ChangeSize);
//    glutDisplayFunc(RenderScene);
//    glutIdleFunc(Idle);
//    glutMainLoop();
//    return 0;
//}
//
//void Idle(void)
//{
//    angleX += 0.1f;
//    angleY += 0.2f;
//    glutPostRedisplay();
//}
//
//void ChangeSize(int w, int h)
//{
//    if (h == 0) h = 1;
//    glViewport(0, 0, w, h);
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//
//    // From Page 7 of PDF: glOrtho(-10,10,-10,10,-10,20);
//    glOrtho(-10, 10, -10, 10, -10, 20);
//
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//}
//
//void RenderScene(void)
//{
//    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    
//    // From Page 5/8 of PDF: gluLookAt(0,0,10.0f ,0,0,0, 0,1,0);
//    gluLookAt(0, 0, 10.0f, 0, 0, 0, 0, 1, 0);
//
//    // Dynamic rotation
//    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
//    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
//
//    // Drawing a Triangular Prism using multiple triangles
//    // Each Rectangular face is formed by 2 triangles
//    // Counter-clockwise order (Page 4)
//
//    // Vertices (from Page 3):
//    // V0: ( 0.00,  1.0,  0.0)
//    // V1: (-0.86, -0.5,  0.0)
//    // V2: ( 0.86, -0.5,  0.0)
//    // V3: ( 0.00,  1.0, -2.0)
//    // V4: (-0.86, -0.5, -2.0)
//    // V5: ( 0.86, -0.5, -2.0)
//
//    // Front triangle face (z=0)
//    glBegin(GL_TRIANGLES);
//    glColor3f(0.7f, 0.7f, 0.7f); // Gray
//    glVertex3f(0.0f, 1.0f, 0.0f);    // V0
//    glVertex3f(-0.86f, -0.5f, 0.0f); // V1
//    glVertex3f(0.86f, -0.5f, 0.0f);  // V2
//    glEnd();
//
//    // Back triangle face (z=-2)
//    glBegin(GL_TRIANGLES);
//    glColor3f(0.7f, 0.7f, 0.7f); // Gray
//    glVertex3f(0.0f, 1.0f, -2.0f);    // V3
//    glVertex3f(0.86f, -0.5f, -2.0f);  // V5
//    glVertex3f(-0.86f, -0.5f, -2.0f); // V4
//    glEnd();
//
//    // Right rectangular face (Purple)
//    glBegin(GL_TRIANGLES);
//    glColor3f(0.5f, 0.0f, 0.5f);
//    // Triangle 1
//    glVertex3f(0.0f, 1.0f, 0.0f);    // V0
//    glVertex3f(0.86f, -0.5f, 0.0f);  // V2
//    glVertex3f(0.86f, -0.5f, -2.0f); // V5
//    // Triangle 2
//    glVertex3f(0.0f, 1.0f, 0.0f);    // V0
//    glVertex3f(0.86f, -0.5f, -2.0f); // V5
//    glVertex3f(0.0f, 1.0f, -2.0f);   // V3
//    glEnd();
//
//    // Left rectangular face (Teal)
//    glBegin(GL_TRIANGLES);
//    glColor3f(0.0f, 0.5f, 0.5f);
//    // Triangle 1
//    glVertex3f(0.0f, 1.0f, 0.0f);     // V0
//    glVertex3f(0.0f, 1.0f, -2.0f);    // V3
//    glVertex3f(-0.86f, -0.5f, -2.0f); // V4
//    // Triangle 2
//    glVertex3f(0.0f, 1.0f, 0.0f);     // V0
//    glVertex3f(-0.86f, -0.5f, -2.0f); // V4
//    glVertex3f(-0.86f, -0.5f, 0.0f);  // V1
//    glEnd();
//
//    // Bottom rectangular face (Maroon)
//    glBegin(GL_TRIANGLES);
//    glColor3f(0.5f, 0.0f, 0.0f);
//    // Triangle 1
//    glVertex3f(-0.86f, -0.5f, 0.0f);  // V1
//    glVertex3f(-0.86f, -0.5f, -2.0f); // V4
//    glVertex3f(0.86f, -0.5f, -2.0f);  // V5
//    // Triangle 2
//    glVertex3f(-0.86f, -0.5f, 0.0f);  // V1
//    glVertex3f(0.86f, -0.5f, -2.0f);  // V5
//    glVertex3f(0.86f, -0.5f, 0.0f);   // V2
//    glEnd();
//
//    glutSwapBuffers();
//}
