// SphereWorld.cpp
// OpenGL SuperBible
// Demonstrates an immersive 3D environment using actors
// and a camera. This version adds lights and material properties
// and shadows.
// Program by Richard S. Wright Jr.

#include "gltools.h" // OpenGL toolkit
#include "math3d.h"  // 3D Math Library
#include "glframe.h" // Frame class
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#ifdef _WIN32
// Compile-time configuration: set to 1 to enable Xbox controller support (requires linking -lxinput9_1_0 on Windows),
// or set to 0 to disable it and build a pure standard cross-platform GLUT project.
#ifndef ENABLE_CONTROLLER
#define ENABLE_CONTROLLER 0
#endif

#if ENABLE_CONTROLLER
#include <windows.h>
#include <xinput.h>
#endif
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_SPHERES 30

struct PedestrianInfo
{
    GLFrame frame;
    float walkPhase;
    float walkSpeed;
    float scale;
    float colorTorso[3];
    float colorHead[3];
    float colorLegs[3];
};

PedestrianInfo pedestrians[NUM_SPHERES];
GLFrame frameCamera;

int timer_flag = 1;             // 1 = running, 0 = paused
bool sunlightMode = false;      // false = night/moon mode, true = sun light mode
bool skyscraperGlow = true;     // true = windows/spires/sign glow, false = off
bool skyscraperOutline = false; // true = outline glowing enabled, false = off

// Keyboard States for multi-key simultaneous movement
bool keyState[256] = {false};
bool specialKeyState[256] = {false};

// Light and material Data
GLfloat fLightPos0[4] = {-100.0f, 100.0f, 50.0f, 1.0f}; // Point source (Sun)
GLfloat fLightPos1[4] = {0.0f, 4.0f, 0.0f, 1.0f};       // Point source (Central Neon Lamp)

// Dynamic Moon positions changed by keys 1-4
GLfloat moonPositions[4][4] = {
    {-100.0f, 100.0f, 50.0f, 1.0f},
    {100.0f, 120.0f, -50.0f, 1.0f},
    {0.0f, 150.0f, -100.0f, 1.0f},
    {-80.0f, 80.0f, -120.0f, 1.0f}};
int activeMoonPos = 0;

GLfloat fNoLight[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat fLowLight[] = {0.1f, 0.1f, 0.12f, 1.0f};
GLfloat fBrightLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat fMoonLight[] = {0.12f, 0.12f, 0.16f, 1.0f}; // Dim moon light for night environment
GLfloat fNoSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat fLowSpecular[] = {0.2f, 0.2f, 0.2f, 1.0f};

M3DMatrix44f mShadowMatrix0;
M3DMatrix44f mShadowMatrix1;

#define GRASS_TEXTURE 0
#define CONCRETE_TEXTURE 1
#define ASPHALT_TEXTURE 2
#define METAL_TEXTURE 3
#define NEON_TEXTURE 4
#define HOLO_TEXTURE 5
#define SPOTLIGHT_TEXTURE 6
#define NUM_TEXTURES 7
GLuint textureObjects[NUM_TEXTURES];

const char *szTextureFiles[] = {"materials/cyber_grass.tga", "materials/cyber_concrete.tga", "materials/cyber_asphalt.tga", "materials/cyber_metal.tga", "materials/neon_grid.tga", "materials/hologram.tga", "materials/cyber_spotlight.tga"};

// OBJ Model Loader structures
struct Vertex
{
    float x, y, z;
};

struct Face
{
    int vIndices[4];
    int numIndices;
};

struct OBJModel
{
    std::vector<Vertex> vertices;
    std::vector<Face> faces;
};

OBJModel modelDrone;
float droneTime = 0.0f;

// Parse the OBJ model file
bool LoadOBJModel(const char *filename, OBJModel &model)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    model.vertices.clear();
    model.faces.clear();
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        std::stringstream ss(line);
        std::string prefix;
        if (!(ss >> prefix))
            continue;

        if (prefix == "v")
        {
            Vertex v;
            if (ss >> v.x >> v.y >> v.z)
            {
                model.vertices.push_back(v);
            }
        }
        else if (prefix == "f")
        {
            Face f;
            f.numIndices = 0;
            std::string vertRef;
            while (ss >> vertRef && f.numIndices < 4)
            {
                size_t firstSlash = vertRef.find('/');
                int vIdx = std::stoi(vertRef.substr(0, firstSlash));
                if (vIdx < 0)
                    vIdx = (int)model.vertices.size() + vIdx + 1;
                f.vIndices[f.numIndices] = vIdx - 1;
                f.numIndices++;
            }
            if (f.numIndices > 0)
            {
                model.faces.push_back(f);
            }
        }
    }
    file.close();
    return true;
}

// Draw the loaded OBJ model with computed face normals for proper lighting
void DrawOBJModel(const OBJModel &model)
{
    for (const auto &face : model.faces)
    {
        if (face.numIndices == 3)
            glBegin(GL_TRIANGLES);
        else if (face.numIndices == 4)
            glBegin(GL_QUADS);
        else
            glBegin(GL_POLYGON);

        // Compute flat face normal for correct shading
        if (face.numIndices >= 3)
        {
            int i0 = face.vIndices[0];
            int i1 = face.vIndices[1];
            int i2 = face.vIndices[2];
            M3DVector3f v0 = {model.vertices[i0].x, model.vertices[i0].y, model.vertices[i0].z};
            M3DVector3f v1 = {model.vertices[i1].x, model.vertices[i1].y, model.vertices[i1].z};
            M3DVector3f v2 = {model.vertices[i2].x, model.vertices[i2].y, model.vertices[i2].z};
            M3DVector3f edge1, edge2, normal;
            m3dSubtractVectors3(edge1, v1, v0);
            m3dSubtractVectors3(edge2, v2, v0);
            m3dCrossProduct(normal, edge1, edge2);
            m3dNormalizeVector(normal);
            glNormal3fv(normal);
        }

        for (int i = 0; i < face.numIndices; i++)
        {
            int idx = face.vIndices[i];
            glVertex3f(model.vertices[idx].x, model.vertices[idx].y, model.vertices[idx].z);
        }
        glEnd();
    }
}

// Draw a rectangular solid with texture coordinates and normals
void DrawBox(float width, float height, float depth)
{
    float w2 = width / 2.0f;
    float h2 = height / 2.0f;
    float d2 = depth / 2.0f;

    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-w2, -h2, d2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(w2, -h2, d2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(w2, h2, d2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-w2, h2, d2);

    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-w2, h2, -d2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(w2, h2, -d2);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(w2, -h2, -d2);

    // Top Face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-w2, h2, -d2);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-w2, h2, d2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(w2, h2, d2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(w2, h2, -d2);

    // Bottom Face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(w2, -h2, -d2);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(w2, -h2, d2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-w2, -h2, d2);

    // Right Face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(w2, -h2, -d2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(w2, h2, -d2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(w2, h2, d2);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(w2, -h2, d2);

    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-w2, -h2, d2);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-w2, h2, d2);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-w2, h2, -d2);
    glEnd();
}

// Helper to draw wireframe facade lines (floors and vertical panels)
void DrawWireframeDetails(float w, float h, float d, float r, float g, float b)
{
    glDisable(GL_LIGHTING);
    glLineWidth(1.5f);
    glColor3f(r, g, b);

    float w2 = w / 2.0f;
    float h2 = h / 2.0f;
    float d2 = d / 2.0f;

    // 1. Draw horizontal floor rings every 0.35 units
    float floorSpacing = 0.35f;
    int numFloors = (int)(h / floorSpacing);
    for (int k = 1; k < numFloors; k++)
    {
        float y = -h2 + k * floorSpacing;
        glBegin(GL_LINE_LOOP);
        glVertex3f(-w2, y, d2);
        glVertex3f(w2, y, d2);
        glVertex3f(w2, y, -d2);
        glVertex3f(-w2, y, -d2);
        glEnd();
    }

    // 2. Draw vertical grid lines on the 4 faces
    glBegin(GL_LINES);
    float xOffsets[] = {-w2 * 0.5f, 0.0f, w2 * 0.5f};
    for (float x : xOffsets)
    {
        glVertex3f(x, -h2, d2);
        glVertex3f(x, h2, d2);
        glVertex3f(x, -h2, -d2);
        glVertex3f(x, h2, -d2);
    }
    float zOffsets[] = {-d2 * 0.5f, 0.0f, d2 * 0.5f};
    for (float z : zOffsets)
    {
        glVertex3f(w2, -h2, z);
        glVertex3f(w2, h2, z);
        glVertex3f(-w2, -h2, z);
        glVertex3f(-w2, h2, z);
    }
    glEnd();

    glEnable(GL_LIGHTING);
    glLineWidth(1.0f);
}

// Helper to draw a glowing warning spire/beacon on tallest towers
void DrawSpire(float yTop, float r, float g, float b)
{
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex3f(0.0f, yTop, 0.0f);
    glVertex3f(0.0f, yTop + 0.6f, 0.0f);
    glEnd();

    // Draw a small glowing orb at the tip
    glPushMatrix();
    glTranslatef(0.0f, yTop + 0.6f, 0.0f);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.1f, 0.0f); // Bright glowing red beacon
    gltDrawSphere(0.05f, 7, 5);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}

// Draws the actual 3D boxes and details for different building styles
// Helper to draw random lit skyscraper windows on the 4 vertical faces
void DrawSkyscraperWindows(float w, float h, float d, float r, float g, float b, int seed)
{
    if (sunlightMode || !skyscraperGlow)
        return;
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // Choose columns and floors based on building size
    int nCols = 4;
    int nFloors = (int)(h / 0.45f);
    if (nFloors < 2)
        nFloors = 2;

    float colW = w / nCols;
    float floorH = h / nFloors;

    float winW = colW * 0.5f;
    float winH = floorH * 0.5f;

    float w2 = w / 2.0f;
    float h2 = h / 2.0f;
    float d2 = d / 2.0f;

    glBegin(GL_QUADS);

    // 1. Front and Back vertical faces
    for (int f = 0; f < nFloors; f++)
    {
        float y = -h2 + (f + 0.5f) * floorH;
        for (int c = 0; c < nCols; c++)
        {
            float x = -w2 + (c + 0.5f) * colW;
            unsigned int winSeed = seed + f * 79 + c * 37;

            // Front Face (z = d2)
            if ((winSeed % 7) < 2) // ~28% lit rate
            {
                if (winSeed % 3 == 0)
                    glColor3f(1.0f, 0.85f, 0.4f); // Warm yellow
                else
                    glColor3f(r * 0.6f + 0.4f, g * 0.6f + 0.4f, b * 0.6f + 0.4f); // Brighter neon-matched

                float z = d2 + 0.003f;
                glVertex3f(x - winW / 2, y - winH / 2, z);
                glVertex3f(x + winW / 2, y - winH / 2, z);
                glVertex3f(x + winW / 2, y + winH / 2, z);
                glVertex3f(x - winW / 2, y + winH / 2, z);
            }

            // Back Face (z = -d2)
            winSeed += 13;
            if ((winSeed % 7) < 2)
            {
                if (winSeed % 3 == 0)
                    glColor3f(1.0f, 0.85f, 0.4f);
                else
                    glColor3f(r * 0.6f + 0.4f, g * 0.6f + 0.4f, b * 0.6f + 0.4f);

                float z = -d2 - 0.003f;
                glVertex3f(x - winW / 2, y - winH / 2, z);
                glVertex3f(x - winW / 2, y + winH / 2, z);
                glVertex3f(x + winW / 2, y + winH / 2, z);
                glVertex3f(x + winW / 2, y - winH / 2, z);
            }
        }
    }

    // 2. Left and Right vertical faces
    int nColsD = (int)(d / colW);
    if (nColsD < 2)
        nColsD = 2;
    float colWD = d / nColsD;
    winW = colWD * 0.5f;

    for (int f = 0; f < nFloors; f++)
    {
        float y = -h2 + (f + 0.5f) * floorH;
        for (int c = 0; c < nColsD; c++)
        {
            float z = -d2 + (c + 0.5f) * colWD;
            unsigned int winSeed = seed + f * 73 + c * 43 + 101;

            // Right Face (x = w2)
            if ((winSeed % 7) < 2)
            {
                if (winSeed % 3 == 0)
                    glColor3f(1.0f, 0.85f, 0.4f);
                else
                    glColor3f(r * 0.6f + 0.4f, g * 0.6f + 0.4f, b * 0.6f + 0.4f);

                float x = w2 + 0.003f;
                glVertex3f(x, y - winH / 2, z - winW / 2);
                glVertex3f(x, y + winH / 2, z - winW / 2);
                glVertex3f(x, y + winH / 2, z + winW / 2);
                glVertex3f(x, y - winH / 2, z + winW / 2);
            }

            // Left Face (x = -w2)
            winSeed += 29;
            if ((winSeed % 7) < 2)
            {
                if (winSeed % 3 == 0)
                    glColor3f(1.0f, 0.85f, 0.4f);
                else
                    glColor3f(r * 0.6f + 0.4f, g * 0.6f + 0.4f, b * 0.6f + 0.4f);

                float x = -w2 - 0.003f;
                glVertex3f(x, y - winH / 2, z - winW / 2);
                glVertex3f(x, y - winH / 2, z + winW / 2);
                glVertex3f(x, y + winH / 2, z + winW / 2);
                glVertex3f(x, y + winH / 2, z - winW / 2);
            }
        }
    }
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Helper to draw a grid of cell lines (floors & columns) on skyscraper faces
void DrawSkyscraperGrid(float w, float h, float d, int nCols, int nFloors)
{
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;
    float d2 = d / 2.0f;

    float colW = w / nCols;
    float floorH = h / nFloors;

    glBegin(GL_LINES);

    // 1. Front and Back faces
    for (int f = 0; f <= nFloors; f++)
    {
        float y = -h2 + f * floorH;
        // Front
        glVertex3f(-w2, y, d2);
        glVertex3f(w2, y, d2);
        // Back
        glVertex3f(-w2, y, -d2);
        glVertex3f(w2, y, -d2);
    }
    for (int c = 0; c <= nCols; c++)
    {
        float x = -w2 + c * colW;
        // Front
        glVertex3f(x, -h2, d2);
        glVertex3f(x, h2, d2);
        // Back
        glVertex3f(x, -h2, -d2);
        glVertex3f(x, h2, -d2);
    }

    // 2. Left and Right faces
    float colD = d / nCols;
    for (int f = 0; f <= nFloors; f++)
    {
        float y = -h2 + f * floorH;
        // Left
        glVertex3f(-w2, y, -d2);
        glVertex3f(-w2, y, d2);
        // Right
        glVertex3f(w2, y, -d2);
        glVertex3f(w2, y, d2);
    }
    for (int c = 0; c <= nCols; c++)
    {
        float z = -d2 + c * colD;
        // Left
        glVertex3f(-w2, -h2, z);
        glVertex3f(-w2, h2, z);
        // Right
        glVertex3f(w2, -h2, z);
        glVertex3f(w2, h2, z);
    }

    glEnd();
}

void DrawDetailedSkyscraper(float w, float h, float d, float r, float g, float b, int style, GLint nShadow)
{
    if (nShadow == 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureObjects[CONCRETE_TEXTURE]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glColor3f(0.45f, 0.47f, 0.50f); // Solid concrete grey color
        glMaterialfv(GL_FRONT, GL_SPECULAR, fLowSpecular);
        glMateriali(GL_FRONT, GL_SHININESS, 10);

        if (skyscraperOutline)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, 1.0f);
        }
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        if (sunlightMode)
            glColor4f(0.0f, 0.0f, 0.0f, 0.35f); // Translucent shadows in daylight
        else
            glColor4f(0.0f, 0.0f, 0.0f, 0.55f); // Soft translucent shadows at night
    }

    // 1. Draw solid boxes (without windows)
    if (style == 0)
    {
        // Style 0: Simple building block
        DrawBox(w, h, d);
    }
    else if (style == 1 || style == 2)
    {
        // Style 1 & 2: Stepped / Tiered tower
        float baseH = h * 0.6f;
        float topH = h * 0.4f;

        // Base Box
        glPushMatrix();
        glTranslatef(0.0f, baseH / 2.0f - h / 2.0f, 0.0f);
        DrawBox(w, baseH, d);
        glPopMatrix();

        // Top Box (narrower)
        float topW = w * 0.7f;
        float topD = d * 0.7f;
        glPushMatrix();
        glTranslatef(0.0f, baseH + topH / 2.0f - h / 2.0f, 0.0f);
        if (nShadow == 0)
        {
            glColor3f(0.45f, 0.47f, 0.50f); // Reset to solid concrete grey color
        }
        DrawBox(topW, topH, topD);
        glPopMatrix();

        // Spire for Style 2
        if (style == 2 && nShadow == 0 && skyscraperGlow)
        {
            DrawSpire(h / 2.0f, r, g, b);
        }
    }

    // 2. Draw Outlines
    if (nShadow == 0 && skyscraperOutline)
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        // Glowing outline using the building's specific theme color
        glColor3f(r, g, b);

        int nCols = 4;
        if (style == 0)
        {
            int nFloors = (int)(h / 0.45f);
            if (nFloors < 2)
                nFloors = 2;
            DrawBox(w, h, d);
            DrawSkyscraperGrid(w, h, d, nCols, nFloors);
        }
        else if (style == 1 || style == 2)
        {
            float baseH = h * 0.6f;
            float topH = h * 0.4f;

            int nFloorsBase = (int)(baseH / 0.45f);
            if (nFloorsBase < 2)
                nFloorsBase = 2;
            int nFloorsTop = (int)(topH / 0.45f);
            if (nFloorsTop < 2)
                nFloorsTop = 2;

            glPushMatrix();
            glTranslatef(0.0f, baseH / 2.0f - h / 2.0f, 0.0f);
            DrawBox(w, baseH, d);
            DrawSkyscraperGrid(w, baseH, d, nCols, nFloorsBase);
            glPopMatrix();

            float topW = w * 0.7f;
            float topD = d * 0.7f;
            glPushMatrix();
            glTranslatef(0.0f, baseH + topH / 2.0f - h / 2.0f, 0.0f);
            DrawBox(topW, topH, topD);
            DrawSkyscraperGrid(topW, topH, topD, nCols, nFloorsTop);
            glPopMatrix();
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }

    // 3. Draw Windows on top of the outlines
    if (nShadow == 0)
    {
        if (style == 0)
        {
            DrawSkyscraperWindows(w, h, d, r, g, b, (int)(w * 100 + h * 10));
        }
        else if (style == 1 || style == 2)
        {
            float baseH = h * 0.6f;
            float topH = h * 0.4f;

            glPushMatrix();
            glTranslatef(0.0f, baseH / 2.0f - h / 2.0f, 0.0f);
            DrawSkyscraperWindows(w, baseH, d, r, g, b, (int)(w * 100 + baseH * 10));
            glPopMatrix();

            float topW = w * 0.7f;
            float topD = d * 0.7f;
            glPushMatrix();
            glTranslatef(0.0f, baseH + topH / 2.0f - h / 2.0f, 0.0f);
            DrawSkyscraperWindows(topW, topH, topD, r * 1.2f, g * 1.2f, b * 1.2f, (int)(topW * 100 + topH * 10 + 50));
            glPopMatrix();
        }
    }

    if (nShadow == 0)
    {
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
    }
}

// Applies rotation/translation for radial block alignment
void DrawCitySkyscraper(float R, float angleInDegrees, float w, float h, float d, float r, float g, float b, int style, GLint nShadow)
{
    glPushMatrix();
    glRotatef(angleInDegrees, 0.0f, 1.0f, 0.0f);
    // Translate with bottom of the building set at y = -0.39f (above road pavement/markings)
    glTranslatef(R, h / 2.0f - 0.39f, 0.0f);
    DrawDetailedSkyscraper(w, h, d, r, g, b, style, nShadow);
    glPopMatrix();
}

// Draw a circular arc sector by sector, skipping the intersection areas exactly
void DrawCircularRoadArc(float R, float marginDistance, bool isDashed)
{
    glBegin(GL_LINES);
    int segmentsPerSector = 24;
    for (int k = 0; k < 6; k++)
    {
        float phiLeft = (float)(M_PI / 6.0f) + k * (float)(M_PI / 3.0f);
        float phiRight = (float)(M_PI / 6.0f) + (k + 1) * (float)(M_PI / 3.0f);

        float thetaStart = phiLeft + asin(marginDistance / R);
        float thetaEnd = phiRight - asin(marginDistance / R);

        float step = (thetaEnd - thetaStart) / segmentsPerSector;
        for (int i = 0; i < segmentsPerSector; i++)
        {
            if (!isDashed || (i % 2 == 0))
            {
                float a1 = thetaStart + i * step;
                float a2 = thetaStart + (i + 1) * step;
                glVertex3f(R * cos(a1), -0.385f, R * sin(a1));
                glVertex3f(R * cos(a2), -0.385f, R * sin(a2));
            }
        }
    }
    glEnd();
}

// // Draw a circular paved road strip using dark asphalt color and shoulder/divider markings
// detailLevel: 0 = raw pavement, 1 = solid borders, 2 = solid borders + dashed center divider
void DrawCircularRoad(float rInner, float rOuter, int detailLevel = 2)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureObjects[ASPHALT_TEXTURE]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Asphalt base (drawn at y = -0.398f to sit cleanly above ground y = -0.4f)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUAD_STRIP);
    int segments = 120; // more segments for smoother circles
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * M_PI * i / segments;
        float c = cos(angle);
        float s = sin(angle);
        glNormal3f(0.0f, 1.0f, 0.0f);
        float tCoord = angle * 4.0f; // Tile asphalt along the circle
        glTexCoord2f(tCoord, 0.0f);
        glVertex3f(rOuter * c, -0.398f, rOuter * s);
        glTexCoord2f(tCoord, 1.0f);
        glVertex3f(rInner * c, -0.398f, rInner * s);
    }
    glEnd();

    if (detailLevel > 0)
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(1.5f);

        // 1. Draw solid outer and inner shoulder lines (at y = -0.395f to sit above asphalt)
        glColor3f(0.8f, 0.8f, 0.75f); // light-gray/off-white borders

        // Inner shoulder line
        DrawCircularRoadArc(rInner + 0.05f, 0.55f, false);

        // Outer shoulder line
        DrawCircularRoadArc(rOuter - 0.05f, 0.55f, false);

        // 2. Draw dashed middle lane (at y = -0.395f)
        if (detailLevel == 2)
        {
            glColor3f(1.0f, 1.0f, 0.9f); // warm-white lane dashes
            float rMid = (rInner + rOuter) / 2.0f;
            DrawCircularRoadArc(rMid, 0.70f, true);
        }

        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }

    glDisable(GL_BLEND);
}

// Draw the complete road grid (circular ring roads, radial avenues, and minor streets)
void DrawRoads(void)
{
    glEnable(GL_TEXTURE_2D);
    // Concentric paved circular roads & park promenade
    DrawCircularRoad(3.5f, 3.8f, 0);     // Park Promenade (just raw pavement)
    DrawCircularRoad(4.95f, 5.25f, 1);   // Minor Concentric Road 1 (borders only)
    DrawCircularRoad(7.0f, 8.5f, 2);     // Main Ring Road 1 (borders + center lane)
    DrawCircularRoad(9.95f, 10.45f, 1);  // Minor Concentric Road 2 (borders only)
    DrawCircularRoad(12.0f, 13.5f, 2);   // Main Ring Road 2 (borders + center lane)
    DrawCircularRoad(14.95f, 15.85f, 2); // Main Ring Road 3 (borders + center lane)
    DrawCircularRoad(16.85f, 17.75f, 2); // Main Ring Road 4 (borders + center lane)

    // Pave 6 radial straight avenues (spokes) at 60 degree intervals (at y = -0.398f)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureObjects[ASPHALT_TEXTURE]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1.0f, 1.0f, 1.0f);

    for (int k = 0; k < 6; k++)
    {
        glPushMatrix();
        glRotatef(k * 60.0f, 0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        // Corrected CCW winding order to avoid backface culling
        glTexCoord2f(0.0f, 3.5f);
        glVertex3f(0.6f, -0.398f, 3.5f);
        glTexCoord2f(1.0f, 3.5f);
        glVertex3f(-0.6f, -0.398f, 3.5f);
        glTexCoord2f(1.0f, 19.2f);
        glVertex3f(-0.6f, -0.398f, 19.2f);
        glTexCoord2f(0.0f, 19.2f);
        glVertex3f(0.6f, -0.398f, 19.2f);
        glEnd();
        glPopMatrix();
    }

    // Draw lane markings for the 6 radial roads (at y = -0.395f to sit above asphalt)
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    for (int k = 0; k < 6; k++)
    {
        glPushMatrix();
        glRotatef(k * 60.0f, 0.0f, 1.0f, 0.0f);

        // 1. Draw dashed middle lane (white) - breaking at intersections with a buffer
        glColor3f(1.0f, 1.0f, 0.9f);
        glLineWidth(1.5f);
        glBegin(GL_LINES);

        float dashRanges[][2] = {
            {3.95f, 4.80f},
            {5.40f, 6.85f},
            {8.65f, 9.80f},
            {10.60f, 11.85f},
            {13.65f, 14.80f},
            {16.00f, 16.70f},
            {17.90f, 19.2f}};

        for (int r = 0; r < 7; r++)
        {
            float rStart = dashRanges[r][0];
            float rEnd = dashRanges[r][1];
            for (float z = rStart; z < rEnd; z += 0.8f)
            {
                float zEnd = (z + 0.4f < rEnd) ? (z + 0.4f) : rEnd;
                glVertex3f(0.0f, -0.385f, z);
                glVertex3f(0.0f, -0.385f, zEnd);
            }
        }
        glEnd();

        // 2. Draw solid outer shoulder lines (grayish-white) - stopping exactly at concentric road shoulders
        glColor3f(0.8f, 0.8f, 0.75f);
        glBegin(GL_LINES);

        float zRanges[][2] = {
            {3.5071f, 4.9697f},
            {5.1708f, 7.0285f},
            {8.4320f, 9.9848f},
            {10.3854f, 12.0374f},
            {13.4387f, 14.9899f},
            {15.7904f, 16.8910f},
            {17.6914f, 19.2f}};

        for (int r = 0; r < 7; r++)
        {
            float zStart = zRanges[r][0];
            float zEnd = zRanges[r][1];

            glVertex3f(-0.55f, -0.385f, zStart);
            glVertex3f(-0.55f, -0.385f, zEnd);
            glVertex3f(0.55f, -0.385f, zStart);
            glVertex3f(0.55f, -0.385f, zEnd);
        }
        glEnd();

        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
}

// Draw a simple low-poly tree for the central park
void DrawTree(float x, float z)
{
    glPushMatrix();
    // Trees sit at y = -0.39f, resting cleanly on the ground
    glTranslatef(x, -0.39f, z);

    // Trunk (Brown box)
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.40f, 0.25f, 0.12f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    DrawBox(0.08f, 0.4f, 0.08f);
    glPopMatrix();

    // Foliage (Green sphere)
    glColor3f(0.12f, 0.65f, 0.22f);
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.0f);
    gltDrawSphere(0.18f, 9, 5);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

// Draw a detailed 3D streetlight with a self-illuminating glowing bulb and ground spotlight pool
void DrawStreetlight(float R, float angleInDegrees, float armDir, float r, float g, float b)
{
    glPushMatrix();
    glRotatef(angleInDegrees, 0.0f, 1.0f, 0.0f);
    // Base of the pole is at radius R, y = -0.39f (which is ground level)
    glTranslatef(R, 0.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);

    // Pole
    glPushMatrix();
    glTranslatef(0.0f, 0.21f, 0.0f); // Center of the 1.2f high pole starting at -0.39f
    glColor3f(0.25f, 0.25f, 0.28f);  // Dark metallic gray/silver
    DrawBox(0.04f, 1.2f, 0.04f);
    glPopMatrix();

    // Arm
    glPushMatrix();
    glTranslatef(armDir * 0.15f, 0.81f, 0.0f); // Horizontal arm at the top of the pole
    glColor3f(0.25f, 0.25f, 0.28f);
    DrawBox(0.3f, 0.03f, 0.03f);
    glPopMatrix();

    // Bulb
    glPushMatrix();
    glTranslatef(armDir * 0.3f, 0.81f, 0.0f); // Bulb at the tip of the arm

    glDisable(GL_TEXTURE_2D); // Solid color
    if (sunlightMode)
    {
        glColor3f(0.35f, 0.35f, 0.35f); // Matte off-white/grey bulb during day
        gltDrawSphere(0.07f, 15, 9);
    }
    else
    {
        glDisable(GL_LIGHTING);       // Self-illuminated
        glColor3f(1.0f, 0.95f, 0.8f); // Bright warm white glow
        gltDrawSphere(0.07f, 15, 9);
        glEnable(GL_LIGHTING);
    }
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();

    if (!sunlightMode)
    {
        glDepthMask(GL_FALSE); // Disable depth writes for transparent components

        // Volumetric Spotlight Cone (fades from bulb to ground)
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE); // Disable culling so we see both front and back of the transparent cone

        glBegin(GL_QUAD_STRIP);
        int segments = 16;
        float bulbX = armDir * 0.3f;
        float bulbY = 0.81f;
        for (int j = 0; j <= segments; j++)
        {
            float rad = 2.0f * M_PI * j / segments;
            float c = cos(rad);
            float s = sin(rad);

            // Top vertex (near bulb, higher alpha)
            glColor4f(r, g, b, 0.25f);
            glVertex3f(bulbX + 0.03f * c, bulbY - 0.02f, 0.03f * s);

            // Bottom vertex (on ground, zero alpha for smooth fade)
            glColor4f(r, g, b, 0.0f);
            glVertex3f(bulbX + 1.5f * c, -0.39f, 1.5f * s);
        }
        glEnd();
        glEnable(GL_CULL_FACE); // Re-enable culling

        // Spotlight Ground Pool (light cookie on the pavement)
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureObjects[SPOTLIGHT_TEXTURE]);

        glColor4f(r, g, b, 0.98f);
        float poolRadius = 1.5f;
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        // Corrected CCW winding order to prevent culling
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(bulbX - poolRadius, -0.388f, poolRadius);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(bulbX + poolRadius, -0.388f, poolRadius);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(bulbX + poolRadius, -0.388f, -poolRadius);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(bulbX - poolRadius, -0.388f, -poolRadius);
        glEnd();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glDepthMask(GL_TRUE); // Re-enable depth writes
    }
    glPopMatrix();
}

// Draw a detailed 3D streetlight along the radial avenues
void DrawRadialStreetlight(float roadAngle, float xOffset, float zOffset, float armDir, float r, float g, float b)
{
    glPushMatrix();
    glRotatef(roadAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(xOffset, 0.0f, zOffset);

    glDisable(GL_TEXTURE_2D);

    // Pole
    glPushMatrix();
    glTranslatef(0.0f, 0.21f, 0.0f);
    glColor3f(0.25f, 0.25f, 0.28f);
    DrawBox(0.04f, 1.2f, 0.04f);
    glPopMatrix();

    // Arm
    glPushMatrix();
    glTranslatef(armDir * 0.15f, 0.81f, 0.0f);
    glColor3f(0.25f, 0.25f, 0.28f);
    DrawBox(0.3f, 0.03f, 0.03f);
    glPopMatrix();

    // Bulb
    glPushMatrix();
    glTranslatef(armDir * 0.3f, 0.81f, 0.0f);

    glDisable(GL_TEXTURE_2D); // Solid color
    if (sunlightMode)
    {
        glColor3f(0.35f, 0.35f, 0.35f); // Matte off-white/grey bulb during day
        gltDrawSphere(0.07f, 15, 9);
    }
    else
    {
        glDisable(GL_LIGHTING);       // Self-illuminated
        glColor3f(1.0f, 0.95f, 0.8f); // Bright warm white glow
        gltDrawSphere(0.07f, 15, 9);
        glEnable(GL_LIGHTING);
    }
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();

    if (!sunlightMode)
    {
        glDepthMask(GL_FALSE); // Disable depth writes for transparent components

        // Volumetric Spotlight Cone (fades from bulb to ground)
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE); // Disable culling so we see both front and back of the transparent cone

        glBegin(GL_QUAD_STRIP);
        int segments = 16;
        float bulbX = armDir * 0.3f;
        float bulbY = 0.81f;
        for (int j = 0; j <= segments; j++)
        {
            float rad = 2.0f * M_PI * j / segments;
            float c = cos(rad);
            float s = sin(rad);

            // Top vertex (near bulb, higher alpha)
            glColor4f(r, g, b, 0.25f);
            glVertex3f(bulbX + 0.03f * c, bulbY - 0.02f, 0.03f * s);

            // Bottom vertex (on ground, zero alpha for smooth fade)
            glColor4f(r, g, b, 0.0f);
            glVertex3f(bulbX + 1.5f * c, -0.39f, 1.5f * s);
        }
        glEnd();
        glEnable(GL_CULL_FACE); // Re-enable culling

        // Spotlight Ground Pool (light cookie on the pavement)
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureObjects[SPOTLIGHT_TEXTURE]);

        glColor4f(r, g, b, 0.98f);
        float poolRadius = 1.5f;
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        // Corrected CCW winding order to prevent culling
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(bulbX - poolRadius, -0.388f, poolRadius);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(bulbX + poolRadius, -0.388f, poolRadius);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(bulbX + poolRadius, -0.388f, -poolRadius);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(bulbX - poolRadius, -0.388f, -poolRadius);
        glEnd();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glDepthMask(GL_TRUE); // Re-enable depth writes
    }
    glPopMatrix();
}

// Draw 6 laser spotlights targeting the central statue (robot)
void DrawLaserBeams(void)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow
    glDisable(GL_TEXTURE_2D);

    float colors[6][3] = {
        {1.0f, 0.1f, 0.2f}, // Neon Red
        {0.0f, 0.9f, 1.0f}, // Neon Cyan
        {0.2f, 1.0f, 0.3f}, // Neon Green
        {1.0f, 0.6f, 0.0f}, // Neon Orange
        {0.8f, 0.1f, 1.0f}, // Neon Purple
        {0.9f, 0.9f, 0.1f}  // Neon Yellow
    };

    float targetY = 0.8f; // Target height (robot torso)

    for (int i = 0; i < 6; i++)
    {
        float angle = i * 60.0f * (float)(M_PI / 180.0f);
        float startX = 3.5f * cos(angle);
        float startZ = 3.5f * sin(angle);
        float startY = 1.5f;

        // Draw emitter post at the start position
        glColor3f(0.25f, 0.25f, 0.28f);
        glPushMatrix();
        glTranslatef(startX, (startY - (-0.39f)) / 2.0f - 0.39f, startZ); // pole from ground to 1.5f
        DrawBox(0.04f, startY - (-0.39f), 0.04f);
        glPopMatrix();

        // Draw glowing emitter bulb at the top of the post
        glColor3fv(colors[i]);
        glPushMatrix();
        glTranslatef(startX, startY, startZ);
        gltDrawSphere(0.05f, 9, 7);
        glPopMatrix();

        // Compute direction vector
        float dx = -startX;
        float dy = targetY - startY;
        float dz = -startZ;
        float len = sqrt(dx * dx + dy * dy + dz * dz);
        if (len < 0.01f)
            continue;

        float ux = dx / len;
        float uy = dy / len;
        float uz = dz / len;

        // Perpendicular vectors
        float px = -uz;
        float py = 0.0f;
        float pz = ux;
        float plen = sqrt(px * px + pz * pz);
        if (plen < 0.01f)
        {
            px = 1.0f;
            py = 0.0f;
            pz = 0.0f;
        }
        else
        {
            px /= plen;
            pz /= plen;
        }

        float qx = uy * pz - uz * py;
        float qy = uz * px - ux * pz;
        float qz = ux * py - uy * px;

        // Draw cylinder using GL_QUAD_STRIP
        glDepthMask(GL_FALSE); // Disable depth writes for transparent beam
        glBegin(GL_QUAD_STRIP);
        int segments = 8;
        float radiusStart = 0.015f; // Thin beam at start
        float radiusEnd = 0.06f;    // Spreads out slightly at the target

        for (int j = 0; j <= segments; j++)
        {
            float rad = 2.0f * M_PI * j / segments;
            float c = cos(rad);
            float s = sin(rad);

            float ox = c * px + s * qx;
            float oy = c * py + s * qy;
            float oz = c * pz + s * qz;

            // Start vertex (near emitter)
            glColor4f(colors[i][0], colors[i][1], colors[i][2], 0.7f);
            glVertex3f(startX + radiusStart * ox, startY + radiusStart * oy, startZ + radiusStart * oz);

            // End vertex (near statue)
            glColor4f(colors[i][0], colors[i][1], colors[i][2], 0.1f);
            glVertex3f(0.0f + radiusEnd * ox, targetY + radiusEnd * oy, 0.0f + radiusEnd * oz);
        }
        glEnd();
        glDepthMask(GL_TRUE); // Re-enable depth writes
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// Draw the central spinning humanoid robot with swinging walk animation limbs
void DrawRobot(GLint nShadow)
{
    static float robotAnimTime = 0.0f;
    if (nShadow == 0 && timer_flag)
    {
        robotAnimTime += 0.08f;
    }

    float swingAngle = sin(robotAnimTime) * 35.0f;

    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[METAL_TEXTURE]);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        if (sunlightMode)
        {
            glMaterialfv(GL_FRONT, GL_EMISSION, fNoLight);
        }
        else
        {
            // Set soft ambient/diffuse emission to make it glow under the laser spotlights
            GLfloat robotEmission[] = {0.35f, 0.4f, 0.45f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, robotEmission);
        }
    }
    else
    {
        if (sunlightMode)
            glColor4f(0.00f, 0.00f, 0.00f, 0.25f);
        else
            glColor4f(0.00f, 0.00f, 0.00f, 0.45f);
    }

    // Torso (body central core)
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f); // raise above ground
    gltDrawSphere(0.32f, 21, 11);

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.0f);
    gltDrawSphere(0.18f, 15, 9);

    // Sensor eye (laser red dot scanner)
    if (nShadow == 0)
    {
        glDisable(GL_TEXTURE_2D);
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.16f);
    gltDrawSphere(0.04f, 9, 5);
    glPopMatrix();
    if (nShadow == 0)
    {
        glEnable(GL_TEXTURE_2D);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    glPopMatrix(); // End head

    // Left Arm
    glPushMatrix();
    glTranslatef(-0.42f, 0.15f, 0.0f);
    glRotatef(swingAngle, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.2f, 0.0f);
    DrawBox(0.10f, 0.4f, 0.10f);
    glPopMatrix();

    // Right Arm
    glPushMatrix();
    glTranslatef(0.42f, 0.15f, 0.0f);
    glRotatef(-swingAngle, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.2f, 0.0f);
    DrawBox(0.10f, 0.4f, 0.10f);
    glPopMatrix();

    glPopMatrix(); // End torso

    // Left Leg
    glPushMatrix();
    glTranslatef(-0.16f, 0.25f, 0.0f);
    glRotatef(-swingAngle, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.25f, 0.0f);
    DrawBox(0.12f, 0.5f, 0.12f);
    glPopMatrix();

    // Right Leg
    glPushMatrix();
    glTranslatef(0.16f, 0.25f, 0.0f);
    glRotatef(swingAngle, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.25f, 0.0f);
    DrawBox(0.12f, 0.5f, 0.12f);
    glPopMatrix();

    // Reset emission to zero
    if (nShadow == 0)
    {
        GLfloat fNoEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, fNoEmission);
    }
}

// Draw a very dense star-city block layout inspired by Palmanova Italy
// featuring dense row-buildings grouped into sectors divided by 6 radial avenues
void DrawCity(GLint nShadow)
{
    // Roads are now drawn separately via DrawRoads() in RenderScene to ensure correct shadow projection.

    // 2. Draw Skyscrapers in Concentric Rings (highly dense row placement, aligned radially)
    // Cohesive neon colors for each sub-ring to create stunning concentric patterns
    float ringColors[7][3] = {
        {1.0f, 0.1f, 0.6f}, // Ring 1a: Magenta/Pink
        {1.0f, 0.5f, 0.0f}, // Ring 1b: Orange
        {0.0f, 0.9f, 1.0f}, // Ring 2a: Cyan
        {0.1f, 0.3f, 1.0f}, // Ring 2b: Electric Blue
        {0.7f, 0.0f, 1.0f}, // Ring 3a: Purple
        {0.2f, 1.0f, 0.3f}, // Ring 3b: Green
        {0.9f, 0.9f, 0.1f}  // Ring 3c: Neon Yellow
    };

    // Sub-ring structural definitions
    // We define the 7 sub-rings with their radius, slot count, base height, height variation, and radial depth.
    struct SubRingInfo
    {
        float radius;
        int slots;
        float baseH;
        float varH;
        float radialW;
        int colorIdx;
    };

    SubRingInfo subRings[7] = {
        {4.375f, 36, 1.4f, 0.25f, 0.85f, 0},   // Inner Ring 1a
        {6.125f, 48, 1.9f, 0.30f, 1.45f, 1},   // Inner Ring 1b
        {9.225f, 72, 2.8f, 0.40f, 1.15f, 2},   // Middle Ring 2a
        {11.225f, 84, 3.6f, 0.50f, 1.25f, 3},  // Middle Ring 2b
        {14.225f, 108, 4.8f, 0.60f, 1.15f, 4}, // Outer Ring 3a
        {16.35f, 120, 5.8f, 0.70f, 0.70f, 5},  // Outer Ring 3b
        {18.40f, 132, 6.8f, 0.80f, 1.00f, 6}   // Outer Ring 3c
    };

    for (int ringIdx = 0; ringIdx < 7; ringIdx++)
    {
        float R = subRings[ringIdx].radius;
        int N = subRings[ringIdx].slots;
        float baseH = subRings[ringIdx].baseH;
        float varH = subRings[ringIdx].varH;
        float w = subRings[ringIdx].radialW;
        int colorIdx = subRings[ringIdx].colorIdx;

        // Calculate tangential width d to leave a small gap of 0.08 units between buildings
        float arcLength = (2.0f * M_PI * R) / N;
        float d = arcLength - 0.08f;
        if (d < 0.2f)
            d = 0.2f; // safety minimum

        float r = ringColors[colorIdx][0];
        float g = ringColors[colorIdx][1];
        float b = ringColors[colorIdx][2];

        for (int i = 0; i < N; i++)
        {
            float angle = i * (2.0f * M_PI / N);

            // Road safety check using our exact geometric distance formula (offsetting by M_PI/6 to match Z-aligned roads)
            float diff = fmod(angle - (float)(M_PI / 6.0f) + 2.0f * M_PI, (float)(M_PI / 3.0f));
            float deltaTheta = (diff < (float)(M_PI / 6.0f)) ? diff : (((float)(M_PI / 3.0f)) - diff);
            float distToRoad = (R - w / 2.0f) * sin(deltaTheta) - (d / 2.0f) * cos(deltaTheta);

            // Skip buildings that overlap or lie too close to the radial roads (0.585 units road margin for tight sidewalk fit)
            if (distToRoad < 0.585f)
                continue;

            // Height calculation with variance to create realistic skyline blocks
            float h = baseH + sin(i * 0.7f) * varH + (i % 3) * 0.25f;

            // Determine building style based on ring radius and position
            int style = 0;
            if (R < 7.0f)
            {
                style = (i % 3 == 0) ? 1 : 0;
            }
            else if (R < 12.0f)
            {
                style = (i % 2 == 0) ? 1 : 2;
            }
            else
            {
                style = (i % 3 == 0) ? 0 : ((i % 3 == 1) ? 1 : 2);
            }

            DrawCitySkyscraper(R, angle * (180.0f / M_PI), w, h, d, r, g, b, style, nShadow);
        }
    }

    // 3. Central Park Details (Circular ring of trees offset from radial street exits)
    if (nShadow == 0)
    {
        for (int i = 0; i < 6; i++)
        {
            // Trees placed at 30, 90, 150, 210, 270, 330 degrees (clear of the avenues)
            float angle = (i * 60.0f + 30.0f) * (M_PI / 180.0f);
            DrawTree(2.8f * cos(angle), 2.8f * sin(angle));
        }
    }

    // 4. Detailed 3D Streetlights (only drawn once, not in shadow passes, for performance)
    if (nShadow == 0)
    {
        // 4.1 Concentric curbside streetlights
        for (int i = 0; i < 12; i++)
        {
            // Position at 12 offset angles: i * 30 + 15 degrees to sit in the middle of blocks, clearing radial avenues
            float angle = i * 30.0f + 15.0f;

            // Main Ring Road 1 (R=7.0 to 8.5)
            // Inner curb: R=6.95f, pointing outwards (armDir = 1.0f), warm neon orange/yellow light
            DrawStreetlight(6.95f, angle, 1.0f, 1.0f, 0.5f, 0.0f);
            // Outer curb: R=8.55f, pointing inwards (armDir = -1.0f), warm neon orange/yellow light
            DrawStreetlight(8.55f, angle, -1.0f, 1.0f, 0.5f, 0.0f);

            // Main Ring Road 2 (R=12.0 to 13.5)
            // Inner curb: R=11.95f, pointing outwards (armDir = 1.0f), electric cyan light
            DrawStreetlight(11.95f, angle, 1.0f, 0.0f, 0.9f, 1.0f);
            // Outer curb: R=13.55f, pointing inwards (armDir = -1.0f), electric cyan light
            DrawStreetlight(13.55f, angle, -1.0f, 0.0f, 0.9f, 1.0f);

            // Main Ring Road 3 (R=14.95 to 15.85)
            // Inner curb: R=14.90f, pointing outwards (armDir = 1.0f), neon green light
            DrawStreetlight(14.90f, angle, 1.0f, 0.2f, 1.0f, 0.3f);
            // Outer curb: R=15.90f, pointing inwards (armDir = -1.0f), neon green light
            DrawStreetlight(15.90f, angle, -1.0f, 0.2f, 1.0f, 0.3f);

            // Main Ring Road 4 (R=16.85 to 17.75)
            // Inner curb: R=16.80f, pointing outwards (armDir = 1.0f), neon yellow light
            DrawStreetlight(16.80f, angle, 1.0f, 0.9f, 0.9f, 0.1f);
            // Outer curb: R=17.80f, pointing inwards (armDir = -1.0f), neon yellow light
            DrawStreetlight(17.80f, angle, -1.0f, 0.9f, 0.9f, 0.1f);
        }

        // 4.2 Radial (vertical) road streetlights along the 6 radial spokes
        float radialZPositions[] = {4.375f, 6.125f, 9.225f, 11.225f, 14.225f, 16.35f, 18.40f};
        float colors[7][3] = {
            {1.0f, 0.1f, 0.6f}, // Magenta/Pink
            {1.0f, 0.5f, 0.0f}, // Orange
            {0.0f, 0.9f, 1.0f}, // Cyan
            {0.1f, 0.3f, 1.0f}, // Electric Blue
            {0.7f, 0.0f, 1.0f}, // Purple
            {0.2f, 1.0f, 0.3f}, // Green
            {0.9f, 0.9f, 0.1f}  // Neon Yellow
        };
        for (int k = 0; k < 6; k++)
        {
            float roadAngle = k * 60.0f;
            for (int r = 0; r < 7; r++)
            {
                float z = radialZPositions[r];
                // Left side of the road (X = -0.65f, pointing inwards armDir = 1.0f)
                DrawRadialStreetlight(roadAngle, -0.65f, z, 1.0f, colors[r][0], colors[r][1], colors[r][2]);
                // Right side of the road (X = 0.65f, pointing inwards armDir = -1.0f)
                DrawRadialStreetlight(roadAngle, 0.65f, z, -1.0f, colors[r][0], colors[r][1], colors[r][2]);
            }
        }

        // 4.3 Laser spotlights targeting the central robot statue
        if (!sunlightMode)
            DrawLaserBeams();
    }
}

// This function does any needed initialization on the rendering context.
void SetupRC()
{
    // Log depth and stencil bits
    std::ofstream debugLog("opengl_debug.log");
    if (debugLog.is_open()) {
        GLint depthBits, stencilBits;
        glGetIntegerv(GL_DEPTH_BITS, &depthBits);
        glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        debugLog << "Depth Buffer Bits: " << depthBits << std::endl;
        debugLog << "Stencil Buffer Bits: " << stencilBits << std::endl;
        debugLog.close();
    }

    M3DVector3f vPoints[3] = {{0.0f, -0.4f, 0.0f},
                              {10.0f, -0.4f, 0.0f},
                              {5.0f, -0.4f, -5.0f}};
    int iSphere;
    int i;

    // Grayish background
    glClearColor(fLowLight[0], fLowLight[1], fLowLight[2], fLowLight[3]);

    // Clear stencil buffer with zero, increment by one whenever anybody
    // draws into it. When stencil function is enabled, only write where
    // stencil value is zero. This prevents the transparent shadow from drawing
    // over itself
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glClearStencil(0);
    glStencilFunc(GL_EQUAL, 0x0, 0x01);

    // Cull backs of polygons
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE_ARB);

    // Setup light 0 parameters (Sun/Moon light)
    GLfloat fGlobalAmbient[] = {0.1f, 0.1f, 0.15f, 1.0f}; // Soft dark-blue global ambient to mimic city light scattering
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fGlobalAmbient);
    // Make moonlight soft for night environment so it doesn't overpower the streetlights
    GLfloat fDimMoon[] = {0.15f, 0.15f, 0.18f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, fNoLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, fDimMoon);
    glLightfv(GL_LIGHT0, GL_SPECULAR, fDimMoon);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    // Setup light 1 parameters (Central Neon Point Light - softer so streetlights stand out)
    GLfloat fNeonAmbient[] = {0.05f, 0.08f, 0.08f, 1.0f};
    GLfloat fNeonDiffuse[] = {0.25f, 0.35f, 0.40f, 1.0f};
    GLfloat fNeonSpecular[] = {0.25f, 0.35f, 0.40f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, fNeonAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fNeonDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, fNeonSpecular);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.08f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.005f);
    glEnable(GL_LIGHT1);

    // Setup light 2 to 5 parameters (Dynamic Curbside Streetlights)
    for (int l = GL_LIGHT2; l <= GL_LIGHT5; l++)
    {
        glLightf(l, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(l, GL_LINEAR_ATTENUATION, 0.4f);
        glLightf(l, GL_QUADRATIC_ATTENUATION, 0.08f);
        glLightf(l, GL_SPOT_CUTOFF, 45.0f);
        glLightf(l, GL_SPOT_EXPONENT, 4.0f);
        GLfloat spotDir[] = {0.0f, -1.0f, 0.0f};
        glLightfv(l, GL_SPOT_DIRECTION, spotDir);
    }

    // Setup light 6 parameters (Ground Bounce / Indirect Light)
    // Warm, low-intensity light pointing upwards to simulate bounced indirect illumination
    GLfloat fGroundAmbient[] = {0.05f, 0.05f, 0.06f, 1.0f};
    GLfloat fGroundDiffuse[] = {0.15f, 0.13f, 0.10f, 1.0f};
    glLightfv(GL_LIGHT6, GL_AMBIENT, fGroundAmbient);
    glLightfv(GL_LIGHT6, GL_DIFFUSE, fGroundDiffuse);
    glLightfv(GL_LIGHT6, GL_SPECULAR, fNoLight);
    glEnable(GL_LIGHT6);

    glEnable(GL_LIGHTING);

    // Calculate shadow matrices for both light sources
    M3DVector4f pPlane;
    m3dGetPlaneEquation(pPlane, vPoints[0], vPoints[1], vPoints[2]);
    m3dMakePlanarShadowMatrix(mShadowMatrix0, pPlane, fLightPos0);
    m3dMakePlanarShadowMatrix(mShadowMatrix1, pPlane, fLightPos1);

    // Mostly use material tracking
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, fBrightLight);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    // Place the 30 pedestrian inhabitants with randomized attributes (color, scale, walk phase)
    for (iSphere = 0; iSphere < NUM_SPHERES; iSphere++)
    {
        float radius = (rand() % 2 == 0) ? 7.75f : 12.75f; // On inner or outer road ring
        float angle = ((float)rand() / RAND_MAX) * 2.0f * M_PI;

        pedestrians[iSphere].frame.SetOrigin(radius * cos(angle), 0.0f, radius * sin(angle));

        // Random attributes to avoid copy-paste appearances
        pedestrians[iSphere].walkPhase = (float)(rand() % 1000) / 1000.0f * 2.0f * M_PI;
        pedestrians[iSphere].walkSpeed = 0.002f + (float)(rand() % 100) / 100.0f * 0.003f;
        pedestrians[iSphere].scale = 0.0018f + (float)(rand() % 100) / 100.0f * 0.0004f; // 0.0018 to 0.0022

        // Torso color (random neon hues)
        pedestrians[iSphere].colorTorso[0] = (float)(rand() % 100) / 100.0f * 0.8f + 0.2f;
        pedestrians[iSphere].colorTorso[1] = (float)(rand() % 100) / 100.0f * 0.8f + 0.2f;
        pedestrians[iSphere].colorTorso[2] = (float)(rand() % 100) / 100.0f * 0.8f + 0.2f;

        // Head color (glowing orange/yellow/red)
        pedestrians[iSphere].colorHead[0] = (float)(rand() % 100) / 100.0f * 0.9f + 0.1f;
        pedestrians[iSphere].colorHead[1] = (float)(rand() % 100) / 100.0f * 0.9f + 0.1f;
        pedestrians[iSphere].colorHead[2] = 0.1f;

        // Leg color (mostly dark or matching)
        pedestrians[iSphere].colorLegs[0] = pedestrians[iSphere].colorTorso[0] * 0.5f;
        pedestrians[iSphere].colorLegs[1] = pedestrians[iSphere].colorTorso[1] * 0.5f;
        pedestrians[iSphere].colorLegs[2] = pedestrians[iSphere].colorTorso[2] * 0.5f;
    }

    // Set up texture maps
    glEnable(GL_TEXTURE_2D);
    glGenTextures(NUM_TEXTURES, textureObjects);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    for (i = 0; i < NUM_TEXTURES; i++)
    {
        GLbyte *pBytes;
        GLint iWidth, iHeight, iComponents;
        GLenum eFormat;

        glBindTexture(GL_TEXTURE_2D, textureObjects[i]);

        // Load this texture map
        pBytes = gltLoadTGA(szTextureFiles[i], &iWidth, &iHeight, &iComponents, &eFormat);
        gluBuild2DMipmaps(GL_TEXTURE_2D, iComponents, iWidth, iHeight, eFormat, GL_UNSIGNED_BYTE, pBytes);
        free(pBytes);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Load external models
    LoadOBJModel("materials/drone.obj", modelDrone);
}

// Do shutdown for the rendering context
void ShutdownRC(void)
{
    glDeleteTextures(NUM_TEXTURES, textureObjects);
}

// Draw the ground pavement as a series of triangle strips with floor texture
void DrawGround(void)
{
    glEnable(GL_TEXTURE_2D);
    GLfloat fExtent = 300.0f; // Cover the full horizon up to the far clipping plane
    GLfloat fStep = 5.0f;     // Grid step size for performance
    GLfloat y = -0.4f;
    GLfloat iStrip, iRun;

    glBindTexture(GL_TEXTURE_2D, textureObjects[CONCRETE_TEXTURE]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    for (iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
        {
            glTexCoord2f(iStrip * 0.1111f, iRun * 0.1111f);
            glNormal3f(0.0f, 1.0f, 0.0f); // All Point up
            glVertex3f(iStrip, y, iRun);

            glTexCoord2f((iStrip + fStep) * 0.1111f, iRun * 0.1111f);
            glNormal3f(0.0f, 1.0f, 0.0f); // All Point up
            glVertex3f(iStrip + fStep, y, iRun);
        }
        glEnd();
    }
}

// Hierarchical rendering for Lab 13 style walking animation
void DrawHierarchicalPedestrian(const PedestrianInfo &ped, int nShadow)
{
    float phase = ped.walkPhase;

    // Swing angles matching Lab 13 math
    float headRot = 20.0f * sinf(phase);
    float upperArmRotL = 45.0f * sinf(phase);
    float lowerArmRotL = -30.0f * (sinf(phase) + 1.0f);
    float upperArmRotR = 45.0f * sinf(phase + 3.14159f);
    float lowerArmRotR = -30.0f * (sinf(phase + 3.14159f) + 1.0f);
    float upperLegRotL = 30.0f * sinf(phase + 3.14159f);
    float lowerLegRotL = 20.0f * (sinf(phase + 3.14159f) + 1.0f);
    float upperLegRotR = 30.0f * sinf(phase);
    float lowerLegRotR = 20.0f * (sinf(phase) + 1.0f);

    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[HOLO_TEXTURE]);
        glColor3fv(ped.colorTorso);
        if (sunlightMode)
        {
            glMaterialfv(GL_FRONT, GL_EMISSION, fNoLight);
        }
        else
        {
            // Set soft emission matching neon body color so pedestrians remain visible in darker road blocks
            GLfloat fPedEmission[4] = {ped.colorTorso[0] * 0.45f, ped.colorTorso[1] * 0.45f, ped.colorTorso[2] * 0.45f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, fPedEmission);
        }
    }
    else
    {
        if (sunlightMode)
            glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
        else
            glColor4f(0.0f, 0.0f, 0.0f, 0.45f);
    }

    // Torso (height: 40, width: 20, depth: 10). Origin at center.
    glPushMatrix();
    DrawBox(20.0f, 40.0f, 10.0f);

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 25.0f, 0.0f);
    glRotatef(headRot, 0.0f, 1.0f, 0.0f);
    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[NEON_TEXTURE]); // Glowing eye/face
        glColor3fv(ped.colorHead);
        if (sunlightMode)
        {
            glMaterialfv(GL_FRONT, GL_EMISSION, fNoLight);
        }
        else
        {
            // Set brighter head emission
            GLfloat fHeadEmission[4] = {ped.colorHead[0] * 0.6f, ped.colorHead[1] * 0.6f, ped.colorHead[2] * 0.6f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, fHeadEmission);
        }
    }
    gltDrawSphere(8.0f, 15, 9);
    // Restore body emission for remaining parts
    if (nShadow == 0)
    {
        if (sunlightMode)
        {
            glMaterialfv(GL_FRONT, GL_EMISSION, fNoLight);
        }
        else
        {
            GLfloat fPedEmission[4] = {ped.colorTorso[0] * 0.45f, ped.colorTorso[1] * 0.45f, ped.colorTorso[2] * 0.45f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, fPedEmission);
        }
    }
    glPopMatrix();

    // Left Arm
    glPushMatrix();
    glTranslatef(-15.0f, 15.0f, 0.0f);
    glRotatef(upperArmRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[HOLO_TEXTURE]);
        glColor3fv(ped.colorTorso);
    }
    DrawBox(6.0f, 20.0f, 6.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(lowerArmRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawBox(5.0f, 20.0f, 5.0f); // Lower
    glPopMatrix();

    // Right Arm
    glPushMatrix();
    glTranslatef(15.0f, 15.0f, 0.0f);
    glRotatef(upperArmRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[HOLO_TEXTURE]);
        glColor3fv(ped.colorTorso);
    }
    DrawBox(6.0f, 20.0f, 6.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(lowerArmRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawBox(5.0f, 20.0f, 5.0f); // Lower
    glPopMatrix();

    // Left Leg
    glPushMatrix();
    glTranslatef(-7.0f, -20.0f, 0.0f);
    glRotatef(upperLegRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[HOLO_TEXTURE]);
        glColor3fv(ped.colorLegs);
    }
    DrawBox(8.0f, 20.0f, 8.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(lowerLegRotL, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawBox(7.0f, 20.0f, 7.0f); // Lower
    glPopMatrix();

    // Right Leg
    glPushMatrix();
    glTranslatef(7.0f, -20.0f, 0.0f);
    glRotatef(upperLegRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[HOLO_TEXTURE]);
        glColor3fv(ped.colorLegs);
    }
    DrawBox(8.0f, 20.0f, 8.0f); // Upper

    glTranslatef(0.0f, -10.0f, 0.0f);
    glRotatef(lowerLegRotR, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -10.0f, 0.0f);
    DrawBox(7.0f, 20.0f, 7.0f); // Lower
    glPopMatrix();

    glPopMatrix(); // Torso

    // Reset emission to zero
    if (nShadow == 0)
    {
        GLfloat fNoEmission[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, fNoEmission);
    }
}

// Draw a flat circular grass disk for the center park (radius < 3.5f)
void DrawCenterParkGrass(void)
{
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, textureObjects[GRASS_TEXTURE]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float r = 3.5f;
    int segments = 64;
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.5f, 0.5f);
    // Draw at y = -0.398f to match road height and reduce Z-fighting, and use CCW winding (negative angle step) to avoid backface culling
    glVertex3f(0.0f, -0.398f, 0.0f); // Center

    for (int i = 0; i <= segments; i++)
    {
        float angle = -2.0f * M_PI * i / segments; // Negative angle step for counter-clockwise (CCW) winding order
        float c = cos(angle);
        float s = sin(angle);
        glTexCoord2f(0.5f + c * 2.0f, 0.5f + s * 2.0f);
        glVertex3f(r * c, -0.398f, r * s);
    }
    glEnd();
    glPopMatrix();
}

// Draw random inhabitants, spinning humanoid robot, orbiting orange sphere, and flying AI drone
void DrawInhabitants(GLint nShadow, bool isLight1 = false)
{
    static GLfloat yRot = 0.0f; // Rotation angle for central robot
    GLint i;

    if (nShadow == 0)
    {
        if (timer_flag)
        {
            yRot += 0.5f;
            droneTime += 0.03f;
        }
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        if (sunlightMode)
            glColor4f(0.00f, 0.00f, 0.00f, 0.30f); // Soft transparent shadow in daylight (ambient bounce)
        else
            glColor4f(0.00f, 0.00f, 0.00f, 0.50f); // Soft transparent shadow at night
    }

    // Draw the randomly located pedestrians walking on the road
    if (nShadow == 0)
        glMaterialfv(GL_FRONT, GL_SPECULAR, fBrightLight);
    for (i = 0; i < NUM_SPHERES; i++)
    {
        glPushMatrix();
        pedestrians[i].frame.ApplyActorTransform();
        float pedScale = pedestrians[i].scale;
        // Position them precisely so they stand on the road pavement (y = -0.398f)
        float Ty = -0.398f + 60.0f * pedScale;
        glTranslatef(0.0f, Ty, 0.0f);
        glScalef(pedScale, pedScale, pedScale);
        DrawHierarchicalPedestrian(pedestrians[i], nShadow);
        glPopMatrix();
    }

    // Draw Orbiting surveillance drone B around spinning Robot A (sine-wave altitude oscillation)
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.0f); // center around central park/origin
    static float orbitAngle = 0.0f;
    if (nShadow == 0 && timer_flag)
        orbitAngle += 0.02f;
    glRotatef(orbitAngle * 57.2958f, 0.0f, 1.0f, 0.0f);
    float orbY = 0.8f + sin(orbitAngle * 4.0f) * 0.3f;
    glTranslatef(1.2f, orbY, 0.0f);
    if (nShadow == 0)
    {
        glBindTexture(GL_TEXTURE_2D, textureObjects[NEON_TEXTURE]);
        if (sunlightMode)
            glColor3f(0.5f, 0.5f, 0.5f); // Unlit orange in day
        else
            glColor3f(1.0f, 0.5f, 0.0f); // orange glow at night
    }
    gltDrawSphere(0.06f, 15, 9); // scaled to actual size (0.06f radius)
    glPopMatrix();

    // Draw central spinning giant robot (now placed in the center of the park at (0, 0, 0))
    if (!isLight1 || nShadow == 0)
    {
        if (nShadow == 0)
            glMaterialfv(GL_FRONT, GL_SPECULAR, fLowSpecular);
        glPushMatrix();
        glTranslatef(0.0f, 0.1f, 0.0f);
        glRotatef(yRot, 0.0f, 1.0f, 0.0f);
        DrawRobot(nShadow);
        glPopMatrix();
    }

    // Draw 8 loaded AI delivery drones flying in different orbits
    struct DroneConfig
    {
        float speedX, speedZ, speedY;
        float radiusX, radiusZ;
        float baseY;
        float r, g, b;
    };

    // Flying heights (baseY) set to 8.0f - 9.7f to fly safely above the skyscrapers and avoid building collisions
    DroneConfig droneConfigs[8] = {
        {0.5f, 0.3f, 1.5f, 6.0f, 6.0f, 8.0f, 0.5f, 0.8f, 1.0f},     // Cyan
        {-0.4f, 0.5f, 1.2f, 10.0f, 8.0f, 8.5f, 1.0f, 0.2f, 0.6f},   // Magenta
        {0.3f, -0.4f, 1.8f, 14.0f, 12.0f, 9.0f, 0.9f, 0.9f, 0.1f},  // Yellow
        {-0.6f, -0.5f, 1.0f, 17.0f, 15.0f, 9.5f, 0.2f, 1.0f, 0.4f}, // Green
        {0.4f, -0.3f, 1.4f, 8.0f, 8.0f, 8.2f, 1.0f, 0.5f, 0.0f},    // Orange
        {-0.5f, 0.4f, 1.6f, 12.0f, 10.0f, 8.7f, 0.8f, 0.2f, 1.0f},  // Purple
        {0.2f, 0.5f, 1.1f, 15.0f, 13.0f, 9.2f, 1.0f, 0.3f, 0.3f},   // Red
        {-0.3f, -0.6f, 1.3f, 18.0f, 16.0f, 9.7f, 0.1f, 0.9f, 0.9f}  // Bright Teal
    };

    for (int dIdx = 0; dIdx < 8; dIdx++)
    {
        float dX = sin(droneTime * droneConfigs[dIdx].speedX) * droneConfigs[dIdx].radiusX;
        float dZ = cos(droneTime * droneConfigs[dIdx].speedZ) * droneConfigs[dIdx].radiusZ;
        float dY = droneConfigs[dIdx].baseY + sin(droneTime * droneConfigs[dIdx].speedY) * 0.4f;

        if (nShadow == 0)
            glMaterialfv(GL_FRONT, GL_SPECULAR, fBrightLight);
        glPushMatrix();
        glTranslatef(dX, dY, dZ);
        glRotatef(droneTime * 25.0f * (dIdx + 1), 0.0f, 1.0f, 0.0f); // spin body
        glScalef(0.1f, 0.1f, 0.1f);                                  // Scale drone to actual size (width ~0.08 units)
        if (nShadow == 0)
        {
            glBindTexture(GL_TEXTURE_2D, textureObjects[METAL_TEXTURE]);
            glColor3f(droneConfigs[dIdx].r, droneConfigs[dIdx].g, droneConfigs[dIdx].b);

            if (sunlightMode)
            {
                glMaterialfv(GL_FRONT, GL_EMISSION, fNoLight);
            }
            else
            {
                // Add soft emission for drones so they are visible in dark areas
                GLfloat droneEmission[] = {droneConfigs[dIdx].r * 0.4f, droneConfigs[dIdx].g * 0.4f, droneConfigs[dIdx].b * 0.4f, 1.0f};
                glMaterialfv(GL_FRONT, GL_EMISSION, droneEmission);
            }
        }
        DrawOBJModel(modelDrone);
        if (nShadow == 0)
        {
            GLfloat fNoEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, fNoEmission);
        }
        glPopMatrix();
    }
}

// Dynamically configure and update the 4 hardware lights closest to the camera
void SetupDynamicStreetlights(void)
{
    if (sunlightMode)
    {
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHT3);
        glDisable(GL_LIGHT4);
        glDisable(GL_LIGHT5);
        return;
    }

    M3DVector3f vOrigin;
    frameCamera.GetOrigin(vOrigin);
    float camX = vOrigin[0];
    float camZ = vOrigin[2];

    struct LightInfo
    {
        float x, y, z;
        float r, g, b;
        float distSq;
    };

    const int NUM_LAMPS = 180;
    static LightInfo lamps[NUM_LAMPS];
    static bool lampsInit = false;

    if (!lampsInit)
    {
        // 1. Populate concentric lamps (96 lamps)
        float curbs[8] = {6.95f, 8.55f, 11.95f, 13.55f, 14.90f, 15.90f, 16.80f, 17.80f};
        float armDirs[8] = {1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f};
        float colors[8][3] = {
            {1.0f, 0.5f, 0.0f}, // Ring 1 inner (orange)
            {1.0f, 0.5f, 0.0f}, // Ring 1 outer
            {0.0f, 0.9f, 1.0f}, // Ring 2 inner (cyan)
            {0.0f, 0.9f, 1.0f}, // Ring 2 outer
            {0.2f, 1.0f, 0.3f}, // Ring 3 inner (green)
            {0.2f, 1.0f, 0.3f}, // Ring 3 outer
            {0.9f, 0.9f, 0.1f}, // Ring 4 inner (yellow)
            {0.9f, 0.9f, 0.1f}  // Ring 4 outer
        };

        int idx = 0;
        for (int c = 0; c < 8; c++)
        {
            float R = curbs[c];
            float arm = armDirs[c];
            float R_bulb = R + arm * 0.3f;
            for (int i = 0; i < 12; i++)
            {
                float angleDeg = i * 30.0f + 15.0f;
                float angleRad = angleDeg * (float)(M_PI / 180.0f);

                lamps[idx].x = R_bulb * cos(angleRad);
                lamps[idx].y = 0.81f; // Bulb height in world space
                lamps[idx].z = -R_bulb * sin(angleRad);

                lamps[idx].r = colors[c][0];
                lamps[idx].g = colors[c][1];
                lamps[idx].b = colors[c][2];
                idx++;
            }
        }

        // 2. Populate radial (vertical) lamps (84 lamps)
        float radialZPositions[7] = {4.375f, 6.125f, 9.225f, 11.225f, 14.225f, 16.35f, 18.40f};
        float radColors[7][3] = {
            {1.0f, 0.1f, 0.6f}, // Magenta/Pink
            {1.0f, 0.5f, 0.0f}, // Orange
            {0.0f, 0.9f, 1.0f}, // Cyan
            {0.1f, 0.3f, 1.0f}, // Electric Blue
            {0.7f, 0.0f, 1.0f}, // Purple
            {0.2f, 1.0f, 0.3f}, // Green
            {0.9f, 0.9f, 0.1f}  // Neon Yellow
        };
        for (int k = 0; k < 6; k++)
        {
            float roadAngle = k * 60.0f * (float)(M_PI / 180.0f);
            float cRoad = cos(roadAngle);
            float sRoad = sin(roadAngle);

            for (int r = 0; r < 7; r++)
            {
                float z = radialZPositions[r];

                // Left side of road (xLocal = -0.35f, zLocal = z)
                float bulbX_left = -0.35f;
                lamps[idx].x = bulbX_left * cRoad + z * sRoad;
                lamps[idx].y = 0.81f;
                lamps[idx].z = -bulbX_left * sRoad + z * cRoad;
                lamps[idx].r = radColors[r][0];
                lamps[idx].g = radColors[r][1];
                lamps[idx].b = radColors[r][2];
                idx++;

                // Right side of road (xLocal = 0.35f, zLocal = z)
                float bulbX_right = 0.35f;
                lamps[idx].x = bulbX_right * cRoad + z * sRoad;
                lamps[idx].y = 0.81f;
                lamps[idx].z = -bulbX_right * sRoad + z * cRoad;
                lamps[idx].r = radColors[r][0];
                lamps[idx].g = radColors[r][1];
                lamps[idx].b = radColors[r][2];
                idx++;
            }
        }
        lampsInit = true;
    }

    for (int i = 0; i < NUM_LAMPS; i++)
    {
        float dx = lamps[i].x - camX;
        float dz = lamps[i].z - camZ;
        lamps[i].distSq = dx * dx + dz * dz;
    }

    // Create a local copy of lamps for sorting to prevent in-place swapping from shuffling the static lamps array
    LightInfo sortedLamps[NUM_LAMPS];
    for (int i = 0; i < NUM_LAMPS; i++)
    {
        sortedLamps[i] = lamps[i];
    }

    for (int i = 0; i < 4; i++)
    {
        int minDistIdx = i;
        for (int j = i + 1; j < NUM_LAMPS; j++)
        {
            if (sortedLamps[j].distSq < sortedLamps[minDistIdx].distSq)
            {
                minDistIdx = j;
            }
        }
        if (minDistIdx != i)
        {
            LightInfo tmp = sortedLamps[i];
            sortedLamps[i] = sortedLamps[minDistIdx];
            sortedLamps[minDistIdx] = tmp;
        }
    }

    // Apply the 4 closest lights to GL_LIGHT2 through GL_LIGHT5
    for (int l = 0; l < 4; l++)
    {
        GLenum lightEnum = GL_LIGHT2 + l;

        GLfloat position[4] = {sortedLamps[l].x, sortedLamps[l].y, sortedLamps[l].z, 1.0f};
        glLightfv(lightEnum, GL_POSITION, position);

        GLfloat ambient[4] = {sortedLamps[l].r * 0.15f, sortedLamps[l].g * 0.15f, sortedLamps[l].b * 0.15f, 1.0f};
        GLfloat diffuse[4] = {sortedLamps[l].r * 0.85f, sortedLamps[l].g * 0.85f, sortedLamps[l].b * 0.85f, 1.0f};
        GLfloat specular[4] = {sortedLamps[l].r * 0.85f, sortedLamps[l].g * 0.85f, sortedLamps[l].b * 0.85f, 1.0f};

        glLightfv(lightEnum, GL_AMBIENT, ambient);
        glLightfv(lightEnum, GL_DIFFUSE, diffuse);
        glLightfv(lightEnum, GL_SPECULAR, specular);

        // Configure spotlight direction under the current view matrix V
        GLfloat spotDir[] = {0.0f, -1.0f, 0.0f};
        glLightfv(lightEnum, GL_SPOT_DIRECTION, spotDir);
        glLightf(lightEnum, GL_SPOT_CUTOFF, 45.0f);
        glLightf(lightEnum, GL_SPOT_EXPONENT, 2.0f); // Softer exponent for wider spill

        // Set attenuation parameters (reduced to make spotlight stronger on the road)
        glLightf(lightEnum, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(lightEnum, GL_LINEAR_ATTENUATION, 0.08f);
        glLightf(lightEnum, GL_QUADRATIC_ATTENUATION, 0.002f);

        // Overdrive light intensities (more brighter and stronger)
        GLfloat strongerDiffuse[] = {sortedLamps[l].r * 4.5f, sortedLamps[l].g * 4.5f, sortedLamps[l].b * 4.5f, 1.0f};
        GLfloat strongerSpecular[] = {sortedLamps[l].r * 4.5f, sortedLamps[l].g * 4.5f, sortedLamps[l].b * 4.5f, 1.0f};
        glLightfv(lightEnum, GL_DIFFUSE, strongerDiffuse);
        glLightfv(lightEnum, GL_SPECULAR, strongerSpecular);

        glEnable(lightEnum);
    }
}

// Helper function to render a 2D keybinding hint overlay at the bottom left
void RenderTextOverlay(void)
{
    glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Get current window viewport size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    // Set 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

#if ENABLE_CONTROLLER
    // Draw a semi-transparent dark background box for the overlay (larger for gamepad controls)
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2i(10, 10);
    glVertex2i(360, 10);
    glVertex2i(360, 240);
    glVertex2i(10, 240);
    glEnd();

    // Render keybinding hints
    glColor3f(1.0f, 1.0f, 1.0f); // White text

    const char *lines[] = {
        "=== KEYBOARD CONTROLS ===",
        "W / S      : Move Forward / Backward",
        "A / D      : Strafe Left / Right",
        "0 / Q      : Tilt Camera Up / Down",
        "Arrows LR  : Pan Camera Left / Right",
        "Arrows UD  : Fly Up / Down",
        "Keys [1]-[5]: Sunlight / Night Moon Modes",
        "G / O / P  : Toggle Glow / Outline / Pause",
        "",
        "=== GAMEPAD CONTROLS ===",
        "Left Stick : Move Forward/Backward & Strafe",
        "Right Stick: Fly Up/Down & Turn Left/Right",
        "Triggers   : Tilt Camera Up / Down",
        "Buttons    : A (Pause), Y (Day/Night)",
        "              : B (Outline), X (Glow)"};
    int numLines = sizeof(lines) / sizeof(lines[0]);
    int startY = 220;
#else
    // Draw a semi-transparent dark background box for the overlay (smaller for keyboard only)
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2i(10, 10);
    glVertex2i(360, 10);
    glVertex2i(360, 145);
    glVertex2i(10, 145);
    glEnd();

    // Render keybinding hints
    glColor3f(1.0f, 1.0f, 1.0f); // White text

    const char *lines[] = {
        "=== KEYBOARD CONTROLS ===",
        "W / S      : Move Forward / Backward",
        "A / D      : Strafe Left / Right",
        "0 / Q      : Tilt Camera Up / Down",
        "Arrows LR  : Pan Camera Left / Right",
        "Arrows UD  : Fly Up / Down",
        "Keys [1]-[5]: Sunlight / Night Moon Modes",
        "G / O / P  : Toggle Glow / Outline / Pause"};
    int numLines = sizeof(lines) / sizeof(lines[0]);
    int startY = 125;
#endif

    for (int i = 0; i < numLines; i++)
    {
        glRasterPos2i(20, startY - i * 15);
        const char *str = lines[i];
        while (*str)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *str);
            str++;
        }
    }

    // Restore matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();
}

// Called to draw scene
void RenderScene(void)
{
    if (sunlightMode)
        glClearColor(0.5f, 0.7f, 0.95f, 1.0f); // Bright blue sky for day
    else
        glClearColor(fLowLight[0], fLowLight[1], fLowLight[2], fLowLight[3]); // Dark sky for night

    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Update moon light position dynamically from keyboard state
    fLightPos0[0] = moonPositions[activeMoonPos][0];
    fLightPos0[1] = moonPositions[activeMoonPos][1];
    fLightPos0[2] = moonPositions[activeMoonPos][2];
    fLightPos0[3] = moonPositions[activeMoonPos][3];

    // Recalculate planar shadow matrix dynamically for Light 0
    M3DVector3f vPoints[3] = {{0.0f, -0.4f, 0.0f},
                              {10.0f, -0.4f, 0.0f},
                              {5.0f, -0.4f, -5.0f}};
    M3DVector4f pPlane;
    m3dGetPlaneEquation(pPlane, vPoints[0], vPoints[1], vPoints[2]);
    m3dMakePlanarShadowMatrix(mShadowMatrix0, pPlane, fLightPos0);

    glPushMatrix();
    frameCamera.ApplyCameraTransform();

    // Draw the Moon/Sun in the sky (self-illuminating sphere)
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glTranslatef(fLightPos0[0], fLightPos0[1], fLightPos0[2]);
    if (sunlightMode)
        glColor3f(1.0f, 0.9f, 0.3f); // Bright yellow sun
    else
        glColor3f(0.95f, 0.95f, 0.85f); // Soft glowing white moon
    gltDrawSphere(8.0f, 16, 16);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glPopMatrix();

    // Position lights and configure colors based on day/night mode
    if (sunlightMode)
    {
        // Bright global ambient light for daylight
        GLfloat fSunAmbient[] = {0.35f, 0.35f, 0.38f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fSunAmbient);

        // Bright white sunlight
        GLfloat fSunLight[] = {0.95f, 0.95f, 0.9f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, fNoLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, fSunLight);
        glLightfv(GL_LIGHT0, GL_SPECULAR, fSunLight);
        glLightfv(GL_LIGHT0, GL_POSITION, fLightPos0);

        // Disable central neon point light
        glDisable(GL_LIGHT1);
    }
    else
    {
        // Soft dark-blue global ambient to mimic city light scattering
        GLfloat fGlobalAmbient[] = {0.1f, 0.1f, 0.15f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fGlobalAmbient);

        // Soft moonlight
        GLfloat fDimMoon[] = {0.15f, 0.15f, 0.18f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, fNoLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, fDimMoon);
        glLightfv(GL_LIGHT0, GL_SPECULAR, fDimMoon);
        glLightfv(GL_LIGHT0, GL_POSITION, fLightPos0);

        // Enable central neon point light
        glEnable(GL_LIGHT1);
        glLightfv(GL_LIGHT1, GL_POSITION, fLightPos1);
    }

    // Position Ground Bounce Light pointing straight up (directional light in world space)
    // to simulate indirect light bounce from the city pavement
    GLfloat fGroundLightPos[] = {0.0f, 1.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT6, GL_POSITION, fGroundLightPos);

    // Position the 4 closest streetlights dynamically as spotlights
    SetupDynamicStreetlights();

    // Draw the ground (concrete), center park (grass), and paved roads (asphalt) with specular reflection
    // (shiny/wet look to reflect streetlights and moon light)
    GLfloat fRoadSpecular[] = {0.35f, 0.35f, 0.35f, 1.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, fRoadSpecular);
    glMateriali(GL_FRONT, GL_SHININESS, 64);
    glColor3f(1.0f, 1.0f, 1.0f);
    DrawGround();
    DrawCenterParkGrass();
    DrawRoads();

    // Draw shadows first using stencil buffer to avoid overlaps
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);

    // Clear stencil buffer before drawing shadows
    glClear(GL_STENCIL_BUFFER_BIT);

    // Draw shadow from Light 0
    glPushMatrix();
    glMultMatrixf(mShadowMatrix0);
    DrawInhabitants(1);
    DrawCity(1);
    glPopMatrix();

    // Draw shadow from Light 1 (skip skyscraper city shadows since Light 1 is a low point light at y=4.0
    // which projects inverted/infinite shadows that cover the entire road pavement in pitch black)
    glPushMatrix();
    glMultMatrixf(mShadowMatrix1);
    DrawInhabitants(1, true);
    glPopMatrix();

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    // Draw inhabitants (Spheres, Central Robot, Drones) normally
    DrawInhabitants(0);

    // Draw background buildings / cityblock normally
    DrawCity(0);

    glPopMatrix();

    // Draw HUD text overlay at the bottom left
    RenderTextOverlay();

    // Check for OpenGL errors and log them
    GLenum err;
    static int errorLogCount = 0;
    while (errorLogCount < 100 && (err = glGetError()) != GL_NO_ERROR)
    {
        std::ofstream debugLog("opengl_debug.log", std::ios::app);
        if (debugLog.is_open())
        {
            debugLog << "OpenGL Error in RenderScene: 0x" << std::hex << err << std::endl;
            debugLog.close();
        }
        errorLogCount++;
    }

    // Do the buffer Swap
    glutSwapBuffers();
}

// Key Down handlers for tracking keys pressed down
void NormalKeysDown(unsigned char key, int x, int y)
{
    keyState[key] = true;
    if (key == 'p' || key == 'P')
    {
        timer_flag = !timer_flag;
    }
    else if (key == '1')
    {
        activeMoonPos = 0;
        sunlightMode = false;
    }
    else if (key == '2')
    {
        activeMoonPos = 1;
        sunlightMode = false;
    }
    else if (key == '3')
    {
        activeMoonPos = 2;
        sunlightMode = false;
    }
    else if (key == '4')
    {
        activeMoonPos = 3;
        sunlightMode = false;
    }
    else if (key == '5')
    {
        sunlightMode = true;
    }
    else if (key == 'g' || key == 'G')
    {
        skyscraperGlow = !skyscraperGlow;
    }
    else if (key == 'o' || key == 'O')
    {
        skyscraperOutline = !skyscraperOutline;
    }
}

// Key Up handlers for tracking released keys
void NormalKeysUp(unsigned char key, int x, int y)
{
    keyState[key] = false;
}

void SpecialKeysDown(int key, int x, int y)
{
    if (key >= 0 && key < 256)
        specialKeyState[key] = true;
}

void SpecialKeysUp(int key, int x, int y)
{
    if (key >= 0 && key < 256)
        specialKeyState[key] = false;
}

// OOP input management for keyboard and XInput controllers
class InputManager
{
public:
    static void UpdateControls(GLFrame &camera, M3DVector3f &vOrigin, bool &bMoved, float fSpeed, float fRotSpeed)
    {
        // 1. Keyboard WASD / RF movement controls
        M3DVector3f vForward, vRight;
        camera.GetForwardVector(vForward);
        camera.GetXAxis(vRight);

        M3DVector3f vForwardProj = {vForward[0], 0.0f, vForward[2]};
        m3dNormalizeVector(vForwardProj);

        M3DVector3f vRightProj = {vRight[0], 0.0f, vRight[2]};
        m3dNormalizeVector(vRightProj);

        if (keyState['w'] || keyState['W'])
        {
            vOrigin[0] += vForwardProj[0] * fSpeed;
            vOrigin[2] += vForwardProj[2] * fSpeed;
            bMoved = true;
        }
        if (keyState['s'] || keyState['S'])
        {
            vOrigin[0] -= vForwardProj[0] * fSpeed;
            vOrigin[2] -= vForwardProj[2] * fSpeed;
            bMoved = true;
        }
        if (keyState['a'] || keyState['A'])
        {
            vOrigin[0] += vRightProj[0] * fSpeed;
            vOrigin[2] += vRightProj[2] * fSpeed;
            bMoved = true;
        }
        if (keyState['d'] || keyState['D'])
        {
            vOrigin[0] -= vRightProj[0] * fSpeed;
            vOrigin[2] -= vRightProj[2] * fSpeed;
            bMoved = true;
        }
        if (keyState['0'])
        {
            camera.RotateLocalX(-fRotSpeed);
        }
        if (keyState['q'] || keyState['Q'])
        {
            camera.RotateLocalX(fRotSpeed);
        }

        // 2. Keyboard Arrow keys for looking direction (Pan/Tilt)
        if (specialKeyState[GLUT_KEY_UP])
        {
            vOrigin[1] += fSpeed;
            bMoved = true;
        }
        if (specialKeyState[GLUT_KEY_DOWN])
        {
            vOrigin[1] -= fSpeed;
            bMoved = true;
        }
        if (specialKeyState[GLUT_KEY_LEFT])
        {
            camera.RotateWorld(fRotSpeed, 0.0f, 1.0f, 0.0f);
        }
        if (specialKeyState[GLUT_KEY_RIGHT])
        {
            camera.RotateWorld(-fRotSpeed, 0.0f, 1.0f, 0.0f);
        }

        // 3. XInput Gamepad controls (Mode 2 flight)
#if ENABLE_CONTROLLER
#ifdef _WIN32
        XINPUT_STATE xState;
        ZeroMemory(&xState, sizeof(XINPUT_STATE));
        if (XInputGetState(0, &xState) == ERROR_SUCCESS)
        {
            const int deadzone = 8000;

            // Left Stick Y (Pitch: Forward/Backward)
            short sThumbLY = xState.Gamepad.sThumbLY;
            if (abs(sThumbLY) > deadzone)
            {
                float lyNorm = (float)sThumbLY / 32767.0f;
                vOrigin[0] += vForwardProj[0] * lyNorm * fSpeed;
                vOrigin[2] += vForwardProj[2] * lyNorm * fSpeed;
                bMoved = true;
            }

            // Left Stick X (Roll: Strafe Left/Right)
            short sThumbLX = xState.Gamepad.sThumbLX;
            if (abs(sThumbLX) > deadzone)
            {
                float lxNorm = (float)sThumbLX / 32767.0f;
                vOrigin[0] -= vRightProj[0] * lxNorm * fSpeed;
                vOrigin[2] -= vRightProj[2] * lxNorm * fSpeed;
                bMoved = true;
            }

            // Right Stick Y (Throttle: Altitude Up/Down)
            short sThumbRY = xState.Gamepad.sThumbRY;
            if (abs(sThumbRY) > deadzone)
            {
                float ryNorm = (float)sThumbRY / 32767.0f;
                vOrigin[1] += ryNorm * fSpeed;
                bMoved = true;
            }

            // Right Stick X (Yaw: Turn Left/Right)
            short sThumbRX = xState.Gamepad.sThumbRX;
            if (abs(sThumbRX) > deadzone)
            {
                float rxNorm = (float)sThumbRX / 32767.0f;
                camera.RotateWorld(-rxNorm * fRotSpeed * 1.5f, 0.0f, 1.0f, 0.0f);
                bMoved = true;
            }

            // Triggers (Camera Tilt)
            BYTE lt = xState.Gamepad.bLeftTrigger;
            BYTE rt = xState.Gamepad.bRightTrigger;
            if (lt > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            {
                camera.RotateLocalX((float)lt / 255.0f * fRotSpeed);
                bMoved = true;
            }
            if (rt > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
            {
                camera.RotateLocalX(-(float)rt / 255.0f * fRotSpeed);
                bMoved = true;
            }

            // Buttons
            WORD wButtons = xState.Gamepad.wButtons;
            static WORD wLastButtons = 0;
            if ((wButtons & XINPUT_GAMEPAD_A) && !(wLastButtons & XINPUT_GAMEPAD_A))
            {
                timer_flag = !timer_flag;
            }
            if ((wButtons & XINPUT_GAMEPAD_Y) && !(wLastButtons & XINPUT_GAMEPAD_Y))
            {
                sunlightMode = !sunlightMode;
            }
            if ((wButtons & XINPUT_GAMEPAD_X) && !(wLastButtons & XINPUT_GAMEPAD_X))
            {
                skyscraperGlow = !skyscraperGlow;
            }
            if ((wButtons & XINPUT_GAMEPAD_B) && !(wLastButtons & XINPUT_GAMEPAD_B))
            {
                skyscraperOutline = !skyscraperOutline;
            }

            wLastButtons = wButtons;
        }
#endif
#endif
    }
};

// Main update timer, handling camera controls and rendering updates at ~60 FPS
void TimerFunction(int value)
{
    M3DVector3f vOrigin;
    frameCamera.GetOrigin(vOrigin);

    float fSpeed = 0.08f;
    float fRotSpeed = 0.02f;
    bool bMoved = false;

    // Delegate control updating to our unified InputManager
    InputManager::UpdateControls(frameCamera, vOrigin, bMoved, fSpeed, fRotSpeed);

    // Clamp camera height to keep it above ground level and within safe rendering bounds
    if (vOrigin[1] < 0.0f)
    {
        vOrigin[1] = 0.0f;
        bMoved = true;
    }
    else if (vOrigin[1] > 120.0f)
    {
        vOrigin[1] = 120.0f;
        bMoved = true;
    }

    if (bMoved)
        frameCamera.SetOrigin(vOrigin);

    // Move the 30 pedestrians along the circular roads and animate their walking limbs
    if (timer_flag)
    {
        for (int i = 0; i < NUM_SPHERES; i++)
        {
            M3DVector3f vOrigin;
            pedestrians[i].frame.GetOrigin(vOrigin);
            float radius = sqrt(vOrigin[0] * vOrigin[0] + vOrigin[2] * vOrigin[2]);
            if (radius > 1.0f)
            {
                float angle = atan2(vOrigin[2], vOrigin[0]);

                // Move along the circle using the pedestrian's specific walk speed
                float speed = pedestrians[i].walkSpeed;
                float dirSpeed = (radius < 10.0f) ? speed : -speed;
                angle += dirSpeed;

                vOrigin[0] = radius * cos(angle);
                vOrigin[2] = radius * sin(angle);
                pedestrians[i].frame.SetOrigin(vOrigin);

                // Align forward vector to walking direction
                float fwdX = -sin(angle) * (dirSpeed > 0 ? 1.0f : -1.0f);
                float fwdZ = cos(angle) * (dirSpeed > 0 ? 1.0f : -1.0f);
                pedestrians[i].frame.SetForwardVector(fwdX, 0.0f, fwdZ);

                // Animate leg/arm swinging
                pedestrians[i].walkPhase += speed * 30.0f;
            }
        }
    }

    // Force frame redraw
    glutPostRedisplay();
    glutTimerFunc(16, TimerFunction, 1);
}

void ChangeSize(int w, int h)
{
    GLfloat fAspect;

    if (h == 0)
        h = 1;

    glViewport(0, 0, w, h);

    fAspect = (GLfloat)w / (GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Increased far clipping plane to 600.0f to prevent ground and moon clipping, and set near clipping plane to 0.3f to prevent Z-fighting/flickering
    gluPerspective(35.0f, fAspect, 0.3f, 600.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Palmanova Star City");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    // Register keyboards callbacks for KeyDown and KeyUp states
    glutKeyboardFunc(NormalKeysDown);
    glutKeyboardUpFunc(NormalKeysUp);
    glutSpecialFunc(SpecialKeysDown);
    glutSpecialUpFunc(SpecialKeysUp);

    SetupRC();
    glutTimerFunc(16, TimerFunction, 1);

    glutMainLoop();

    ShutdownRC();

    return 0;
}
