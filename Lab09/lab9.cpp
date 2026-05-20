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

struct Color
{
	float r, g, b;
	std::string name;
};

struct Pixel
{
	int x, y;
	Color color;
};

struct GridApp
{
	int dimension = 15;
	int winW = 800, winH = 800;
	bool showEdges = true;
	bool isFilled = true;
	std::vector<Pixel> vertices;
	std::vector<Pixel> fillPixels;
	std::vector<Pixel> edgePixels;
} app;

// --- Forward Declarations ---
void ChangeSize(int w, int h);
void UpdatePolygon();

// --- Helper Functions ---

void ScreenToGrid(int mouseX, int mouseY, int &gridX, int &gridY)
{
	float range = app.dimension + 0.5f;
	float worldX = ((float)mouseX / app.winW) * (2.0f * range) - range;
	float worldY = (1.0f - (float)mouseY / app.winH) * (2.0f * range) - range;
	gridX = (int)floor(worldX + 0.5f);
	gridY = (int)floor(worldY + 0.5f);
}

void DrawText(float x, float y, const std::string &text)
{
	glRasterPos2f(x, y);
	for (char c : text)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}
}

// --- Lab 07 Midpoint Line Functions (Modified for Polygon Edges) ---

void AddEdgePixel(int x, int y)
{
	// app.edgePixels.push_back({x, y, {1.0f, 1.0f, 1.0f, "Edge"}});
	app.edgePixels.push_back({x, y, {0.2f, 0.4f, 0.6f, "Edge"}});
}

void MidpointLine(int x0, int y0, int x1, int y1)
{
	int dx = abs(x1 - x0), dy = abs(y1 - y0);
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while (true)
	{
		AddEdgePixel(x0, y0);
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

// --- Jordan Curve Theorem (Ray Casting) ---

bool IsInsidePolygon(int x, int y)
{
	int n = app.vertices.size();
	if (n < 3)
		return false;

	bool inside = false;
	for (int i = 0, j = n - 1; i < n; j = i++)
	{
		int xi = app.vertices[i].x, yi = app.vertices[i].y;
		int xj = app.vertices[j].x, yj = app.vertices[j].y;

		// Standard ray-casting algorithm
		// Ray cast in +X direction
		if (((yi > y) != (yj > y)) &&
			(x < (double)(xj - xi) * (y - yi) / (yj - yi) + xi))
		{
			inside = !inside;
		}
	}
	return inside;
}

void UpdatePolygon()
{
	app.fillPixels.clear();
	app.edgePixels.clear();
	if (app.vertices.size() < 3)
		return;

	// Calculate bounding box
	int minX = app.vertices[0].x, maxX = app.vertices[0].x;
	int minY = app.vertices[0].y, maxY = app.vertices[0].y;
	for (const auto &v : app.vertices)
	{
		minX = std::min(minX, v.x);
		maxX = std::max(maxX, v.x);
		minY = std::min(minY, v.y);
		maxY = std::max(maxY, v.y);
	}

	// Clamp to grid
	minX = std::max(minX, -app.dimension);
	maxX = std::min(maxX, app.dimension);
	minY = std::max(minY, -app.dimension);
	maxY = std::min(maxY, app.dimension);

	Color fillColor = {0.2f, 0.4f, 0.6f, "Fill"};

	if (app.isFilled)
	{
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				if (IsInsidePolygon(x, y))
				{
					app.fillPixels.push_back({x, y, fillColor});
				}
			}
		}
	}

	if (app.showEdges)
	{
		for (size_t i = 0; i < app.vertices.size(); ++i)
		{
			int next = (i + 1) % app.vertices.size();
			MidpointLine(app.vertices[i].x, app.vertices[i].y, app.vertices[next].x, app.vertices[next].y);
		}
	}
}

// --- Rendering Functions ---

void DrawGrid()
{
	glLineWidth(1.0f);
	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_LINES);
	for (int i = -app.dimension; i <= app.dimension + 1; ++i)
	{
		float pos = (float)i - 0.5f;
		glVertex2f(pos, -app.dimension - 0.5f);
		glVertex2f(pos, app.dimension + 0.5f);
		glVertex2f(-app.dimension - 0.5f, pos);
		glVertex2f(app.dimension + 0.5f, pos);
	}
	glEnd();

	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glColor3f(0.6f, 0.2f, 0.2f);
	glVertex2f(-app.dimension - 0.5f, 0.0f);
	glVertex2f(app.dimension + 0.5f, 0.0f);
	glColor3f(0.2f, 0.6f, 0.2f);
	glVertex2f(0.0f, -app.dimension - 0.5f);
	glVertex2f(0.0f, app.dimension + 0.5f);
	glEnd();
}

void DrawScene()
{
	// Draw Filled Pixels
	glBegin(GL_QUADS);
	for (const auto &p : app.fillPixels)
	{
		glColor3f(p.color.r, p.color.g, p.color.b);
		glVertex2f(p.x - 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f);
		glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	// Draw Edge Pixels
	for (const auto &p : app.edgePixels)
	{
		glColor3f(p.color.r, p.color.g, p.color.b);
		glVertex2f(p.x - 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f);
		glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	// Draw Vertices
	for (const auto &p : app.vertices)
	{
		glColor3f(1.0f, 0.5f, 0.0f);
		glVertex2f(p.x - 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f);
		glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	glEnd();



	
}

void DrawSceneOverlay(){
	// Draw Outline
	if (app.vertices.size() >= 2)
	{
		glLineWidth(1.0f);
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_LINE_LOOP);
		for (const auto &p : app.vertices)
			glVertex2f((float)p.x, (float)p.y);
		glEnd();
	}
	// Draw Vertex Labels
	glColor3f(1.0f, 1.0f, 1.0f);
	for (size_t i = 0; i < app.vertices.size(); ++i)
	{
		DrawText(app.vertices[i].x + 0.4f, app.vertices[i].y + 0.4f, std::to_string(i));
	}
}

// --- GLUT Callbacks ---

void RenderScene() {
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	DrawScene();
	DrawGrid();
	DrawSceneOverlay();
	glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
	app.winW = w;
	app.winH = (h == 0) ? 1 : h;
	glViewport(0, 0, app.winW, app.winH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float r = app.dimension + 0.5f;
	glOrtho(-r, r, -r, r, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		int gx, gy;
		ScreenToGrid(x, y, gx, gy);

		bool removed = false;
		for (size_t i = 0; i < app.vertices.size(); ++i)
		{
			if (app.vertices[i].x == gx && app.vertices[i].y == gy)
			{
				app.vertices.erase(app.vertices.begin() + i);
				removed = true;
				break;
			}
		}

		if (!removed && abs(gx) <= app.dimension && abs(gy) <= app.dimension)
		{
			app.vertices.push_back({gx, gy, {1.0f, 0.0f, 0.0f, "Vertex"}});
		}

		UpdatePolygon();
		glutPostRedisplay();
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == 'r' || key == 'R')
	{
		app.vertices.clear();
		app.fillPixels.clear();
		app.edgePixels.clear();
		printf("Resetting polygon.\n");
	}
	else if (key == 'f' || key == 'F')
	{
		app.isFilled = !app.isFilled;
		UpdatePolygon();
	}
	else if (key == 'e' || key == 'E')
	{
		app.showEdges = !app.showEdges;
		UpdatePolygon();
	}
	glutPostRedisplay();
}

void ProcessMenu(int value)
{
	if (value == 0)
	{
		app.vertices.clear();
		app.fillPixels.clear();
		app.edgePixels.clear();
	}
	else if (value == 1)
	{
		app.isFilled = !app.isFilled;
		UpdatePolygon();
	}
	else if (value == 2)
	{
		app.showEdges = !app.showEdges;
		UpdatePolygon();
	}
	else
	{
		app.dimension = value;
		app.vertices.clear();
		app.fillPixels.clear();
		app.edgePixels.clear();
		ChangeSize(app.winW, app.winH);
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(app.winW, app.winH);
	glutCreateWindow("Lab 09: Jordan Curve Theorem (Polygon Filling)");

	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Clear All", 0);
	glutAddMenuEntry("Toggle Fill", 1);
	glutAddMenuEntry("Toggle Midpoint Edges", 2);
	glutAddMenuEntry("Grid 10x10", 10);
	glutAddMenuEntry("Grid 15x15", 15);
	glutAddMenuEntry("Grid 20x20", 20);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	printf("Lab 09: Jordan Curve Theorem Loaded.\n");
	printf("Controls: Left-click: Add/Remove vertices, 'f': Toggle Fill, 'e': Toggle Edges, 'r': Reset.\n");
	glutMainLoop();
	return 0;
}