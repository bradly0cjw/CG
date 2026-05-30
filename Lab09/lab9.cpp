#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

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
	int dimension = 15;
	int winW = 800, winH = 800;
	bool isFilled = true;
	bool showMidpoint = true;
	std::vector<Pixel> vertices;
	std::vector<Pixel> fillPixels;
	std::vector<Pixel> edgePixels;
	
	// Jordan Curve Check State
	int checkX = -100, checkY = -100;
	bool showCheck = false;
} app;

// --- Forward Declarations ---
void ChangeSize(int w, int h);
void UpdatePolygon();
bool IsInsidePolygon(int x, int y);

// --- Helper Functions ---

void ScreenToGrid(int mouseX, int mouseY, int &gridX, int &gridY) {
	float range = app.dimension + 0.5f;
	float worldX = ((float)mouseX / app.winW) * (2.0f * range) - range;
	float worldY = (1.0f - (float)mouseY / app.winH) * (2.0f * range) - range;
	gridX = (int)floor(worldX + 0.5f);
	gridY = (int)floor(worldY + 0.5f);
}

void DrawText(float x, float y, const std::string& text) {
	glRasterPos2f(x, y);
	for (char c : text) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}
}

// --- Lab 07/08 Midpoint Line Functions ---

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

void AddEdgePixel(int x, int y, const Color& c) {
	app.edgePixels.push_back({x, y, c});
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
	AddEdgePixel(x, y, cAxis); // Start point

	if (region == 1) {
		int d = 2 * dy - dx, dE = 2 * dy, dNE = 2 * (dy - dx);
		while (x < x1) {
			if (d <= 0) { x++; d += dE; AddEdgePixel(x, y, cAxis); }
			else { x++; y++; d += dNE; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 2) {
		int d = 2 * dx - dy, dN = 2 * dx, dNE = 2 * (dx - dy);
		while (y < y1) {
			if (d <= 0) { y++; d += dN; AddEdgePixel(x, y, cAxis); }
			else { x++; y++; d += dNE; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 3) {
		int d = 2 * (-dx) - dy, dN = 2 * (-dx), dNW = 2 * (-dx - dy);
		while (y < y1) {
			if (d <= 0) { y++; d += dN; AddEdgePixel(x, y, cAxis); }
			else { x--; y++; d += dNW; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 4) {
		int d = 2 * dy - (-dx), dW = 2 * dy, dNW = 2 * (dy - (-dx));
		while (x > x1) {
			if (d <= 0) { x--; d += dW; AddEdgePixel(x, y, cAxis); }
			else { x--; y++; d += dNW; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 5) {
		int d = 2 * (-dy) - (-dx), dW = 2 * (-dy), dSW = 2 * ((-dy) - (-dx));
		while (x > x1) {
			if (d <= 0) { x--; d += dW; AddEdgePixel(x, y, cAxis); }
			else { x--; y--; d += dSW; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 6) {
		int d = 2 * (-dx) - (-dy), dS = 2 * (-dx), dSW = 2 * ((-dx) - (-dy));
		while (y > y1) {
			if (d <= 0) { y--; d += dS; AddEdgePixel(x, y, cAxis); }
			else { x--; y--; d += dSW; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 7) {
		int d = 2 * dx - (-dy), dS = 2 * dx, dSE = 2 * (dx - (-dy));
		while (y > y1) {
			if (d <= 0) { y--; d += dS; AddEdgePixel(x, y, cAxis); }
			else { x++; y--; d += dSE; AddEdgePixel(x, y, cDiag); }
		}
	} else if (region == 8) {
		int d = 2 * (-dy) - dx, dE = 2 * (-dy), dSE = 2 * ((-dy) - dx);
		while (x < x1) {
			if (d <= 0) { x++; d += dE; AddEdgePixel(x, y, cAxis); }
			else { x++; y--; d += dSE; AddEdgePixel(x, y, cDiag); }
		}
	}
}

// --- Jordan Curve Theorem (Ray Casting) ---

bool IsInsidePolygon(int x, int y) {
	int n = app.vertices.size();
	if (n < 3) return false;
	
	bool inside = false;
	for (int i = 0, j = n - 1; i < n; j = i++) {
		int xi = app.vertices[i].x, yi = app.vertices[i].y;
		int xj = app.vertices[j].x, yj = app.vertices[j].y;

		// Standard ray-casting algorithm (includes lower endpoint, excludes upper endpoint)
		if (((yi > y) != (yj > y)) &&
			(x < (double)(xj - xi) * (y - yi) / (yj - yi) + xi)) {
			inside = !inside;
		}
	}
	return inside;
}

void IOPolygonCheck(int x, int y) {
	int n = app.vertices.size();
	if (n < 3) {
		printf("Please select at least 3 vertices first.\n");
		return;
	}

	app.checkX = x; app.checkY = y; app.showCheck = true;
	printf("\n---Jordan Curve Algo for Point (%d, %d) ---\n", x, y);
	printf("Ray: y = %d, x >= %d\n", y, x);

	int totalIntersectionCount = 0;
	std::vector<bool> handled(n, false);

	for (int i = 0; i < n; ++i) {
		if (handled[i]) continue;

		int curr = i;
		int prev = (curr - 1 + n) % n;
		int next = (curr + 1) % n;

		// Check if vertex is on the ray's Y level
		if (app.vertices[curr].y == y) {
			// Is it part of a horizontal segment?
			if (app.vertices[next].y == y || app.vertices[prev].y == y) {
				// Find the full horizontal chain
				int s = curr;
				while (app.vertices[(s - 1 + n) % n].y == y) {
					s = (s - 1 + n) % n;
					if (s == curr) break; 
				}
				int e = curr;
				while (app.vertices[(e + 1) % n].y == y) {
					e = (e + 1) % n;
					if (e == s) break;
				}

				// Mark all as handled
				int temp = s;
				while (true) {
					handled[temp] = true;
					if (temp == e) break;
					temp = (temp + 1) % n;
				}

				// Check if any part of the chain is on the ray (x >= checkX)
				bool onRay = false;
				temp = s;
				while (true) {
					if (app.vertices[temp].x >= x) onRay = true;
					if (temp == e) break;
					temp = (temp + 1) % n;
				}

				if (onRay) {
					int vBefore = (s - 1 + n) % n;
					int vAfter = (e + 1) % n;
					int yBefore = app.vertices[vBefore].y;
					int yAfter = app.vertices[vAfter].y;

					int count = 0;
					std::string type;
					if (yBefore < y && yAfter < y) { type = "Case 4"; count = 0; }
					else if (yBefore > y && yAfter > y) { type = "Case 5"; count = 2; }
					else { type = "Case 6"; count = 1; }

					printf("Found %s from Vertex %d to %d. Intersection Count: %d\n", type.c_str(), s, e, count);
					totalIntersectionCount += count;
				}
			} else {
				// Single vertex on ray's Y level
				handled[curr] = true;
				if (app.vertices[curr].x >= x) {
					int yPrev = app.vertices[prev].y;
					int yNext = app.vertices[next].y;

					int count = 0;
					std::string type;
					if (yPrev < y && yNext < y) { type = "Case 1"; count = 0; }
					else if (yPrev > y && yNext > y) { type = "Case 2"; count = 2; }
					else { type = "Case 3"; count = 1; }

					printf("Found %s at Vertex %d. Intersection Count: %d\n", type.c_str(), curr, count);
					totalIntersectionCount += count;
				}
			}
			continue;
		}

		// Check for standard edge intersection (not at endpoints on the ray)
		int x1 = app.vertices[curr].x, y1 = app.vertices[curr].y;
		int x2 = app.vertices[next].x, y2 = app.vertices[next].y;

		if (y1 != y && y2 != y && ((y1 > y) != (y2 > y))) {
			double intersectX = (double)(x2 - x1) * (y - y1) / (y2 - y1) + x1;
			if (intersectX >= x) {
				printf("Found Edge Intersection (Edges %d-%d). Intersection Count: 1\n", curr, next);
				totalIntersectionCount += 1;
			}
		}
	}

	printf("Total Intersections Counted: %d\n", totalIntersectionCount);
	printf("Parity: %s\n", (totalIntersectionCount % 2 == 1) ? "ODD (INSIDE)" : "EVEN (OUTSIDE)");
	
	Color resultColor = (totalIntersectionCount % 2 == 1) ? Color{0.0f, 1.0f, 1.0f, "Inside"} : Color{1.0f, 1.0f, 0.0f, "Outside"};
	app.fillPixels.push_back({x, y, resultColor});
}

void UpdatePolygon() {
	app.fillPixels.clear();
	app.edgePixels.clear();
	if (app.vertices.size() < 3) return;

	// Calculate bounding box
	int minX = app.vertices[0].x, maxX = app.vertices[0].x;
	int minY = app.vertices[0].y, maxY = app.vertices[0].y;
	for (const auto& v : app.vertices) {
		minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
		minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
	}

	// Clamp to grid
	minX = std::max(minX, -app.dimension); maxX = std::min(maxX, app.dimension);
	minY = std::max(minY, -app.dimension); maxY = std::min(maxY, app.dimension);

	Color fillColor = {0.7f, 0.7f, 0.7f, "Interior (Light Grey)"}; // Lab 08 Interior Color

	if (app.isFilled) {
		for (int y = minY; y <= maxY; ++y) {
			for (int x = minX; x <= maxX; ++x) {
				if (IsInsidePolygon(x, y)) {
					app.fillPixels.push_back({x, y, fillColor});
				}
			}
		}
	}

	if (app.showMidpoint) {
		for (size_t i = 0; i < app.vertices.size(); ++i) {
			int next = (i + 1) % app.vertices.size();
			MidpointLine(app.vertices[i].x, app.vertices[i].y, app.vertices[next].x, app.vertices[next].y);
		}
	}
}

// --- Rendering Functions ---

void DrawGrid() {
	glLineWidth(1.0f);
	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_LINES);
	for (int i = -app.dimension; i <= app.dimension + 1; ++i) {
		float pos = (float)i - 0.5f;
		glVertex2f(pos, -app.dimension - 0.5f); glVertex2f(pos, app.dimension + 0.5f);
		glVertex2f(-app.dimension - 0.5f, pos); glVertex2f(app.dimension + 0.5f, pos);
	}
	glEnd();

	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(0.6f, 0.2f, 0.2f); glVertex2f(-app.dimension - 0.5f, 0.0f); glVertex2f(app.dimension + 0.5f, 0.0f);
	glColor3f(0.2f, 0.6f, 0.2f); glVertex2f(0.0f, -app.dimension - 0.5f); glVertex2f(0.0f, app.dimension + 0.5f);
	glEnd();
}

void DrawScene() {
	// Draw Filled Pixels
	glBegin(GL_QUADS);
	for (const auto& p : app.fillPixels) {
		glColor3f(p.color.r, p.color.g, p.color.b);
		glVertex2f(p.x - 0.5f, p.y - 0.5f); glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f); glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	// Draw Edge Pixels
	for (const auto& p : app.edgePixels) {
		glColor3f(p.color.r, p.color.g, p.color.b);
		glVertex2f(p.x - 0.5f, p.y - 0.5f); glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f); glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	// Draw Vertices
	for (const auto& p : app.vertices) {
		glColor3f(1.0f, 0.0f, 0.0f); // Lab 08 Red Vertices
		glVertex2f(p.x - 0.5f, p.y - 0.5f); glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f); glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	glEnd();
}
void DrawOverlay(){
	// Draw Outline
	if (app.vertices.size() >= 2) {
		glLineWidth(2.0f); glColor3f(1.0f, 1.0f, 1.0f); // Lab 08 White Outline
		glBegin(GL_LINE_LOOP);
		for (const auto& p : app.vertices) glVertex2f((float)p.x, (float)p.y);
		glEnd();
	}

	// Draw Check Ray
	if (app.showCheck) {
		glLineWidth(3.0f); glColor3f(1.0f, 0.5f, 0.0f);
		glBegin(GL_LINES);
		glVertex2f((float)app.checkX, (float)app.checkY);
		glVertex2f((float)app.dimension + 0.5f, (float)app.checkY);
		glEnd();
		
		// Draw checked point highlight
		glPointSize(10.0f);
		glBegin(GL_POINTS);
		glVertex2f((float)app.checkX, (float)app.checkY);
		glEnd();
	}

	// Draw Vertex Labels (Requested modification)
	glColor3f(1.0f, 1.0f, 1.0f);
	for (size_t i = 0; i < app.vertices.size(); ++i) {
		DrawText(app.vertices[i].x + 0.4f, app.vertices[i].y + 0.4f, std::to_string(i));
	}
}

// --- GLUT Callbacks ---

void RenderScene() {
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Lab 08 Background
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	DrawScene();
	DrawGrid();
	DrawOverlay();
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
			for (size_t i = 0; i < app.vertices.size(); ++i) {
				if (app.vertices[i].x == gx && app.vertices[i].y == gy) {
					app.vertices.erase(app.vertices.begin() + i);
					found = true;
					break;
				}
			}
			if (!found) {
				app.vertices.push_back({gx, gy, {1.0f, 0.0f, 0.0f, "Vertex"}});
			}
			app.showCheck = false;
			UpdatePolygon();
			glutPostRedisplay();
		}
	}
}

void Keyboard(unsigned char key, int x, int y) {
	if (key == 'r' || key == 'R') {
		app.vertices.clear(); app.fillPixels.clear(); app.edgePixels.clear();
		app.showCheck = false;
		printf("\n--- Canvas Reset ---\n");
	} else if (key == 'f' || key == 'F') {
		app.isFilled = !app.isFilled;
		UpdatePolygon();
	} else if (key == 'm' || key == 'M') {
		app.showMidpoint = !app.showMidpoint;
		printf("Midpoint Edge Display: %s\n", app.showMidpoint ? "ENABLED" : "DISABLED");
		UpdatePolygon();
	} else if (key == 'g') {
		int gx, gy; ScreenToGrid(x, y, gx, gy);
		IOPolygonCheck(gx, gy);
	}
	glutPostRedisplay();
}

void ProcessMenu(int value) {
	if (value == 0) {
		app.vertices.clear(); app.fillPixels.clear(); app.edgePixels.clear();
		app.showCheck = false;
		printf("\n--- Points Cleared ---\n");
	} else if (value == 1) {
		app.isFilled = !app.isFilled;
		UpdatePolygon();
	} else if (value == 2) {
		app.showMidpoint = !app.showMidpoint;
		printf("Midpoint Edge Display: %s\n", app.showMidpoint ? "ENABLED" : "DISABLED");
		UpdatePolygon();
	} else {
		app.dimension = value;
		app.vertices.clear(); app.fillPixels.clear(); app.edgePixels.clear();
		app.showCheck = false;
		ChangeSize(app.winW, app.winH);
		printf("\n--- Grid Dimension: %d x %d ---\n", value, value);
	}
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(app.winW, app.winH);
	glutCreateWindow("Lab 09: Jordan Curve Theorem (Polygon Filling)");

	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Clear All Points", 0);
	glutAddMenuEntry("Toggle Filling", 1);
	glutAddMenuEntry("Toggle Midpoint Edges", 2);
	glutAddMenuEntry("Dimension 10 x 10", 10);
	glutAddMenuEntry("Dimension 15 x 15", 15);
	glutAddMenuEntry("Dimension 20 x 20", 20);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	printf("Lab 09: Jordan Curve Theorem Loaded (with Lab 08 Features).\n");
	printf("Controls: Left-click: Add/Remove vertices, 'f': Toggle Fill, 'm': Toggle Midpoint Edges, 'g': IO Check, 'r': Reset.\n");
	glutMainLoop();
	return 0;
}