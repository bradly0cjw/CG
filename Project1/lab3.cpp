#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include ".\GL\freeglut.h"

// Object transformation variables
float objX = 0.0f;
float objY = 0.0f;
float objZ = 0.0f;
float objScale = 5.0f;
float objRotX = 0.0f;
float objRotY = 0.0f;
float objRotZ = 0.0f;

void ChangeSize(int, int);
void RenderScene(void);
void Idle(void);
void Keyboard(unsigned char key, int x, int y);

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(400, 80);
	glutCreateWindow("Object Transformation - Lab03");

	glEnable(GL_DEPTH_TEST);

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
	return 0;
}

void Idle(void)
{
	// No auto-rotation
	glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y)
{
	float step = 0.5f;
	float scaleStep = 0.5f;
	float rotStep = 5.0f;
	switch (key) {
	case 'x': objX -= step; break;
	case 'X': objX += step; break;
	case 'y': objY -= step; break;
	case 'Y': objY += step; break;
	case 'z': objZ -= step; break;
	case 'Z': objZ += step; break;
	case 's': objScale -= scaleStep; if (objScale < 0.1f) objScale = 0.1f; break;
	case 'S': objScale += scaleStep; break;
	case 'j': objRotX -= rotStep; break;
	case 'J': objRotX += rotStep; break;
	case 'k': objRotY -= rotStep; break;
	case 'K': objRotY += rotStep; break;
	case 'l': objRotZ -= rotStep; break;
	case 'L': objRotZ += rotStep; break;
	case 'r':
	case 'R':
		objX = 0.0f; objY = 0.0f; objZ = 0.0f;
		objScale = 5.0f;
		objRotX = 0.0f; objRotY = 0.0f; objRotZ = 0.0f;
		break;
	case 27: // ESC
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void ChangeSize(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Large clipping volume for orthographic view
	glOrtho(-50, 50, -50, 50, -500, 500);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void DrawAxes()
{
	glBegin(GL_LINES);
	// X-axis (Red)
	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-50.0f, 0.0f, 0.0f); glVertex3f(50.0f, 0.0f, 0.0f);
	// Y-axis (Green)
	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -50.0f, 0.0f); glVertex3f(0.0f, 50.0f, 0.0f);
	// Z-axis (Blue)
	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -50.0f); glVertex3f(0.0f, 0.0f, 50.0f);
	glEnd();
}

void RenderText(float x, float y, const char* text)
{
	glRasterPos2f(x, y);
	for (const char* c = text; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
	}
}

void RenderScene(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Fixed Camera at 45 degrees (15, 15, 15) facing 0,0,0
	gluLookAt(15.0f, 15.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	DrawAxes();

	glPushMatrix();
	// Apply object transformations: Translate -> Scale -> Rotate
	glTranslatef(objX, objY, objZ);
	glScalef(objScale, objScale, objScale);
	glRotatef(objRotX, 1.0f, 0.0f, 0.0f);
	glRotatef(objRotY, 0.0f, 1.0f, 0.0f);
	glRotatef(objRotZ, 0.0f, 0.0f, 1.0f);

	// Centered CM (Z from -1 to 1)
	glBegin(GL_TRIANGLES);
	// Front
	glColor3f(0.7f, 0.7f, 0.7f); glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, 1.0f);
	// Back
	glColor3f(0.7f, 0.7f, 0.7f); glVertex3f(0.0f, 1.0f, -1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(-0.86f, -0.5f, -1.0f);
	// Right
	glColor3f(0.5f, 0.0f, 0.5f);
	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f);
	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(0.0f, 1.0f, -1.0f);
	// Left
	glColor3f(0.0f, 0.5f, 0.5f);
	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.0f, 1.0f, -1.0f); glVertex3f(-0.86f, -0.5f, -1.0f);
	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(-0.86f, -0.5f, -1.0f); glVertex3f(-0.86f, -0.5f, 1.0f);
	// Bottom
	glColor3f(0.5f, 0.0f, 0.0f);
	glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(-0.86f, -0.5f, -1.0f); glVertex3f(0.86f, -0.5f, -1.0f);
	glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(0.86f, -0.5f, 1.0f);
	glEnd();
	glPopMatrix();

	// Fixed 2D UI Overlay
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);
	RenderText(10, 580, "Keybindings:");
	RenderText(10, 560, "X/x, Y/y, Z/z: Move Object");
	RenderText(10, 540, "S/s: Scale Object (Up/Down)");
	RenderText(10, 520, "J/j, K/k, L/l: Rotate Object (X/Y/Z)");
	RenderText(10, 500, "R/r: Reset Object");
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glutSwapBuffers();
}
