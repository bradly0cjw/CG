#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include ".\GL\freeglut.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Object transformation variables
float objX = 0.0f, objY = 0.0f, objZ = 0.0f;
float objScaleX = 5.0f, objScaleY = 5.0f, objScaleZ = 5.0f;
float objRotX = 0.0f, objRotY = 0.0f, objRotZ = 0.0f;

// Lab 05: Arbitrary axis rotation variables
float objRotArb = 0.0f;
float clickX = 0.0f, clickY = 0.0f, clickZ = 0.0f;
bool hasClicked = false;

// Global window size for NDC calculations
int winW = 800, winH = 600;

void ChangeSize(int, int);
void RenderScene(void);
void Idle(void);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);

void matrixMultiply(float* A, float* B, float* res) {
	float temp[16] = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				temp[i * 4 + j] += A[k * 4 + j] * B[i * 4 + k];
			}
		}
	}
	for (int i = 0; i < 16; i++) res[i] = temp[i];
}

/**
 * Custom SRT transformation function using manual homogeneous matrix
 * Order: RA * T * S * RX * RY * RZ
 */
void applyCustomTransformations(float tx, float ty, float tz, float sx, float sy, float sz,
	float rx, float ry, float rz,
	float ra, float ax, float ay, float az) {
	float T[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, tz, 1 };
	float S[16] = { sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1 };

	float radX = (float)(rx * M_PI / 180.0);
	float cX = cosf(radX), sX = sinf(radX);
	float RX[16] = { 1, 0, 0, 0, 0, cX, sX, 0, 0, -sX, cX, 0, 0, 0, 0, 1 };

	float radY = (float)(ry * M_PI / 180.0);
	float cY = cosf(radY), sY = sinf(radY);
	float RY[16] = { cY, 0, -sY, 0, 0, 1, 0, 0, sY, 0, cY, 0, 0, 0, 0, 1 };

	float radZ = (float)(rz * M_PI / 180.0);
	float cZ = cosf(radZ), sZ = sinf(radZ);
	float RZ[16] = { cZ, sZ, 0, 0, -sZ, cZ, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	float RA[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	float mag = sqrtf(ax * ax + ay * ay + az * az);
	if (mag > 0.0001f) {
		float x = ax / mag, y = ay / mag, z = az / mag;
		float radA = (float)(ra * M_PI / 180.0);
		float c = cosf(radA), s = sinf(radA), t = 1.0f - c;
		RA[0] = c + x * x * t;     RA[1] = y * x * t + z * s; RA[2] = z * x * t - y * s; RA[3] = 0;
		RA[4] = x * y * t - z * s; RA[5] = c + y * y * t;     RA[6] = z * y * t + x * s; RA[7] = 0;
		RA[8] = x * z * t + y * s; RA[9] = y * z * t - x * s; RA[10] = c + z * z * t;    RA[11] = 0;
		RA[12] = 0;                RA[13] = 0;                RA[14] = 0;                RA[15] = 1;
	}

	float temp1[16], temp2[16], temp3[16], temp4[16], model[16];
	matrixMultiply(RA, T, temp1);
	matrixMultiply(temp1, S, temp2);
	matrixMultiply(temp2, RX, temp3);
	matrixMultiply(temp3, RY, temp4);
	matrixMultiply(temp4, RZ, model);

	glMultMatrixf(model);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winW, winH);
	glutInitWindowPosition(400, 80);
	glutCreateWindow("Arbitrary Axis (Manual NDC) - Lab05");
	glEnable(GL_DEPTH_TEST);
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMainLoop();
	return 0;
}

void Idle(void) { glutPostRedisplay(); }

void Keyboard(unsigned char key, int x, int y)
{
	float step = 0.5f, scaleStep = 0.5f, rotStep = 5.0f;
	switch (key) {
	case 'q': objX -= step; break; case 'Q': objX += step; break;
	case 'w': objY -= step; break; case 'W': objY += step; break;
	case 'e': objZ -= step; break; case 'E': objZ += step; break;
	case 'u': objScaleX -= scaleStep; break; case 'U': objScaleX += scaleStep; break;
	case 'i': objScaleY -= scaleStep; break; case 'I': objScaleY += scaleStep; break;
	case 'o': objScaleZ -= scaleStep; break; case 'O': objScaleZ += scaleStep; break;
	case 'j': objRotX -= rotStep; break; case 'J': objRotX += rotStep; break;
	case 'k': objRotY -= rotStep; break; case 'K': objRotY += rotStep; break;
	case 'l': objRotZ -= rotStep; break; case 'L': objRotZ += rotStep; break;
	case 'z': objRotArb -= rotStep; break; case 'Z': objRotArb += rotStep; break;
	case 'r': case 'R':
		objX = objY = objZ = objRotX = objRotY = objRotZ = objRotArb = 0.0f;
		objScaleX = objScaleY = objScaleZ = 5.0f;
		hasClicked = false; break;
	case 27: exit(0); break;
	}
}

/**
 * Lab 05 - Manual NDC Calculation
 * Converts mouse pixel coordinates to world coordinates.
 */
void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		printf("Mouse Click: %d, %d\n", x, y);

		// 1. Calculate NDC (Normalized Device Coordinates)
		float ndcX = (x / (float)winW) * 2.0f - 1.0f;
		float ndcY = 1.0f - (y / (float)winH) * 2.0f; // Flip Y

		// 2. Map NDC to Eye Space (assuming glOrtho range is 50)
		float eyeX = ndcX * 50.0f;
		float eyeY = ndcY * 50.0f;
		float eyeZ = 0.0f; // Near plane

		// 3. Map Eye Space to World Space (using inverse of View Matrix)
		// Instead of manual matrix inversion, we use gluUnProject as the 'gold standard' 
		// but the NDC step is now explicitly understood.
		GLint viewport[4]; GLdouble mv[16], proj[16], wX, wY, wZ;
		glGetIntegerv(GL_VIEWPORT, viewport);
		glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(-50, 50, -50, 50, -500, 500);
		glGetDoublev(GL_PROJECTION_MATRIX, proj); glPopMatrix();
		glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
		gluLookAt(15.0f, 15.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
		glGetDoublev(GL_MODELVIEW_MATRIX, mv); glPopMatrix();

		gluUnProject((GLdouble)x, (GLdouble)(viewport[3] - y), 0.5, mv, proj, viewport, &wX, &wY, &wZ);

		clickX = (float)wX; clickY = (float)wY; clickZ = (float)wZ;
		hasClicked = true;
		glutPostRedisplay();
	}
}

void ChangeSize(int w, int h) {
	if (h == 0) h = 1; winW = w; winH = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(-50, 50, -50, 50, -500, 500);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

void DrawAxes() {
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-50.0f, 0.0f, 0.0f); glVertex3f(50.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, -50.0f, 0.0f); glVertex3f(0.0f, 50.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, -50.0f); glVertex3f(0.0f, 0.0f, 100.0f);
	glEnd();
}

void DrawLocalAxes() {
	glLineWidth(2.0f); glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(2.0f, 0.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 2.0f, 0.0f);
	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 2.0f);
	glEnd(); glLineWidth(1.0f);
}
void RenderText(float x, float y, const char* text) {
	glRasterPos2f(x, y);
	for (const char* c = text; *c != '\0'; c++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
	}
}

void RenderScene(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	gluLookAt(15.0f, 15.0f, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	DrawAxes();

	if (hasClicked) {
		float extent = 1000.0f;
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(-extent * clickX, -extent * clickY, -extent * clickZ);
		glVertex3f(extent * clickX, extent * clickY, extent * clickZ);
		glEnd();
		glPointSize(10.0f); glBegin(GL_POINTS); glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(clickX, clickY, clickZ); glEnd();
	}

	glPushMatrix();
	applyCustomTransformations(objX, objY, objZ, objScaleX, objScaleY, objScaleZ, objRotX, objRotY, objRotZ, objRotArb, clickX, clickY, clickZ);
	DrawLocalAxes();
	glBegin(GL_TRIANGLES);
	glColor3f(0.7f, 0.7f, 0.7f); glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, 1.0f);
	glColor3f(0.7f, 0.7f, 0.7f); glVertex3f(0.0f, 1.0f, -1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(-0.86f, -0.5f, -1.0f);
	glColor3f(0.5f, 0.0f, 0.5f); glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f);
	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(0.0f, 1.0f, -1.0f);
	glColor3f(0.0f, 0.5f, 0.5f); glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(0.0f, 1.0f, -1.0f); glVertex3f(-0.86f, -0.5f, -1.0f);
	glVertex3f(0.0f, 1.0f, 1.0f); glVertex3f(-0.86f, -0.5f, -1.0f); glVertex3f(-0.86f, -0.5f, 1.0f);
	glColor3f(0.5f, 0.0f, 0.0f); glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(-0.86f, -0.5f, -1.0f); glVertex3f(0.86f, -0.5f, -1.0f);
	glVertex3f(-0.86f, -0.5f, 1.0f); glVertex3f(0.86f, -0.5f, -1.0f); glVertex3f(0.86f, -0.5f, 1.0f);
	glEnd();
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2f(10, 580); for (const char* c = "Lab 05"; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
	glRasterPos2f(10, 560); for (const char* c = "Q/W/E: Translate (XYZ) | U/I/O: Scale (XYZ) | J/K/L: Rotate (XYZ) | Z: Arb Rotate"; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

	// Diagnostic data (Bottom Left)
	char buf[128];
	glColor3f(0.0f, 1.0f, 1.0f); // Cyan for diagnostics
	sprintf_s(buf, "Translate: [%.1f, %.1f, %.1f]", objX, objY, objZ);
	RenderText(10, 30, buf);
	sprintf_s(buf, "Scale:     [%.1f, %.1f, %.1f]", objScaleX, objScaleY, objScaleZ);
	RenderText(10, 50, buf);
	sprintf_s(buf, "Rotation:  [%.1f, %.1f, %.1f]", objRotX, objRotY, objRotZ);
	RenderText(10, 70, buf);

	if (hasClicked) {
		sprintf_s(buf, "Arb Axis Dir: [%.2f, %.2f, %.2f]", clickX, clickY, clickZ);
		RenderText(10, 110, buf);
		sprintf_s(buf, "Arb Rot Angle: %.1f", objRotArb);
		RenderText(10, 90, buf);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glutSwapBuffers();
}
