//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
//#include ".\GL\freeglut.h"
//
//#ifndef M_PI
//#define M_PI 3.14159265358979323846
//#endif
//
//// Object transformation variables
//float objX = 0.0f;
//float objY = 0.0f;
//float objZ = 0.0f;
//
//// Separate scaling for each axis
//float objScaleX = 5.0f;
//float objScaleY = 5.0f;
//float objScaleZ = 5.0f;
//
//float objRotX = 0.0f;
//float objRotY = 0.0f;
//float objRotZ = 0.0f;
//
//void ChangeSize(int, int);
//void RenderScene(void);
//void Idle(void);
//void Keyboard(unsigned char key, int x, int y);
//
//// Custom matrix multiplication function
//void matrixMultiply(float* A, float* B, float* res) {
//	float temp[16] = { 0 };
//	for (int i = 0; i < 4; i++) { // Column
//		for (int j = 0; j < 4; j++) { // Row
//			for (int k = 0; k < 4; k++) {
//				temp[i * 4 + j] += A[k * 4 + j] * B[i * 4 + k];
//			}
//		}
//	}
//	for (int i = 0; i < 16; i++) res[i] = temp[i];
//}
//
//// Custom SRT transformation function using manual homogeneous matrix
//void applyCustomTransformations(float tx, float ty, float tz, float sx, float sy, float sz, float rx, float ry, float rz) {
//	float model[16] = {
//		1, 0, 0, 0,
//		0, 1, 0, 0,
//		0, 0, 1, 0,
//		0, 0, 0, 1
//	};
//
//	// 1. Translation Matrix
//	float T[16] = {
//		1, 0, 0, 0,
//		0, 1, 0, 0,
//		0, 0, 1, 0,
//		tx, ty, tz, 1
//	};
//
//	// 2. Scaling Matrix
//	float S[16] = {
//		sx, 0, 0, 0,
//		0, sy, 0, 0,
//		0, 0, sz, 0,
//		0, 0, 0, 1
//	};
//
//	// 3. Rotation X Matrix
//	float radX = (float)(rx * M_PI / 180.0);
//	float cX = cosf(radX);
//	float sX = sinf(radX);
//	float RX[16] = {
//		1,  0,   0,  0,
//		0,  cX,  sX, 0,
//		0, -sX,  cX, 0,
//		0,  0,   0,  1
//	};
//
//	// 4. Rotation Y Matrix
//	float radY = (float)(ry * M_PI / 180.0);
//	float cY = cosf(radY);
//	float sY = sinf(radY);
//	float RY[16] = {
//		 cY, 0, -sY, 0,
//		 0,  1,  0,  0,
//		 sY, 0,  cY, 0,
//		 0,  0,  0,  1
//	};
//
//	// 5. Rotation Z Matrix
//	float radZ = (float)(rz * M_PI / 180.0);
//	float cZ = cosf(radZ);
//	float sZ = sinf(radZ);
//	float RZ[16] = {
//		 cZ, sZ, 0, 0,
//		-sZ, cZ, 0, 0,
//		 0,  0,  1, 0,
//		 0,  0,  0, 1
//	};
//
//	// Final Matrix = T * S * RX * RY * RZ
//	float temp1[16], temp2[16], temp3[16];
//	matrixMultiply(T, S, temp1);
//	matrixMultiply(temp1, RX, temp2);
//	matrixMultiply(temp2, RY, temp3);
//	matrixMultiply(temp3, RZ, model);
//
//	// Load the final custom matrix into OpenGL
//	glMultMatrixf(model);
//}
//
//int main(int argc, char** argv)
//{
//	glutInit(&argc, argv);
//	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
//	glutInitWindowSize(800, 600);
//	glutInitWindowPosition(400, 80);
//	glutCreateWindow("SRT Homogeneous Matrix - Lab04");
//
//	glEnable(GL_DEPTH_TEST);
//
//	glutReshapeFunc(ChangeSize);
//	glutDisplayFunc(RenderScene);
//	glutIdleFunc(Idle);
//	glutKeyboardFunc(Keyboard);
//	glutMainLoop();
//	return 0;
//}
//
//void Idle(void)
//{
//	glutPostRedisplay();
//}
//
//void Keyboard(unsigned char key, int x, int y)
//{
//	float step = 0.5f;
//	float scaleStep = 0.5f;
//	float rotStep = 5.0f;
//	switch (key) {
//	// Movement
//	case 'q': objX -= step; break;
//	case 'Q': objX += step; break;
//	case 'w': objY -= step; break;
//	case 'W': objY += step; break;
//	case 'e': objZ -= step; break;
//	case 'E': objZ += step; break;
//	
//	// Separate Scaling
//	case 'u': objScaleX -= scaleStep; if (objScaleX < 0.1f) objScaleX = 0.1f; break;
//	case 'U': objScaleX += scaleStep; break;
//	case 'i': objScaleY -= scaleStep; if (objScaleY < 0.1f) objScaleY = 0.1f; break;
//	case 'I': objScaleY += scaleStep; break;
//	case 'o': objScaleZ -= scaleStep; if (objScaleZ < 0.1f) objScaleZ = 0.1f; break;
//	case 'O': objScaleZ += scaleStep; break;
//
//	// Rotation
//	case 'j': objRotX -= rotStep; break;
//	case 'J': objRotX += rotStep; break;
//	case 'k': objRotY -= rotStep; break;
//	case 'K': objRotY += rotStep; break;
//	case 'l': objRotZ -= rotStep; break;
//	case 'L': objRotZ += rotStep; break;
//
//	case 'r':
//	case 'R':
//		objX = 0.0f; objY = 0.0f; objZ = 0.0f;
//		objScaleX = 5.0f; objScaleY = 5.0f; objScaleZ = 5.0f;
//		objRotX = 0.0f; objRotY = 0.0f; objRotZ = 0.0f;
//		break;
//	case 27: // ESC
//		exit(0);
//		break;
//	}
//	glutPostRedisplay();
//}
//
//void ChangeSize(int w, int h)
//{
//	if (h == 0) h = 1;
//	glViewport(0, 0, w, h);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(-50, 50, -50, 50, -500, 500);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//}
//
//void DrawAxes()
//{
//	glBegin(GL_LINES);
//	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-50.0f, 0.0f, 0.0f); glVertex3f(50.0f, 0.0f, 0.0f);
//	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -50.0f, 0.0f); glVertex3f(0.0f, 50.0f, 0.0f);
//	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -50.0f); glVertex3f(0.0f, 0.0f, 100.0f);
//	glEnd();
//}
//
//void DrawLocalAxes()
//{
//	glLineWidth(2.0f);
//	glBegin(GL_LINES);
//	// X axis (Red)
//	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(2.0f, 0.0f, 0.0f);
//	// Y axis (Green)
//	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 2.0f, 0.0f);
//	// Z axis (Blue)
//	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 2.0f);
//	glEnd();
//	glLineWidth(1.0f);
//}
//
//void RenderText(float x, float y, const char* text)
//{
//	glRasterPos2f(x, y);
//	for (const char* c = text; *c != '\0'; c++) {
//		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
//	}
//}
//
//void RenderScene(void)
//{
//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	gluLookAt(15.0f, 15.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
//
//	DrawAxes();
//
//	glPushMatrix();
//	
//	// CUSTOM TRANSFORMATION PIPELINE
//	applyCustomTransformations(objX, objY, objZ, objScaleX, objScaleY, objScaleZ, objRotX, objRotY, objRotZ);
//
//	// Draw local axes for the triangle
//	DrawLocalAxes();
//
//	glBegin(GL_TRIANGLES);
//	// Front
//	glColor3f(0.7f, 0.7f, 0.7f); glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, 1.0f);
//	// Back
//	glColor3f(0.7f, 0.7f, 0.7f); glVertex3f(0.0f, 1.0f, -1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(-0.86f, -0.5f, -1.0f);
//	// Right
//	glColor3f(0.5f, 0.0f, 0.5f);
//	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f);
//	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(0.0f, 1.0f, -1.0f);
//	// Left
//	glColor3f(0.0f, 0.5f, 0.5f);
//	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.0f, 1.0f, -1.0f); glVertex3f(-0.86f, -0.5f, -1.0f);
//	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(-0.86f, -0.5f, -1.0f); glVertex3f(-0.86f, -0.5f, 1.0f);
//	// Bottom
//	glColor3f(0.5f, 0.0f, 0.0f);
//	glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(-0.86f, -0.5f, -1.0f); glVertex3f(0.86f, -0.5f, -1.0f);
//	glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(0.86f, -0.5f, 1.0f);
//	glEnd();
//	glPopMatrix();
//
//	// Fixed 2D UI Overlay
//	glMatrixMode(GL_PROJECTION);
//	glPushMatrix();
//	glLoadIdentity();
//	glOrtho(0, 800, 0, 600, -1, 1);
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix();
//	glLoadIdentity();
//	glColor3f(1.0f, 1.0f, 1.0f);
//	RenderText(10, 580, "Keybindings (Custom Matrix Implementation):");
//	RenderText(10, 560, "Q/q, W/w, E/e: Move Object");
//	RenderText(10, 540, "U/u, I/i, O/o: Scale Object (X/Y/Z)");
//	RenderText(10, 520, "J/j, K/k, L/l: Rotate Object (X/Y/Z)");
//	RenderText(10, 500, "R/r: Reset Object");
//
//	// Diagnostic data (Bottom Left)
//	char buf[64];
//	glColor3f(0.0f, 1.0f, 1.0f); // Cyan for diagnostics
//	sprintf_s(buf, "Scale:     [%.1f, %.1f, %.1f]", objScaleX, objScaleY, objScaleZ);
//	RenderText(10, 70, buf);
//	sprintf_s(buf, "Rotation:  [%.1f, %.1f, %.1f]", objRotX, objRotY, objRotZ);
//	RenderText(10, 50, buf);
//	sprintf_s(buf, "Translate: [%.1f, %.1f, %.1f]", objX, objY, objZ);
//	RenderText(10, 30, buf);
//
//	glPopMatrix();
//	glMatrixMode(GL_PROJECTION);
//	glPopMatrix();
//	glMatrixMode(GL_MODELVIEW);
//
//	glutSwapBuffers();
//}
