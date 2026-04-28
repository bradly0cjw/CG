#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include ".\GL\freeglut.h"

void ChangeSize(int, int);
void RenderScene(void);
void Menu(int index);
void SetupLighting();

int _shadeModel = GL_FLAT;

void Menu(int index) {
	switch (index) {
	case 1: _shadeModel = GL_SMOOTH; break;
	case 2: _shadeModel = GL_FLAT; break;
	}
	printf("%i", index);
	glutPostRedisplay();
}

void SetupLighting() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat lightPos[] = { -5.0f, 8.0f, 8.0f, 1.0f };

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
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(400, 80);
	glutCreateWindow("Voxel Character");

	glutCreateMenu(Menu);
	glutAddMenuEntry("smooth", 1);
	glutAddMenuEntry("flat", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glEnable(GL_DEPTH_TEST);
	SetupLighting();

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

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
	if (_shadeModel == GL_FLAT)
		glClearColor(0.20f, 0.20f, 0.32f, 1.0f);
	else
		glClearColor(0.85f, 0.85f, 0.9f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glShadeModel(_shadeModel);
	glEnable(GL_LIGHTING); // Enable lighting for 3D character

	if (_shadeModel == GL_FLAT)
	{
		glDisable(GL_LIGHTING);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(-4.0, 4.0, -3.0, 3.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Head
		glColor3f(0.90f, 0.50f, 0.12f);
		glBegin(GL_POLYGON);
		glVertex2f(-1.2f, 1.35f);
		glVertex2f(1.2f, 1.35f);
		glVertex2f(1.6f, 0.4f);
		glVertex2f(1.15f, -0.5f);
		glVertex2f(-1.15f, -0.5f);
		glVertex2f(-1.6f, 0.4f);
		glEnd();

		// Ears
		glColor3f(0.90f, 0.50f, 0.12f);
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.8f, 1.35f);
		glVertex2f(-1.45f, 2.2f);
		glVertex2f(-0.2f, 1.35f);
		glVertex2f(0.8f, 1.35f);
		glVertex2f(0.2f, 1.35f);
		glVertex2f(1.45f, 2.2f);
		glEnd();

		// Eyes
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex2f(-0.85f, 0.75f);
		glVertex2f(-0.35f, 0.75f);
		glVertex2f(-0.35f, 0.35f);
		glVertex2f(-0.85f, 0.35f);
		glVertex2f(0.35f, 0.75f);
		glVertex2f(0.85f, 0.75f);
		glVertex2f(0.85f, 0.35f);
		glVertex2f(0.35f, 0.35f);
		glEnd();

		glColor3f(0.1f, 0.1f, 0.1f);
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.62f, 0.62f);
		glVertex2f(-0.44f, 0.42f);
		glVertex2f(-0.80f, 0.42f);
		glVertex2f(0.62f, 0.62f);
		glVertex2f(0.80f, 0.42f);
		glVertex2f(0.44f, 0.42f);
		glEnd();

		// Snout and nose
		glColor3f(0.98f, 0.93f, 0.80f);
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.8f, -0.1f);
		glVertex2f(0.8f, -0.1f);
		glVertex2f(0.0f, -0.9f);
		glEnd();

		glColor3f(0.1f, 0.1f, 0.1f);
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.16f, -0.25f);
		glVertex2f(0.16f, -0.25f);
		glVertex2f(0.0f, -0.45f);
		glEnd();

		// Body
		glColor3f(0.16f, 0.65f, 0.45f);
		glBegin(GL_POLYGON);
		glVertex2f(-0.8f, -1.2f);
		glVertex2f(0.8f, -1.2f);
		glVertex2f(1.1f, -2.7f);
		glVertex2f(-1.1f, -2.7f);
		glEnd();

		// Tie
		glColor3f(0.0f, 0.45f, 0.45f);
		glBegin(GL_TRIANGLES);
		glVertex2f(0.0f, -1.2f);
		glVertex2f(0.25f, -2.55f);
		glVertex2f(-0.25f, -2.55f);
		glEnd();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	else
	{
		// Camera positioning
		gluLookAt(0, 1.0f, 8.0f,
			0, 0, 0,
			0, 1, 0);

		// Slight rotation for dimension
		glRotatef(20.0f, 0.0f, 1.0f, 0.0f);

		// ========== BODY (Green) ==========
		glPushMatrix();
		glTranslatef(0.0f, -0.8f, 0.0f);
		glColor3f(0.18f, 0.55f, 0.34f);
		glPushMatrix();
		glScalef(1.0f, 1.2f, 0.8f);
		glutSolidCube(1.0);
		glPopMatrix();
		glPopMatrix();

		// ========== HEAD (Orange) ==========
		glPushMatrix();
		glTranslatef(0.0f, 0.5f, 0.0f);

		// Main head block
		glColor3f(1.0f, 0.65f, 0.0f);
		glPushMatrix();
		glScalef(1.6f, 1.4f, 1.4f);
		glutSolidCube(1.0);
		glPopMatrix();

		// Left ear
		glColor3f(0.95f, 0.6f, 0.0f);
		glPushMatrix();
		glTranslatef(-0.7f, 0.8f, 0.0f);
		glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.4f, 0.6f, 0.4f);
		glutSolidCube(1.0);
		glPopMatrix();

		// Right ear
		glPushMatrix();
		glTranslatef(0.7f, 0.8f, 0.0f);
		glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
		glScalef(0.4f, 0.6f, 0.4f);
		glutSolidCube(1.0);
		glPopMatrix();

		// Left eye
		glColor3f(0.1f, 0.1f, 0.1f);
		glPushMatrix();
		glTranslatef(-0.4f, 0.2f, 0.75f);
		glScalef(0.2f, 0.25f, 0.15f);
		glutSolidCube(1.0);
		glPopMatrix();

		// Right eye
		glPushMatrix();
		glTranslatef(0.4f, 0.2f, 0.75f);
		glScalef(0.2f, 0.25f, 0.15f);
		glutSolidCube(1.0);
		glPopMatrix();

		// Snout (tan/beige)
		glColor3f(1.0f, 0.9f, 0.7f);
		glPushMatrix();
		glTranslatef(0.0f, -0.3f, 0.8f);
		glScalef(0.8f, 0.5f, 0.6f);
		glutSolidCube(1.0);
		glPopMatrix();

		// Nose (black)
		glColor3f(0.1f, 0.1f, 0.1f);
		glPushMatrix();
		glTranslatef(0.0f, -0.15f, 1.15f);
		glScalef(0.25f, 0.2f, 0.15f);
		glutSolidCube(1.0);
		glPopMatrix();

		glPopMatrix();
	}

	glutSwapBuffers();
}
