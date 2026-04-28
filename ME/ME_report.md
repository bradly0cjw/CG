# Computer Graphics Midterm Project: 3D OBJ Parser & Transformer

## 1. Overview
This application is a 3D model viewer designed to load and manipulate `.obj` files. It features a high-precision **Vertex-Based Frustum-Fit** system that ensures every model is automatically centered and scaled to occupy 80% of the screen height, regardless of its original coordinate system or geometry complexity. The project supports comprehensive object transformations, an orbiting camera with adjustable focus, and multiple rendering modes.

---

## 2. Loaded Objects (Screenshots)
*Note: Please replace the placeholders below with actual screenshots of the models fitting 80% of the screen.*

### 2.1 Teapot
![Teapot Screenshot](./teapot.png)
*Description: The Utah Teapot rendered in face mode with the random color toggle enabled.*

### 2.2 Gourd
![Gourd Screenshot](./gourd.png)
*Description: The Gourd model rendered using point mode.*

### 2.3 Octahedron
![Octahedron Screenshot](./octahedron.png)
*Description: A primitive octahedron rendered using line mode.*

### 2.4 Teddy
![Teddy Screenshot](./teddy.png)
*Description: The Teddy bear rendered using line mode.*

---

## 3. Control Guide

### 3.1 Object Transformation (SRT)
- **Translation:**
  - `q` / `Q`: Move along **X-axis**
  - `w` / `W`: Move along **Y-axis**
  - `e` / `E`: Move along **Z-axis**
- **Rotation:**
  - `a` / `A`: Rotate around **X-axis**
  - `s` / `S`: Rotate around **Y-axis**
  - `d` / `D`: Rotate around **Z-axis**
- **Scaling:**
  - `z` / `Z`: Scale **X-axis**
  - `x` / `X`: Scale **Y-axis**
  - `c` / `C`: Scale **Z-axis**

### 3.2 Camera Controls
- **Orbit (Rotation):**
  - `u` / `U`: Adjust **Azimuth** (Theta)
  - `i` / `I`: Adjust **Elevation** (Phi)
- **Zoom:**
  - `o` / `O`: Move camera closer or further from the target (Radius/Distance).
- **Target (Focus Point):**
  - `j` / `J`: Move target along **X-axis**
  - `k` / `K`: Move target along **Y-axis**
  - `l` / `L`: Move target along **Z-axis**

### 3.3 Mouse & Misc
- **Left Click:** Click to define the **Arbitrary Axis** for rotation.
- **`m` / `M`:** Rotate around the selected **Arbitrary Axis**.
- **Right Click:** Context Menu (Render Mode, File Loading, Random Color).
- **`r` / `R`:** **Reset View** (Re-centers model and re-runs auto-scaling).
- **`Esc`:** Exit.

