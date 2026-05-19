#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>

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
	bool showMidpoint = true;
	std::vector<Pixel> endpoints;
	std::vector<Pixel> linePixels;
} app;

// --- Forward Declarations ---
void ChangeSize(int w, int h);
void UpdateTriangle();

// --- Helper Functions ---

void ScreenToGrid(int mouseX, int mouseY, int &gridX, int &gridY) {
	float range = app.dimension + 0.5f;
	float worldX = ((float)mouseX / app.winW) * (2.0f * range) - range;
	float worldY = (1.0f - (float)mouseY / app.winH) * (2.0f * range) - range;
	gridX = (int)floor(worldX + 0.5f);
	gridY = (int)floor(worldY + 0.5f);
}

// --- Lab 07 Midpoint Line Functions ---

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

void AddPixel(int x, int y, const Color& c) {
	app.linePixels.push_back({x, y, c});
}

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

	Color cAxis, cDiag;
	GetRegionColors(region, cAxis, cDiag);

	int x = x0, y = y0;
	AddPixel(x, y, cAxis); // Start point

	if (region == 1) {
		int d = 2 * dy - dx, dE = 2 * dy, dNE = 2 * (dy - dx);
		while (x < x1) {
			if (d <= 0) { x++; d += dE; AddPixel(x, y, cAxis); }
			else { x++; y++; d += dNE; AddPixel(x, y, cDiag); }
		}
	} else if (region == 2) {
		int d = 2 * dx - dy, dN = 2 * dx, dNE = 2 * (dx - dy);
		while (y < y1) {
			if (d <= 0) { y++; d += dN; AddPixel(x, y, cAxis); }
			else { x++; y++; d += dNE; AddPixel(x, y, cDiag); }
		}
	} else if (region == 3) {
		int d = 2 * (-dx) - dy, dN = 2 * (-dx), dNW = 2 * (-dx - dy);
		while (y < y1) {
			if (d <= 0) { y++; d += dN; AddPixel(x, y, cAxis); }
			else { x--; y++; d += dNW; AddPixel(x, y, cDiag); }
		}
	} else if (region == 4) {
		int d = 2 * dy - (-dx), dW = 2 * dy, dNW = 2 * (dy - (-dx));
		while (x > x1) {
			if (d <= 0) { x--; d += dW; AddPixel(x, y, cAxis); }
			else { x--; y++; d += dNW; AddPixel(x, y, cDiag); }
		}
	} else if (region == 5) {
		int d = 2 * (-dy) - (-dx), dW = 2 * (-dy), dSW = 2 * ((-dy) - (-dx));
		while (x > x1) {
			if (d <= 0) { x--; d += dW; AddPixel(x, y, cAxis); }
			else { x--; y--; d += dSW; AddPixel(x, y, cDiag); }
		}
	} else if (region == 6) {
		int d = 2 * (-dx) - (-dy), dS = 2 * (-dx), dSW = 2 * ((-dx) - (-dy));
		while (y > y1) {
			if (d <= 0) { y--; d += dS; AddPixel(x, y, cAxis); }
			else { x--; y--; d += dSW; AddPixel(x, y, cDiag); }
		}
	} else if (region == 7) {
		int d = 2 * dx - (-dy), dS = 2 * dx, dSE = 2 * (dx - (-dy));
		while (y > y1) {
			if (d <= 0) { y--; d += dS; AddPixel(x, y, cAxis); }
			else { x++; y--; d += dSE; AddPixel(x, y, cDiag); }
		}
	} else if (region == 8) {
		int d = 2 * (-dy) - dx, dE = 2 * (-dy), dSE = 2 * ((-dy) - dx);
		while (x < x1) {
			if (d <= 0) { x++; d += dE; AddPixel(x, y, cAxis); }
			else { x++; y--; d += dSE; AddPixel(x, y, cDiag); }
		}
	}
}

// --- Half-Space Test Functions ---

int EdgeFunction(int x0, int y0, int x1, int y1, int x, int y) {
	return (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
}

void UpdateTriangle() {
	app.linePixels.clear();
	if (app.endpoints.size() < 3) return;

	int x0 = app.endpoints[0].x; int y0 = app.endpoints[0].y;
	int x1 = app.endpoints[1].x; int y1 = app.endpoints[1].y;
	int x2 = app.endpoints[2].x; int y2 = app.endpoints[2].y;

	printf("\n--- Rasterizing Triangle: (%d,%d), (%d,%d), (%d,%d) ---\n", x0, y0, x1, y1, x2, y2);

	Color interiorColor = {0.7f, 0.7f, 0.7f, "Interior (Light Grey)"};
	Color simpleEdgeColor = {0.0f, 0.6f, 0.9f, "Edge (Blue)"};

	int minX = std::min({x0, x1, x2});
	int maxX = std::max({x0, x1, x2});
	int minY = std::min({y0, y1, y2});
	int maxY = std::max({y0, y1, y2});

	minX = std::max(minX, -app.dimension); maxX = std::min(maxX, app.dimension);
	minY = std::max(minY, -app.dimension); maxY = std::min(maxY, app.dimension);

	for (int y = minY; y <= maxY; ++y) {
		for (int x = minX; x <= maxX; ++x) {
			int e0 = EdgeFunction(x0, y0, x1, y1, x, y);
			int e1 = EdgeFunction(x1, y1, x2, y2, x, y);
			int e2 = EdgeFunction(x2, y2, x0, y0, x, y);

			if ((e0 >= 0 && e1 >= 0 && e2 >= 0) || (e0 <= 0 && e1 <= 0 && e2 <= 0)) {
				if (!app.showMidpoint && (e0 == 0 || e1 == 0 || e2 == 0)) {
					app.linePixels.push_back({x, y, simpleEdgeColor});
				} else {
					app.linePixels.push_back({x, y, interiorColor});
				}
			}
		}
	}

	if (app.showMidpoint) {
		MidpointLine(x0, y0, x1, y1);
		MidpointLine(x1, y1, x2, y2);
		MidpointLine(x2, y2, x0, y0);
	}

	printf("Triangle rasterized (Midpoint: %s). Total pixels: %zu\n", app.showMidpoint ? "ON" : "OFF", app.linePixels.size());
}


// Check if the cell is inside of triangle or not
void IOTriangleCheck(int x, int y) {
	if (app.endpoints.size()<3) {
		printf("Please select 3 vertices to form a triangle first.\n");
		return;
	}
	Color insideColor = {0.0f, 1.0f, 1.0f, "Inside (Cyan)"};
	Color outsideColor = {1.0f, 1.0f, 0.0f, "Outside (Yellow)"};
	int x0 = app.endpoints[0].x; int y0 = app.endpoints[0].y;
	int x1 = app.endpoints[1].x; int y1 = app.endpoints[1].y;
	int x2 = app.endpoints[2].x; int y2 = app.endpoints[2].y;
	int e0 = EdgeFunction(x0, y0, x1, y1, x, y);
	int e1 = EdgeFunction(x1, y1, x2, y2, x, y);
	int e2 = EdgeFunction(x2, y2, x0, y0, x, y);
	if ((e0 >= 0 && e1 >= 0 && e2 >= 0) || (e0 <= 0 && e1 <= 0 && e2 <= 0)) {
		app.linePixels.push_back({x, y, insideColor});
		printf("Cell (%d, %d) is INSIDE the triangle.\n", x, y);
		
	} else {
		app.linePixels.push_back({x, y, outsideColor});
		printf("Cell (%d, %d) is OUTSIDE the triangle.\n", x, y);
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
		printf("Clicked Cell: (%d, %d)\n", gx, gy);

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
				if (app.endpoints.size() >= 3) app.endpoints.clear();
				app.endpoints.push_back({gx, gy, {1.0f, 0.0f, 0.0f, "Endpoint"}});
			}
			UpdateTriangle(); glutPostRedisplay();
		}
	}
}

void Keyboard(unsigned char key, int x, int y) {
	if (key == 'r' || key == 'R') {
		app.endpoints.clear(); app.linePixels.clear();
		printf("\n--- Canvas Reset ---\n");
	} else if (key == 'm' || key == 'M') {
		app.showMidpoint = !app.showMidpoint;
		printf("Midpoint Edge Display: %s\n", app.showMidpoint ? "ENABLED" : "DISABLED");
		UpdateTriangle();
	} else if (key == 'g'){
		// select cell at mouse position
		int gx, gy; ScreenToGrid(x, y, gx, gy);
		printf("Keyboard 'g' pressed at Cell: (%d, %d)\n",
			gx, gy);
		IOTriangleCheck(gx, gy);

	}

	glutPostRedisplay();
}

void ProcessMenu(int value) {
	if (value == 0) {
		app.endpoints.clear(); app.linePixels.clear();
		printf("\n--- Points Cleared ---\n");
	} else if (value == 1) {
		app.showMidpoint = !app.showMidpoint;
		printf("Midpoint Edge Display: %s\n", app.showMidpoint ? "ENABLED" : "DISABLED");
		UpdateTriangle();
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
	glutCreateWindow("Lab 08: Triangle Rasterization (Half-Space Test)");

	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Clear All Points", 0);
	glutAddMenuEntry("Toggle Midpoint Edges", 1);
	glutAddMenuEntry("Dimension 10 x 10", 10);
	glutAddMenuEntry("Dimension 15 x 15", 15);
	glutAddMenuEntry("Dimension 20 x 20", 20);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	printf("Lab 08: Triangle Rasterization (Half-Space Test) Loaded.\n");
	printf("Controls: Left-click: Select 3 Vertices, 'm': Toggle Midpoint Edges, 'r': Reset.\n");
	glutMainLoop();
	return 0;
}