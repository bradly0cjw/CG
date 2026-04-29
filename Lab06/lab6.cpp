#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>

#include ".\GL\freeglut.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int gridDimension = 10;
int winW = 1000, winH = 1000;

struct Cell
{
	int x, y;
	bool operator==(const Cell &other) const
	{
		return x == other.x && y == other.y;
	}
};

std::vector<Cell> filledCells;

void ScreenToGrid(int mouseX, int mouseY, int &gridX, int &gridY)
{
	float range = gridDimension + 0.5f;

	float worldX = ((float)mouseX / winW) * (2.0f * range) - range;

	float worldY = (1.0f - (float)mouseY / winH) * (2.0f * range) - range;

	gridX = (int)floor(worldX + 0.5f);
	gridY = (int)floor(worldY + 0.5f);
}

void DrawGrid()
{
	glLineWidth(1.0f);
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_LINES);
	for (int i = -gridDimension; i <= gridDimension + 1; ++i)
	{
		float pos = (float)i - 0.5f;
		glVertex2f(pos, -gridDimension - 0.5f);
		glVertex2f(pos, gridDimension + 0.5f);
		glVertex2f(-gridDimension - 0.5f, pos);
		glVertex2f(gridDimension + 0.5f, pos);
	}
	glEnd();

	glLineWidth(2.0f);
	glBegin(GL_LINES);

	glColor3f(1.0f, 0.3f, 0.3f);
	glVertex2f(-gridDimension - 0.5f, 0.0f);
	glVertex2f(gridDimension + 0.5f, 0.0f);

	glColor3f(0.3f, 1.0f, 0.3f);
	glVertex2f(0.0f, -gridDimension - 0.5f);
	glVertex2f(0.0f, gridDimension + 0.5f);
	glEnd();
}

void DrawFilledCells()
{
	glColor3f(0.0f, 0.7f, 0.9f); // Cyan
	glBegin(GL_QUADS);
	for (size_t i = 0; i < filledCells.size(); ++i)
	{
		float x = (float)filledCells[i].x;
		float y = (float)filledCells[i].y;

		glVertex2f(x - 0.5f, y - 0.5f);
		glVertex2f(x + 0.5f, y - 0.5f);
		glVertex2f(x + 0.5f, y + 0.5f);
		glVertex2f(x - 0.5f, y + 0.5f);
	}
	glEnd();
}

void RenderScene(void)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	DrawFilledCells();
	DrawGrid();

	glutSwapBuffers();
}

void ChangeSize(int w, int h)
{
	if (h == 0)
		h = 1;
	winW = w;
	winH = h;
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float range = gridDimension + 0.5f;

	glOrtho(-range, range, -range, range, -1.0, 1.0);
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		int gridX, gridY;

		ScreenToGrid(x, y, gridX, gridY);

		if (gridX >= -gridDimension && gridX <= gridDimension &&
			gridY >= -gridDimension && gridY <= gridDimension)
		{

			printf("Cell coordinate: (%d, %d)\n", gridX, gridY);

			Cell newCell = {gridX, gridY};
			bool exists = false;
			for (size_t i = 0; i < filledCells.size(); ++i)
			{
				if (filledCells[i] == newCell)
				{
					exists = true;
					// remove the cell if it already exists
					filledCells.erase(filledCells.begin() + i);
					break;
				}
			}
			if (!exists)
			{
				filledCells.push_back(newCell);
			}
			glutPostRedisplay();
		}
	}
}

void ProcessMenu(int value)
{
	gridDimension = value;
	filledCells.clear();
	ChangeSize(winW, winH);
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(winW, winH);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Lab06: 2D Clickable Grid");

	glutDisplayFunc(RenderScene);
	glutReshapeFunc(ChangeSize);
	glutMouseFunc(Mouse);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("Dimension 10 x 10", 10);
	glutAddMenuEntry("Dimension 15 x 15", 15);
	glutAddMenuEntry("Dimension 20 x 20", 20);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	printf("Lab 06: 2D Grid Initialized.\n");
	printf("Right-click for menu to change grid dimensions.\n");
	printf("Left-click on cells to fill them and see coordinates.\n");

	glutMainLoop();
	return 0;
}
