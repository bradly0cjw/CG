#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>

#include ".\GL\freeglut.h"

// --- Constants & Data Structures ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Color {
	float r, g, b;
	std::string name;
};

struct Pixel {
	int x, y;
	Color color;
};

struct GridApp {
	int dimension = 10;
	int winW = 1000, winH = 1000;
	std::vector<Pixel> endpoints;
	std::vector<Pixel> linePixels;
} app;

// --- Helper Functions ---

void ScreenToGrid(int mouseX, int mouseY, int &gridX, int &gridY) {
	float range = app.dimension + 0.5f;
	float worldX = ((float)mouseX / app.winW) * (2.0f * range) - range;
	float worldY = (1.0f - (float)mouseY / app.winH) * (2.0f * range) - range;
	gridX = (int)floor(worldX + 0.5f);
	gridY = (int)floor(worldY + 0.5f);
}

void GetRegionColors(int region, Color& axis, Color& diag) {
	if (region == 1 || region == 2) {
		axis = {0.0f, 1.0f, 0.0f, "Bright Green"};
		diag = {0.0f, 0.0f, 1.0f, "Dark Blue"};
	} else if (region == 3 || region == 4) {
		axis = {1.0f, 1.0f, 0.0f, "Bright Yellow"};
		diag = {0.5f, 0.5f, 0.0f, "Dark Yellow"};
	} else if (region == 5 || region == 6) {
		axis = {1.0f, 0.0f, 1.0f, "Bright Magenta"};
		diag = {0.5f, 0.0f, 0.5f, "Dark Magenta"};
	} else {
		axis = {0.0f, 1.0f, 1.0f, "Bright Cyan"};
		diag = {0.0f, 0.5f, 0.5f, "Dark Cyan"};
	}
}

void AddPixel(int x, int y, const Color& c, const char* step) {
	app.linePixels.push_back({x, y, c});
	printf("Pixel (%d, %d): %s (%s)\n", x, y, step, c.name.c_str());
}

// --- Midpoint Algorithm ---

void MidpointLine(int x0, int y0, int x1, int y1) {
	int dx = x1 - x0, dy = y1 - y0;
	int region = 0;

	if (abs(dx) >= abs(dy)) {
		if (dx >= 0 && dy >= 0) region = 1;
		else if (dx < 0 && dy >= 0) region = 4;
		else if (dx < 0 && dy < 0) region = 5;
		else if (dx >= 0 && dy < 0) region = 8;
	} else {
		if (dx >= 0 && dy >= 0) region = 2;
		else if (dx < 0 && dy >= 0) region = 3;
		else if (dx < 0 && dy < 0) region = 6;
		else if (dx >= 0 && dy < 0) region = 7;
	}

	printf("--- Drawing Line: (%d, %d) to (%d, %d) [Region %d] ---\n", x0, y0, x1, y1, region);

	Color cAxis, cDiag;
	GetRegionColors(region, cAxis, cDiag);

	int x = x0, y = y0;

	if (region == 1) {
		int d = 2 * dy - dx, dE = 2 * dy, dNE = 2 * (dy - dx);
		while (x < x1) {
			if (d <= 0) { x++; d += dE; AddPixel(x, y, cAxis, "E"); }
			else { x++; y++; d += dNE; AddPixel(x, y, cDiag, "NE"); }
		}
	} else if (region == 2) {
		int d = 2 * dx - dy, dN = 2 * dx, dNE = 2 * (dx - dy);
		while (y < y1) {
			if (d <= 0) { y++; d += dN; AddPixel(x, y, cAxis, "N"); }
			else { x++; y++; d += dNE; AddPixel(x, y, cDiag, "NE"); }
		}
	} else if (region == 3) {
		int d = 2 * (-dx) - dy, dN = 2 * (-dx), dNW = 2 * (-dx - dy);
		while (y < y1) {
			if (d <= 0) { y++; d += dN; AddPixel(x, y, cAxis, "N"); }
			else { x--; y++; d += dNW; AddPixel(x, y, cDiag, "NW"); }
		}
	} else if (region == 4) {
		int d = 2 * dy - (-dx), dW = 2 * dy, dNW = 2 * (dy - (-dx));
		while (x > x1) {
			if (d <= 0) { x--; d += dW; AddPixel(x, y, cAxis, "W"); }
			else { x--; y++; d += dNW; AddPixel(x, y, cDiag, "NW"); }
		}
	} else if (region == 5) {
		int d = 2 * (-dy) - (-dx), dW = 2 * (-dy), dSW = 2 * ((-dy) - (-dx));
		while (x > x1) {
			if (d <= 0) { x--; d += dW; AddPixel(x, y, cAxis, "W"); }
			else { x--; y--; d += dSW; AddPixel(x, y, cDiag, "SW"); }
		}
	} else if (region == 6) {
		int d = 2 * (-dx) - (-dy), dS = 2 * (-dx), dSW = 2 * ((-dx) - (-dy));
		while (y > y1) {
			if (d <= 0) { y--; d += dS; AddPixel(x, y, cAxis, "S"); }
			else { x--; y--; d += dSW; AddPixel(x, y, cDiag, "SW"); }
		}
	} else if (region == 7) {
		int d = 2 * dx - (-dy), dS = 2 * dx, dSE = 2 * (dx - (-dy));
		while (y > y1) {
			if (d <= 0) { y--; d += dS; AddPixel(x, y, cAxis, "S"); }
			else { x++; y--; d += dSE; AddPixel(x, y, cDiag, "SE"); }
		}
	} else if (region == 8) {
		int d = 2 * (-dy) - dx, dE = 2 * (-dy), dSE = 2 * ((-dy) - dx);
		while (x < x1) {
			if (d <= 0) { x++; d += dE; AddPixel(x, y, cAxis, "E"); }
			else { x++; y--; d += dSE; AddPixel(x, y, cDiag, "SE"); }
		}
	}
	printf("---------------------------------------------------\n");
}

void UpdateLinePixels() {
	app.linePixels.clear();
	if (app.endpoints.size() < 2) return;

	printf("\n--- Recalculating Path for %zu Vertices ---\n", app.endpoints.size());
	for (size_t i = 0; i < app.endpoints.size() - 1; ++i) {
		MidpointLine(app.endpoints[i].x, app.endpoints[i].y, app.endpoints[i+1].x, app.endpoints[i+1].y);
	}

	if (app.endpoints.size() >= 3) {
		MidpointLine(app.endpoints.back().x, app.endpoints.back().y, app.endpoints.front().x, app.endpoints.front().y);
	}
}

// --- Rendering Functions ---

void DrawGrid() {
	glLineWidth(1.0f);
	glColor3f(0.4f, 0.4f, 0.4f);
	glBegin(GL_LINES);
	for (int i = -app.dimension; i <= app.dimension + 1; ++i) {
		float pos = (float)i - 0.5f;
		glVertex2f(pos, -app.dimension - 0.5f); glVertex2f(pos, app.dimension + 0.5f);
		glVertex2f(-app.dimension - 0.5f, pos); glVertex2f(app.dimension + 0.5f, pos);
	}
	glEnd();

	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.2f, 0.2f); glVertex2f(-app.dimension - 0.5f, 0.0f); glVertex2f(app.dimension + 0.5f, 0.0f);
	glColor3f(0.2f, 1.0f, 0.2f); glVertex2f(0.0f, -app.dimension - 0.5f); glVertex2f(0.0f, app.dimension + 0.5f);
	glEnd();
}

void DrawScene() {
	glBegin(GL_QUADS);
	for (const auto& p : app.linePixels) {
		glColor3f(p.color.r, p.color.g, p.color.b);
		glVertex2f(p.x - 0.5f, p.y - 0.5f); glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f); glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	for (const auto& p : app.endpoints) {
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(p.x - 0.5f, p.y - 0.5f); glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f); glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	glEnd();

	if (!app.endpoints.empty()) {
		glLineWidth(2.0f); glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_STRIP);
		for (const auto& p : app.endpoints) glVertex2f((float)p.x, (float)p.y);
		if (app.endpoints.size() >= 3) glVertex2f((float)app.endpoints[0].x, (float)app.endpoints[0].y);
		glEnd();
	}
}

// --- GLUT Callbacks ---

void RenderScene() {
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	DrawScene();
	DrawGrid();
	glutSwapBuffers();
}

void ChangeSize(int w, int h) {
	app.winW = w; app.winH = (h == 0) ? 1 : h;
	glViewport(0, 0, app.winW, app.winH);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	float r = app.dimension + 0.5f;
	glOrtho(-r, r, -r, r, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void Mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		int gx, gy; ScreenToGrid(x, y, gx, gy);
		bool found = false;
		if (abs(gx) <= app.dimension && abs(gy) <= app.dimension) {
			for(size_t i = 0; i < app.endpoints.size(); ++i) {
				if (app.endpoints[i].x == gx && app.endpoints[i].y == gy) {
					app.endpoints.erase(app.endpoints.begin() + i);
					found = true;
					break;
				}
			}
			if (!found){
				app.endpoints.push_back({gx, gy, {1.0f, 0.0f, 0.0f, "Endpoint"}});
			}
			UpdateLinePixels(); glutPostRedisplay();
		}
	}
}

void Keyboard(unsigned char key, int x, int y) {
	if (key == 'r' || key == 'R') {
		app.endpoints.clear(); app.linePixels.clear();
		printf("\n--- Canvas Reset ---\n"); glutPostRedisplay();
	}
}

void ProcessMenu(int value) {
	if (value == 0) {
		app.endpoints.clear(); app.linePixels.clear();
		printf("\n--- Points Cleared ---\n");
	} else {
		app.dimension = value; app.endpoints.clear(); app.linePixels.clear();
		ChangeSize(app.winW, app.winH);
		printf("\n--- Grid Dimension: %d x %d ---\n", value, value);
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(app.winW, app.winH);
	glutCreateWindow("Lab 07: Refactored Midpoint Algorithm");

	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Clear All Points", 0);
	glutAddMenuEntry("Dimension 10 x 10", 10);
	glutAddMenuEntry("Dimension 15 x 15", 15);
	glutAddMenuEntry("Dimension 20 x 20", 20);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	printf("Refactored Midpoint Lab Loaded.\nControls: Left-click: Add Points, 'r': Reset, Right-click: Menu.\n");
	glutMainLoop();
	return 0;
}
