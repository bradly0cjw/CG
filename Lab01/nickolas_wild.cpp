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

	glClearColor(0.85f, 0.85f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glShadeModel(_shadeModel);

	if (_shadeModel == GL_FLAT)
	{
		glDisable(GL_LIGHTING);   // Disable lighting for 2D look

		// Camera positioning - looking directly at the XY plane
		gluLookAt(0, 0, 10.0f,
			0, 0, 0,
			0, 1, 0);

		glBegin(GL_TRIANGLES);

		// ========== TAIL (Orange) ==========
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex2f(1.0f, -1.5f);
		glVertex2f(2.5f, -0.5f);
		glVertex2f(1.2f, -2.0f);

		// ========== TAIL TIP (White) ==========
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex2f(2.5f, -0.5f);
		glVertex2f(2.1f, -0.7f);
		glVertex2f(2.3f, -1.0f);

		// ========== BODY (Orange with color variation) ==========
		glColor3f(1.0f, 0.4f, 0.0f); // Darker orange
		glVertex2f(0.0f, 0.2f);
		glColor3f(1.0f, 0.6f, 0.1f); // Lighter orange
		glVertex2f(-1.5f, -2.5f);
		glColor3f(1.0f, 0.5f, 0.0f); // Main orange
		glVertex2f(1.5f, -2.5f);

		// ========== CHEST (White) ==========
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex2f(0.0f, 0.2f);
		glVertex2f(-0.5f, -1.5f);
		glVertex2f(0.5f, -1.5f);

		// ========== EARS (Orange) ==========
		glColor3f(1.0f, 0.5f, 0.0f);
		// Left Ear
		glVertex2f(-0.9f, 1.3f);
		glVertex2f(-0.5f, 1.4f);
		glVertex2f(-0.8f, 2.0f);
		// Right Ear
		glVertex2f(0.9f, 1.3f);
		glVertex2f(0.5f, 1.4f);
		glVertex2f(0.8f, 2.0f);

		// ========== HEAD (Orange with color variation) ==========
		glColor3f(1.0f, 0.6f, 0.2f); // Light orange
		glVertex2f(0.0f, 0.2f);
		glColor3f(1.0f, 0.4f, 0.0f); // Darker orange
		glVertex2f(-1.0f, 1.5f);
		glColor3f(1.0f, 0.5f, 0.0f); // Main orange
		glVertex2f(1.0f, 1.5f);

		// ========== CHEEKS (White) ==========
		glColor3f(1.0f, 1.0f, 1.0f);
		// Left Cheek
		glVertex2f(0.0f, 0.2f);
		glVertex2f(-1.0f, 1.5f);
		glVertex2f(-0.2f, 0.7f);
		// Right Cheek
		glVertex2f(0.0f, 0.2f);
		glVertex2f(1.0f, 1.5f);
		glVertex2f(0.2f, 0.7f);

		// ========== EYES (Black) ==========
		glColor3f(0.0f, 0.0f, 0.0f);
		// Left Eye
		glVertex2f(-0.4f, 1.0f);
		glVertex2f(-0.3f, 1.1f);
		glVertex2f(-0.2f, 1.0f);
		// Right Eye
		glVertex2f(0.4f, 1.0f);
		glVertex2f(0.3f, 1.1f);
		glVertex2f(0.2f, 1.0f);

		// ========== NOSE (Black) ==========
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex2f(0.0f, 0.2f);
		glVertex2f(-0.1f, 0.35f);
		glVertex2f(0.1f, 0.35f);

		glEnd();
	}
	else
	{
		glEnable(GL_LIGHTING); // Enable lighting for 3D character

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
