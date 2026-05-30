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

enum AnimState
{
	IDLE,
	PLAYING,
	PAUSED
};

struct GridApp
{
	int dimension = 10;
	int winW = 800, winH = 800;

	std::vector<Pixel> vertices;
	std::vector<Pixel> edgePixels;
	std::vector<Pixel> finalPixels;

	// Animation State
	AnimState animState = IDLE;
	int animX = -10, animY = -10;
	int animSpeed = 50; // ms per step
} app;

const std::vector<Color> vertexColors = {
	{1.0f, 0.0f, 0.0f, "Red"},
	{1.0f, 0.5f, 0.0f, "Orange"},
	{1.0f, 1.0f, 0.0f, "Yellow"},
	{0.0f, 1.0f, 0.0f, "Green"},
	{0.0f, 0.0f, 1.0f, "Blue"},
	{0.5f, 0.0f, 0.5f, "Purple"},
	{0.0f, 1.0f, 1.0f, "Cyan"},
	{1.0f, 0.0f, 1.0f, "Magenta"}};

const Color edgeColor = {0.5f, 0.5f, 0.5f, "Edge (Grey)"};
const Color insideColor = {0.0f, 1.0f, 1.0f, "Inside (Cyan)"};
const Color outsideColor = {0.5f, 0.5f, 0.0f, "Outside (Yellow)"};

// --- Forward Declarations ---
void ChangeSize(int w, int h);
void UpdatePolygon();
void SetupAnimation();
void NextPixel();
void Timer(int value);

// --- Helper Functions ---

// Converts mouse screen coordinates to grid integer coordinates
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

// --- Lab 07/08 Midpoint Line Functions ---

void GetRegionColors(int region, Color &axis, Color &diag)
{
	if (region == 1 || region == 2)
	{
		axis = {0.0f, 1.0f, 0.0f, "Bright Green"};
		diag = {0.0f, 0.0f, 1.0f, "Dark Blue"};
	}
	else if (region == 3 || region == 4)
	{
		axis = {1.0f, 1.0f, 0.0f, "Bright Yellow"};
		diag = {0.5f, 0.5f, 0.0f, "Dark Yellow"};
	}
	else if (region == 5 || region == 6)
	{
		axis = {1.0f, 0.0f, 1.0f, "Bright Magenta"};
		diag = {0.5f, 0.0f, 0.5f, "Dark Magenta"};
	}
	else
	{
		axis = {0.0f, 1.0f, 1.0f, "Bright Cyan"};
		diag = {0.0f, 0.5f, 0.5f, "Dark Cyan"};
	}
}

void AddEdgePixel(int x, int y, const Color &c)
{
	app.edgePixels.push_back({x, y, c});
}

void MidpointLine(int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0, dy = y1 - y0;
	int region = 0;

	if (abs(dx) >= abs(dy))
	{
		if (dx >= 0 && dy >= 0)
			region = 1;
		else if (dx < 0 && dy >= 0)
			region = 4;
		else if (dx < 0 && dy < 0)
			region = 5;
		else if (dx >= 0 && dy < 0)
			region = 8;
	}
	else
	{
		if (dx >= 0 && dy >= 0)
			region = 2;
		else if (dx < 0 && dy >= 0)
			region = 3;
		else if (dx < 0 && dy < 0)
			region = 6;
		else if (dx >= 0 && dy < 0)
			region = 7;
	}

	Color cAxis, cDiag;
	GetRegionColors(region, cAxis, cDiag);

	int x = x0, y = y0;
	AddEdgePixel(x, y, cAxis); // Start point

	if (region == 1)
	{
		int d = 2 * dy - dx, dE = 2 * dy, dNE = 2 * (dy - dx);
		while (x < x1)
		{
			if (d <= 0)
			{
				x++;
				d += dE;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x++;
				y++;
				d += dNE;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 2)
	{
		int d = 2 * dx - dy, dN = 2 * dx, dNE = 2 * (dx - dy);
		while (y < y1)
		{
			if (d <= 0)
			{
				y++;
				d += dN;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x++;
				y++;
				d += dNE;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 3)
	{
		int d = 2 * (-dx) - dy, dN = 2 * (-dx), dNW = 2 * (-dx - dy);
		while (y < y1)
		{
			if (d <= 0)
			{
				y++;
				d += dN;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x--;
				y++;
				d += dNW;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 4)
	{
		int d = 2 * dy - (-dx), dW = 2 * dy, dNW = 2 * (dy - (-dx));
		while (x > x1)
		{
			if (d <= 0)
			{
				x--;
				d += dW;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x--;
				y++;
				d += dNW;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 5)
	{
		int d = 2 * (-dy) - (-dx), dW = 2 * (-dy), dSW = 2 * ((-dy) - (-dx));
		while (x > x1)
		{
			if (d <= 0)
			{
				x--;
				d += dW;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x--;
				y--;
				d += dSW;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 6)
	{
		int d = 2 * (-dx) - (-dy), dS = 2 * (-dx), dSW = 2 * ((-dx) - (-dy));
		while (y > y1)
		{
			if (d <= 0)
			{
				y--;
				d += dS;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x--;
				y--;
				d += dSW;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 7)
	{
		int d = 2 * dx - (-dy), dS = 2 * dx, dSE = 2 * (dx - (-dy));
		while (y > y1)
		{
			if (d <= 0)
			{
				y--;
				d += dS;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x++;
				y--;
				d += dSE;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
	else if (region == 8)
	{
		int d = 2 * (-dy) - dx, dE = 2 * (-dy), dSE = 2 * ((-dy) - dx);
		while (x < x1)
		{
			if (d <= 0)
			{
				x++;
				d += dE;
				AddEdgePixel(x, y, cAxis);
			}
			else
			{
				x++;
				y--;
				d += dSE;
				AddEdgePixel(x, y, cDiag);
			}
		}
	}
}

// --- Jordan Curve Theorem & Animation ---

int GetIntersectionCount(int x, int y)
{
	int n = app.vertices.size();
	if (n < 3)
		return 0;

	int totalIntersectionCount = 0;
	std::vector<bool> handled(n, false);

	for (int i = 0; i < n; ++i)
	{
		if (handled[i])
			continue;
		int curr = i;
		int prev = (curr - 1 + n) % n;
		int next = (curr + 1) % n;

		if (app.vertices[curr].y == y)
		{
			if (app.vertices[next].y == y || app.vertices[prev].y == y)
			{
				int s = curr;
				while (app.vertices[(s - 1 + n) % n].y == y)
				{
					s = (s - 1 + n) % n;
					if (s == curr)
						break;
				}
				int e = curr;
				while (app.vertices[(e + 1) % n].y == y)
				{
					e = (e + 1) % n;
					if (e == s)
						break;
				}
				int temp = s;
				while (true)
				{
					handled[temp] = true;
					if (temp == e)
						break;
					temp = (temp + 1) % n;
				}
				bool onRay = false;
				temp = s;
				while (true)
				{
					if (app.vertices[temp].x >= x)
						onRay = true;
					if (temp == e)
						break;
					temp = (temp + 1) % n;
				}
				if (onRay)
				{
					int yBefore = app.vertices[(s - 1 + n) % n].y;
					int yAfter = app.vertices[(e + 1) % n].y;
					if (yBefore < y && yAfter < y)
						totalIntersectionCount += 0;
					else if (yBefore > y && yAfter > y)
						totalIntersectionCount += 2;
					else
						totalIntersectionCount += 1;
				}
			}
			else
			{
				handled[curr] = true;
				if (app.vertices[curr].x >= x)
				{
					int yPrev = app.vertices[prev].y;
					int yNext = app.vertices[next].y;
					if (yPrev < y && yNext < y)
						totalIntersectionCount += 0;
					else if (yPrev > y && yNext > y)
						totalIntersectionCount += 2;
					else
						totalIntersectionCount += 1;
				}
			}
			continue;
		}
		int x1 = app.vertices[curr].x, y1 = app.vertices[curr].y;
		int x2 = app.vertices[next].x, y2 = app.vertices[next].y;
		if (y1 != y && y2 != y && ((y1 > y) != (y2 > y)))
		{
			double intersectX = (double)(x2 - x1) * (y - y1) / (y2 - y1) + x1;
			if (intersectX >= (double)x)
				totalIntersectionCount += 1;
		}
	}
	return totalIntersectionCount;
}

void SetupAnimation()
{
	app.finalPixels.clear();
	app.edgePixels.clear();
	if (app.vertices.size() < 3)
	{
		app.animState = IDLE;
		return;
	}
	// Pre-calculate edge pixels (Grey)
	for (size_t i = 0; i < app.vertices.size(); ++i)
	{
		int next = (i + 1) % app.vertices.size();
		MidpointLine(app.vertices[i].x, app.vertices[i].y, app.vertices[next].x, app.vertices[next].y);
	}
	// Reset animation position
	app.animX = -app.dimension;
	app.animY = -app.dimension;
}
void PreviousPixel()
{
	if (app.finalPixels.empty())
		return;
	app.finalPixels.pop_back();
	printf("Rewound to previous pixel. Remaining pixels: %lu\n", app.finalPixels.size());
	app.animX--;
	if (app.animX < -app.dimension)
	{
		app.animX = app.dimension;
		app.animY--;
	}
}

void NextPixel()
{
	if (app.animY > app.dimension)
	{
		app.animState = IDLE;
		return;
	}

	int x = app.animX, y = app.animY;
	Color c;
	bool found = false;

	// 1. Priority: Vertices
	for (size_t i = 0; i < app.vertices.size(); ++i)
	{
		if (app.vertices[i].x == x && app.vertices[i].y == y)
		{
			c = vertexColors[i % vertexColors.size()];
			found = true;
			break;
		}
	}

	// 2. Priority: Edges (Grey)
	if (!found)
	{
		for (const auto &ep : app.edgePixels)
		{
			if (ep.x == x && ep.y == y)
			{
				c = edgeColor;
				found = true;
				break;
			}
		}
	}

	// 3. Jordan Curve Theorem
	if (!found)
	{
		int crossNum = GetIntersectionCount(x, y);
		printf("Pixel (%d, %d): cross number = %d\n", x, y, crossNum);
		if (crossNum % 2 == 1)
			c = insideColor;
		else
			c = outsideColor;
	}

	app.finalPixels.push_back({x, y, c});

	// Advance
	app.animX++;
	if (app.animX > app.dimension)
	{
		app.animX = -app.dimension;
		app.animY++;
	}
}

void Timer(int value)
{
	if (app.animState == PLAYING)
	{
		NextPixel();
	}
	glutPostRedisplay();
	glutTimerFunc(app.animSpeed, Timer, 0);
}

void UpdatePolygon()
{
	SetupAnimation();
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
	glBegin(GL_QUADS);
	// Draw evaluated pixels
	for (const auto &p : app.finalPixels)
	{
		glColor3f(p.color.r, p.color.g, p.color.b);
		glVertex2f(p.x - 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y - 0.5f);
		glVertex2f(p.x + 0.5f, p.y + 0.5f);
		glVertex2f(p.x - 0.5f, p.y + 0.5f);
	}
	glEnd();

	// Draw Vertices on top
	glBegin(GL_QUADS);
	for (size_t i = 0; i < app.vertices.size(); ++i)
	{
		Color c = vertexColors[i % vertexColors.size()];
		glColor3f(c.r, c.g, c.b);
		glVertex2f(app.vertices[i].x - 0.5f, app.vertices[i].y - 0.5f);
		glVertex2f(app.vertices[i].x + 0.5f, app.vertices[i].y - 0.5f);
		glVertex2f(app.vertices[i].x + 0.5f, app.vertices[i].y + 0.5f);
		glVertex2f(app.vertices[i].x - 0.5f, app.vertices[i].y + 0.5f);
	}
	glEnd();
}

void DrawOverlay()
{
	// Draw Outline
	if (app.vertices.size() >= 2)
	{
		glLineWidth(2.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINE_LOOP);
		for (const auto &p : app.vertices)
			glVertex2f((float)p.x, (float)p.y);
		glEnd();
	}

	// Draw Animation Highlight & Ray
	if (app.animState != IDLE && app.animY <= app.dimension)
	{
		// Current Pixel
		glColor3f(1.0f, 1.0f, 1.0f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(app.animX - 0.55f, app.animY - 0.55f);
		glVertex2f(app.animX + 0.55f, app.animY - 0.55f);
		glVertex2f(app.animX + 0.55f, app.animY + 0.55f);
		glVertex2f(app.animX - 0.55f, app.animY + 0.55f);
		glEnd();

		// Ray
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xAAAA);
		glBegin(GL_LINES);
		glVertex2f((float)app.animX, (float)app.animY);
		glVertex2f((float)app.dimension + 0.5f, (float)app.animY);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	// Draw Vertex Labels
	glColor3f(1.0f, 1.0f, 1.0f);
	for (size_t i = 0; i < app.vertices.size(); ++i)
	{
		DrawText(app.vertices[i].x + 0.4f, app.vertices[i].y + 0.4f, std::to_string(i));
	}

	// Draw bottom-left text overlay
	std::string status = (app.animState == PLAYING) ? "PLAYING" : (app.animState == PAUSED ? "PAUSED" : "IDLE");
	std::string info = "State: " + status + " | Speed: " + std::to_string(app.animSpeed) + "ms | Vertices: " + std::to_string(app.vertices.size());
	std::string hints = "Keys: [P] Play/Pause [N] Next [M] Previous [R] Replay [C] Clear [+/-] Speed";

	float offset = app.dimension + 0.3f;
	DrawText(-offset, -offset, info);
	DrawText(-offset, -offset + 0.8f, hints);
}

// --- GLUT Callbacks ---

void RenderScene()
{
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Lab 08 Background
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	DrawScene();
	DrawGrid();
	DrawOverlay();
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
		if (abs(gx) <= app.dimension && abs(gy) <= app.dimension)
		{
			bool found = false;
			for (size_t i = 0; i < app.vertices.size(); ++i)
			{
				if (app.vertices[i].x == gx && app.vertices[i].y == gy)
				{
					app.vertices.erase(app.vertices.begin() + i);
					found = true;
					break;
				}
			}
			if (!found)
			{
				app.vertices.push_back({gx, gy, vertexColors[app.vertices.size() % vertexColors.size()]});
			}
			app.animState = IDLE;
			SetupAnimation();
			glutPostRedisplay();
		}
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	if (key == 'p' || key == 'P')
	{
		if (app.animState == PLAYING)
			app.animState = PAUSED;
		else
		{
			if (app.animY > app.dimension)
				SetupAnimation(); // Restart if finished
			app.animState = PLAYING;
		}
		printf("Animation: %s\n", (app.animState == PLAYING) ? "PLAYING" : "PAUSED");
	}
	else if (key == 'n' || key == 'N')
	{
		if (app.animState != PLAYING)
		{
			if (app.animY > app.dimension)
				SetupAnimation();
			NextPixel();
		}
	}
	else if (key == 'm' || key == 'M')
	{ // Rewind to previous step
		if (app.animState != PLAYING)
		{
			if (app.animState == IDLE && !app.finalPixels.empty())
				app.animState = PAUSED;
			PreviousPixel();
		}
	}
	else if (key == 'r' || key == 'R')
	{
		SetupAnimation();
		app.animState = PLAYING;
		printf("Animation: REPLAYING\n");
	}
	else if (key == 'c' || key == 'C')
	{
		app.vertices.clear();
		app.finalPixels.clear();
		app.edgePixels.clear();
		app.animState = IDLE;
		printf("Canvas: CLEARED\n");
	}
	else if (key == '+' || key == '=')
	{
		app.animSpeed = std::max(1, app.animSpeed - 10);
		printf("Animation Speed: %d ms delay\n", app.animSpeed);
	}
	else if (key == '-' || key == '_')
	{
		app.animSpeed += 10;
		printf("Animation Speed: %d ms delay\n", app.animSpeed);
	}
	glutPostRedisplay();
}

void ProcessMenu(int value)
{
	if (value == 0)
	{
		app.vertices.clear();
		app.finalPixels.clear();
		app.edgePixels.clear();
		app.animState = IDLE;
		printf("Canvas: CLEARED\n");
	}
	else
	{
		app.dimension = value;
		app.vertices.clear();
		app.finalPixels.clear();
		app.edgePixels.clear();
		app.animState = IDLE;
		app.animX = -app.dimension;
		app.animY = -app.dimension;
		ChangeSize(app.winW, app.winH);
		printf("Grid Dimension: %d x %d\n", value, value);
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(app.winW, app.winH);
	glutCreateWindow("Lab 10: Jordan Curve & Animation");

	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(app.animSpeed, Timer, 0);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Clear All", 0);
	glutAddMenuEntry("Dimension 10 x 10", 10);
	glutAddMenuEntry("Dimension 15 x 15", 15);
	glutAddMenuEntry("Dimension 20 x 20", 20);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	printf("Lab 10: Jordan Curve & Animation\n");
	printf("Controls: \n");
	printf("  Left-click: Add/Remove vertices\n");
	printf("  'p': Play/Pause\n");
	printf("  'n': Next Step\n");
	printf("  'r': Replay\n");
	printf("  'c': Clear\n");
	printf("  '+/-': Speed Up/Down\n");

	glutMainLoop();
	return 0;
}