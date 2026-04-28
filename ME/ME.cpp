#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Structure to hold 3D coordinates
struct Vertex {
    float x, y, z;
};

// Structure to hold face indices
struct Face {
    std::vector<int> vIndices;
};

// Global Variables
std::vector<Vertex> vertices;
std::vector<Face> faces;
int renderMode = 2; // 0: Point, 1: Line, 2: Face
bool useRandomColor = false;
int winW = 800, winH = 600;
float autoScaleFactor = 1.0f;

// Object Transformation
float objTX = 0.0f, objTY = 0.0f, objTZ = 0.0f;
float objSX = 1.0f, objSY = 1.0f, objSZ = 1.0f;
float objRX = 0.0f, objRY = 0.0f, objRZ = 0.0f;
float objRotArb = 0.0f;
float arbX = 0.0f, arbY = 1.0f, arbZ = 0.0f; // Default arb axis
bool hasClickedArb = false;

// Camera Control (Orbit model)
float camRadius = 3.0f;
float camTheta = M_PI / 4.0f; // 45 degrees Azimuth
float camPhi = M_PI / 4.0f;   // 45 degrees Elevation
float lookX = 0.0f, lookY = 0.0f, lookZ = 0.0f;

// Home position (after auto-scale)
float homeRadius = 3.0f;
float homeLookX = 0.0f, homeLookY = 0.0f, homeLookZ = 0.0f;

// Helper to normalize angles for display (0-360)
float normDeg(float deg) {
    float res = fmodf(deg, 360.0f);
    if (res < 0) res += 360.0f;
    return res;
}

// Helper to normalize radians for display (0-360)
float normRad(float rad) {
    return normDeg(rad * 180.0f / (float)M_PI);
}

// Matrix Multiplication Helper
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

// Custom SRT transformation (Order: RA * T * S * RX * RY * RZ)
void applyCustomTransformations() {
    float T[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, objTX, objTY, objTZ, 1 };
    float S[16] = { objSX, 0, 0, 0, 0, objSY, 0, 0, 0, 0, objSZ, 0, 0, 0, 0, 1 };

    float radX = (float)(objRX * M_PI / 180.0);
    float cX = cosf(radX), sX = sinf(radX);
    float RX[16] = { 1, 0, 0, 0, 0, cX, sX, 0, 0, -sX, cX, 0, 0, 0, 0, 1 };

    float radY = (float)(objRY * M_PI / 180.0);
    float cY = cosf(radY), sY = sinf(radY);
    float RY[16] = { cY, 0, -sY, 0, 0, 1, 0, 0, sY, 0, cY, 0, 0, 0, 0, 1 };

    float radZ = (float)(objRZ * M_PI / 180.0);
    float cZ = cosf(radZ), sZ = sinf(radZ);
    float RZ[16] = { cZ, sZ, 0, 0, -sZ, cZ, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

    float RA[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    float mag = sqrtf(arbX * arbX + arbY * arbY + arbZ * arbZ);
    if (mag > 0.0001f) {
        float x = arbX / mag, y = arbY / mag, z = arbZ / mag;
        float radA = (float)(objRotArb * M_PI / 180.0);
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

void resetView() {
    objTX = objTY = objTZ = 0.0f;
    objSX = objSY = objSZ = 1.0f;
    objRX = objRY = objRZ = 0.0f;
    objRotArb = 0.0f;
    camRadius = homeRadius;
    lookX = homeLookX;
    lookY = homeLookY;
    lookZ = homeLookZ;
    camTheta = M_PI / 4.0f;
    camPhi = M_PI / 4.0f;
    hasClickedArb = false;
    arbX = 0.0f; arbY = 1.0f; arbZ = 0.0f;
}

// Bounding Box Adjustment to fit ~80% of screen by adjusting CAMERA
void normalizeObject() {
    if (vertices.empty()) return;

    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& v : vertices) {
        if (v.x < minX) minX = v.x; 
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y; 
        if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z; 
        if (v.z > maxZ) maxZ = v.z;
    }

    // Calculate center of the bounding box
    homeLookX = lookX = (minX + maxX) / 2.0f;
    homeLookY = lookY = (minY + maxY) / 2.0f;
    homeLookZ = lookZ = (minZ + maxZ) / 2.0f;

    // Camera axes for the initial view (Theta=45, Phi=45)
    float ct = cosf(M_PI / 4.0f), st = sinf(M_PI / 4.0f);
    float cp = cosf(M_PI / 4.0f), sp = sinf(M_PI / 4.0f);

    // Forward vector (w) pointing from target to camera
    float wx = sp * ct, wy = cp, wz = sp * st;
    // Right vector (u) = Up(0,1,0) x w
    float ux = wz, uy = 0, uz = -wx;
    float uLen = sqrtf(ux * ux + uz * uz);
    if (uLen > 0.0001f) { ux /= uLen; uz /= uLen; }
    // Up vector (v) = w x u
    float vx = wy * uz - wz * uy, vy = wz * ux - wx * uz, vz = wx * uy - wy * ux;

    float tanHalfVfov = tanf(22.5f * (float)M_PI / 180.0f);
    float aspect = (winH > 0) ? (float)winW / winH : 1.333f;
    float fill = 0.8f;

    float maxD = 0.0f;
    // Check every vertex for a tight fit in the initial view
    for (const auto& v : vertices) {
        float ox = v.x - lookX;
        float oy = v.y - lookY;
        float oz = v.z - lookZ;

        // Project offset relative to center onto camera axes
        float xPrime = ox * ux + oy * uy + oz * uz;
        float yPrime = ox * vx + oy * vy + oz * vz;
        float zPrime = ox * wx + oy * wy + oz * wz;

        // Required distance to fit this vertex horizontally and vertically
        float dX = fabsf(xPrime) / (aspect * tanHalfVfov * fill) + zPrime;
        float dY = fabsf(yPrime) / (tanHalfVfov * fill) + zPrime;

        if (dX > maxD) maxD = dX;
        if (dY > maxD) maxD = dY;
    }

    homeRadius = camRadius = maxD;
    if (homeRadius < 0.1f) homeRadius = camRadius = 0.1f;

    std::cout << "Auto-Scale (Vertex-Fit): Center[" << lookX << "," << lookY << "," << lookZ 
              << "] CamDist[" << camRadius << "]" << std::endl;
}

// Function to parse the OBJ file
void loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return;
    }

    vertices.clear();
    faces.clear();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string prefix;
        if (!(ss >> prefix)) continue;

        if (prefix == "v") {
            Vertex v;
            if (ss >> v.x >> v.y >> v.z) {
                vertices.push_back(v);
            }
        }
        else if (prefix == "f") {
            Face f;
            std::string vertRef;
            while (ss >> vertRef) {
                try {
                    size_t firstSlash = vertRef.find('/');
                    int vIdx = std::stoi(vertRef.substr(0, firstSlash));
                    if (vIdx < 0) vIdx = (int)vertices.size() + vIdx + 1;
                    if (vIdx > 0 && vIdx <= (int)vertices.size()) {
                        f.vIndices.push_back(vIdx - 1);
                    }
                } catch (...) {}
            }
            if (!f.vIndices.empty()) faces.push_back(f);
        }
    }
    file.close();
    std::cout << "Loaded: " << filename << " (" << vertices.size() << " vertices, " << faces.size() << " faces)" << std::endl;
    normalizeObject();
    
    // Reset non-camera transformations
    objTX = objTY = objTZ = 0.0f;
    objSX = objSY = objSZ = 1.0f;
    objRX = objRY = objRZ = 0.0f;
    objRotArb = 0.0f;
    camTheta = M_PI / 4.0f;
    camPhi = M_PI / 4.0f;
    hasClickedArb = false;
    arbX = 0.0f; arbY = 1.0f; arbZ = 0.0f;
}

void renderText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

void drawAxis() {
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // X-axis ray
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(1000, 0, 0);
    // Y-axis ray
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 1000, 0);
    // Z-axis ray
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1000);
    glEnd();
    glLineWidth(1.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Calculate Camera Position from spherical coordinates
    float camX = lookX + camRadius * sinf(camPhi) * cosf(camTheta);
    float camY = lookY + camRadius * cosf(camPhi);
    float camZ = lookZ + camRadius * sinf(camPhi) * sinf(camTheta);

    // Draw 3D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)winW / winH, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camX, camY, camZ, lookX, lookY, lookZ, 0, 1, 0);
    drawAxis();

    glPushMatrix();
    applyCustomTransformations();

    if (renderMode == 0) {
        glPointSize(10.0f);
        glBegin(GL_POINTS);
    }

    for (const auto& face : faces) {
        if (renderMode == 1) glBegin(GL_LINE_LOOP);
        else if (renderMode == 2) glBegin(GL_POLYGON);

        if (!useRandomColor) glColor3f(1.0f, 1.0f, 1.0f);

        for (int idx : face.vIndices) {
            if (useRandomColor) {
                srand(idx);
                glColor3f((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
            }
            glVertex3f(vertices[idx].x, vertices[idx].y, vertices[idx].z);
        }
        if (renderMode != 0) glEnd();
    }
    if (renderMode == 0) glEnd();
    glPopMatrix();

    // Arb axis Ray (Starts from origin)
    if (hasClickedArb) {
        glBegin(GL_LINES);
        glColor3f(1, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(100 * arbX, 100 * arbY, 100 * arbZ);
        glEnd();
        glPointSize(12.0f);
        glBegin(GL_POINTS);
        glColor3f(1, 0, 0);
        glVertex3f(arbX, arbY, arbZ);
        glEnd();
    }

    // Overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    
    glColor3f(1, 1, 1);
    float startY = winH - 20;
    renderText(10, startY, "Obj Translate: q/Q (X), w/W (Y), e/E (Z) | Rotate: a/A, s/S, d/D | Scale: z/Z, x/X, c/C");
    renderText(10, startY - 15, "Cam Orbit: u/U (Azim), i/I (Elev), o/O (Dist) | Target: j/J, k/K, l/L");
    renderText(10, startY - 30, "Arb Axis Rotate: m/M | Reset: r/R | Mouse L: Set Arb Axis | Mouse R: Menu");

    char buf[256];
    glColor3f(0.0f, 1.0f, 1.0f);
    sprintf_s(buf, "Obj T: [%.2f, %.2f, %.2f] S: [%.2f, %.2f, %.2f] R: [%.1f, %.1f, %.1f]", 
        objTX, objTY, objTZ, objSX, objSY, objSZ, normDeg(objRX), normDeg(objRY), normDeg(objRZ));
    renderText(10, 40, buf);
    sprintf_s(buf, "Arb Axis: [%.2f, %.2f, %.2f] Angle: %.1f", 
        arbX, arbY, arbZ, normDeg(objRotArb));
    renderText(10, 25, buf);
    sprintf_s(buf, "Cam Orbit(%.1f deg, %.1f deg) Rad: %.2f | Target: [%.2f, %.2f, %.2f]", 
        normRad(camTheta), normRad(camPhi), camRadius, lookX, lookY, lookZ);
    renderText(10, 10, buf);

    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    float s = 0.1f, rs = 5.0f, ss = 0.1f, as = 0.05f;
    switch (key) {
        // Object Translation (qwe)
    case 'q': objTX -= s; break; case 'Q': objTX += s; break;
    case 'w': objTY -= s; break; case 'W': objTY += s; break;
    case 'e': objTZ -= s; break; case 'E': objTZ += s; break;

        // Object Rotation (asd)
    case 'a': objRX -= rs; break; case 'A': objRX += rs; break;
    case 's': objRY -= rs; break; case 'S': objRY += rs; break;
    case 'd': objRZ -= rs; break; case 'D': objRZ += rs; break;

        // Object Scale (zxc)
    case 'z': objSX -= ss; if (objSX < 0.01f) objSX = 0.01f; break; case 'Z': objSX += ss; break;
    case 'x': objSY -= ss; if (objSY < 0.01f) objSY = 0.01f; break; case 'X': objSY += ss; break;
    case 'c': objSZ -= ss; if (objSZ < 0.01f) objSZ = 0.01f; break; case 'C': objSZ += ss; break;

        // Camera Orbit (uio)
    case 'u': camTheta -= as; break; case 'U': camTheta += as; break;
    case 'i': camPhi -= as; if (camPhi < 0.01f) camPhi = 0.01f; break; 
    case 'I': camPhi += as; if (camPhi > M_PI - 0.01f) camPhi = M_PI - 0.01f; break;
    case 'o': camRadius -= s; if (camRadius < 0.1f) camRadius = 0.1f; break; 
    case 'O': camRadius += s; break;

        // Camera Target (jkl)
    case 'j': lookX -= s; break; case 'J': lookX += s; break;
    case 'k': lookY -= s; break; case 'K': lookY += s; break;
    case 'l': lookZ -= s; break; case 'L': lookZ += s; break;

        // Misc
    case 'm': objRotArb -= rs; break; case 'M': objRotArb += rs; break;
    case 'r': case 'R': resetView(); break;

    case 27: exit(0); break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        GLint vp[4]; GLdouble mv[16], pr[16], wX, wY, wZ;
        glGetIntegerv(GL_VIEWPORT, vp);
        glGetDoublev(GL_MODELVIEW_MATRIX, mv);
        glGetDoublev(GL_PROJECTION_MATRIX, pr);
        gluUnProject((GLdouble)x, (GLdouble)(vp[3] - y), 0.5, mv, pr, vp, &wX, &wY, &wZ);
        arbX = (float)wX; arbY = (float)wY; arbZ = (float)wZ;
        hasClickedArb = true;
        glutPostRedisplay();
    }
}

void menu(int id) {
    switch (id) {
    case 0: renderMode = 0; break;
    case 1: renderMode = 1; break;
    case 2: renderMode = 2; break;
    case 3: useRandomColor = !useRandomColor; break;
    case 10: loadOBJ(".\\obj\\gourd.obj"); break;
    case 11: loadOBJ(".\\obj\\octahedron.obj"); break;
    case 12: loadOBJ(".\\obj\\teapot.obj"); break;
    case 13: loadOBJ(".\\obj\\teddy.obj"); break;
    case 20: {
        std::string path;
        std::cout << "Enter OBJ path: ";
        std::cin >> path;
        loadOBJ(path);
        break;
    }
    case 50: resetView(); break;
    case 99: exit(0); break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0, 0, w, h);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("OBJ Parser - Midterm Project 2026");
    glEnable(GL_DEPTH_TEST);
    
    if (argc > 1) loadOBJ(argv[1]);
    else loadOBJ(".\\obj\\teapot.obj");

    int rm = glutCreateMenu(menu);
    glutAddMenuEntry("Point", 0); glutAddMenuEntry("Line", 1); glutAddMenuEntry("Face", 2);
    int fm = glutCreateMenu(menu);
    glutAddMenuEntry("Gourd", 10); glutAddMenuEntry("Octahedron", 11);
    glutAddMenuEntry("Teapot", 12); glutAddMenuEntry("Teddy", 13);
    glutAddMenuEntry("Open custom (Console)", 20);

    glutCreateMenu(menu);
    glutAddSubMenu("Render Mode", rm);
    glutAddMenuEntry("Toggle Random Color", 3);
    glutAddSubMenu("Files", fm);
    glutAddMenuEntry("Reset View", 50);
    glutAddMenuEntry("Exit", 99);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
