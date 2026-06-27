#include "gltools.h"
#include <stdio.h>
#include <stdlib.h>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Load a TGA file using a clean, self-contained uncompressed TGA parser
GLbyte *gltLoadTGA(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat)
{
    FILE *pFile = fopen(szFileName, "rb");
    if (pFile == NULL)
        {
        printf("Error: Cannot open texture file %s\n", szFileName);
        return NULL;
        }

    // Read TGA header (18 bytes)
    unsigned char header[18];
    if (fread(header, 1, 18, pFile) != 18)
        {
        printf("Error: Failed to read TGA header from %s\n", szFileName);
        fclose(pFile);
        return NULL;
        }

    // Get width, height, and pixel depth
    int width = header[12] + (header[13] << 8);
    int height = header[14] + (header[15] << 8);
    int depth = header[16];

    *iWidth = width;
    *iHeight = height;

    if (depth == 24)
        {
        *iComponents = 3;
        *eFormat = GL_RGB;
        }
    else if (depth == 32)
        {
        *iComponents = 4;
        *eFormat = GL_RGBA;
        }
    else
        {
        printf("Error: Unsupported TGA depth %d in %s\n", depth, szFileName);
        fclose(pFile);
        return NULL;
        }

    // Allocate memory for image data
    int imgSize = width * height * (*iComponents);
    GLbyte *pBits = (GLbyte *)malloc(imgSize);
    if (pBits == NULL)
        {
        printf("Error: Memory allocation failed for %s\n", szFileName);
        fclose(pFile);
        return NULL;
        }

    // Read pixel data
    if (fread(pBits, 1, imgSize, pFile) != (size_t)imgSize)
        {
        printf("Error: Failed to read TGA pixel data from %s\n", szFileName);
        free(pBits);
        fclose(pFile);
        return NULL;
        }
    fclose(pFile);

    // Convert TGA's BGR(A) format to OpenGL's RGB(A) format by swapping B and R channels
    for (int i = 0; i < imgSize; i += *iComponents)
        {
        GLbyte temp = pBits[i];
        pBits[i] = pBits[i + 2];
        pBits[i + 2] = temp;
        }

    printf("Successfully loaded texture %s (%dx%d, %d channels)\n", szFileName, width, height, *iComponents);
    return pBits;
}

// Draw a sphere using GLU quadrics
void gltDrawSphere(GLfloat fRadius, GLint iSlices, GLint iStacks)
{
    GLUquadricObj *pObj = gluNewQuadric();
    gluQuadricNormals(pObj, GLU_SMOOTH);
    gluQuadricTexture(pObj, GL_TRUE);
    gluSphere(pObj, fRadius, iSlices, iStacks);
    gluDeleteQuadric(pObj);
}

// Draw a torus parametrically with texture coordinates and normals
void gltDrawTorus(GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor)
{
    double majorStep = 2.0 * M_PI / numMajor;
    double minorStep = 2.0 * M_PI / numMinor;

    for (int i = 0; i < numMajor; ++i)
        {
        double a0 = i * majorStep;
        double a1 = (i + 1) * majorStep;
        double x0 = cos(a0);
        double y0 = sin(a0);
        double x1 = cos(a1);
        double y1 = sin(a1);

        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= numMinor; ++j)
            {
            double b = j * minorStep;
            double c = cos(b);
            double r = minorRadius * c + majorRadius;
            double z = minorRadius * sin(b);

            // Tex coords, Normals, and Vertices for side 0
            glTexCoord2f((float)i / numMajor, (float)j / numMinor);
            glNormal3f((float)(x0 * c), (float)(y0 * c), (float)sin(b));
            glVertex3f((float)(x0 * r), (float)(y0 * r), (float)z);

            // Tex coords, Normals, and Vertices for side 1
            glTexCoord2f((float)(i + 1) / numMajor, (float)j / numMinor);
            glNormal3f((float)(x1 * c), (float)(y1 * c), (float)sin(b));
            glVertex3f((float)(x1 * r), (float)(y1 * r), (float)z);
            }
        glEnd();
        }
}
