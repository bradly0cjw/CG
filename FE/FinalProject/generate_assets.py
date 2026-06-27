import math
import os

def write_tga(filename, width, height, pixels):
    header = bytearray(18)
    header[2] = 2 # uncompressed true-color
    header[12] = width & 0xFF
    header[13] = (width >> 8) & 0xFF
    header[14] = height & 0xFF
    header[15] = (height >> 8) & 0xFF
    header[16] = 24 # 24 bpp BGR
    header[17] = 0x20 # top-to-bottom

    with open(filename, 'wb') as f:
        f.write(header)
        f.write(pixels)

def generate_textures():
    width = 512
    height = 512
    
    # 1. cyber_grass.tga - Textured Green Grass for Central Park
    pixels_grass = bytearray(width * height * 3)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            # Green organic noise pattern
            val = (x * 127 + y * 53) % 40
            g = min(255, 110 + val)
            r = min(255, 20 + val // 2)
            b = min(255, 10 + val // 3)
            # Add dark patches for contrast/realism
            if (x // 32 + y // 32) % 2 == 0:
                g = int(g * 0.75)
                r = int(r * 0.75)
                b = int(b * 0.75)
            pixels_grass[idx] = b
            pixels_grass[idx+1] = g
            pixels_grass[idx+2] = r
    write_tga("cyber_grass.tga", width, height, pixels_grass)
    print("Generated cyber_grass.tga")

    # 2. cyber_concrete.tga - Concrete paneling grid with rough speckles
    pixels_concrete = bytearray(width * height * 3)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            # Concrete base color + grain noise
            base_val = 175 + (x * 313 + y * 101) % 25
            
            # Dark concrete slab seams every 128 pixels
            if x % 128 < 3 or y % 128 < 3:
                b = g = r = 70
            else:
                b = g = r = base_val
                
            pixels_concrete[idx] = b
            pixels_concrete[idx+1] = g
            pixels_concrete[idx+2] = r
    write_tga("cyber_concrete.tga", width, height, pixels_concrete)
    print("Generated cyber_concrete.tga")

    # 3. cyber_asphalt.tga - Dark granular asphalt texture
    pixels_asphalt = bytearray(width * height * 3)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            # Dark base
            base_val = 30 + (x * 797 + y * 997) % 20
            # High frequency grain
            if (x * 31 + y * 47) % 5 == 0:
                base_val += 25
            elif (x * 31 + y * 47) % 7 == 0:
                base_val = max(10, base_val - 15)
                
            pixels_asphalt[idx] = base_val
            pixels_asphalt[idx+1] = base_val
            pixels_asphalt[idx+2] = base_val
    write_tga("cyber_asphalt.tga", width, height, pixels_asphalt)
    print("Generated cyber_asphalt.tga")

    # 4. cyber_metal.tga - Metallic Panel with neon traces
    pixels2 = bytearray(width * height * 3)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            # Base brushed metal grey
            base_val = 70 + (x * 17 + y * 23) % 15 # subtle noise
            b = g = r = base_val
            
            # Panel borders
            if x % 128 < 2 or y % 128 < 2:
                b = g = r = int(base_val * 0.4)
            elif x % 128 > 126 or y % 128 > 126:
                b = g = r = min(255, int(base_val * 1.5))
                
            # Neon circuit traces
            if (x >= 40 and x <= 44 and y >= 50 and y <= 350) or (x >= 40 and x <= 250 and y >= 346 and y <= 350):
                r, g, b = 0, 220, 255 # Neon cyan
            elif (x >= 300 and x <= 450 and y >= 200 and y <= 204) or (x >= 446 and x <= 450 and y >= 200 and y <= 400):
                r, g, b = 255, 120, 0 # Neon orange
                
            pixels2[idx] = b
            pixels2[idx+1] = g
            pixels2[idx+2] = r
    write_tga("cyber_metal.tga", width, height, pixels2)
    print("Generated cyber_metal.tga")

    # 5. neon_grid.tga - Intense Neon Blue Grid
    pixels3 = bytearray(width * height * 3)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            # Dark base
            b, g, r = 35, 10, 10
            dist_x = min(x % 32, 32 - (x % 32))
            dist_y = min(y % 32, 32 - (y % 32))
            dist = min(dist_x, dist_y)
            if dist <= 4:
                # Bright electric blue/pink grid lines
                factor = (4 - dist) / 4.0
                r = int(10 + factor * 245)
                g = int(10 + factor * 40)
                b = int(35 + factor * 220)
            pixels3[idx] = b
            pixels3[idx+1] = g
            pixels3[idx+2] = r
    write_tga("neon_grid.tga", width, height, pixels3)
    print("Generated neon_grid.tga")

    # 6. hologram.tga - Holographic vertical scanline gradient
    pixels4 = bytearray(width * height * 3)
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            # Blue gradient
            base_b = int(60 + 195 * (y / float(height)))
            base_g = int(20 + 80 * (y / float(height)))
            base_r = int(10 + 20 * (y / float(height)))
            
            # Horizontal scanlines
            if y % 8 < 2:
                b = int(base_b * 0.3)
                g = int(base_g * 0.3)
                r = int(base_r * 0.3)
            else:
                b, g, r = base_b, base_g, base_r
                
            pixels4[idx] = b
            pixels4[idx+1] = g
            pixels4[idx+2] = r
    write_tga("hologram.tga", width, height, pixels4)
    print("Generated hologram.tga")

    # 7. cyber_spotlight.tga - Radial gradient for spotlight pool
    pixels_spot = bytearray(width * height * 3)
    cx, cy = 256, 256
    max_dist = 256.0
    for y in range(height):
        for x in range(width):
            idx = (y * width + x) * 3
            dx = x - cx
            dy = y - cy
            dist = math.sqrt(dx*dx + dy*dy)
            
            if dist < max_dist:
                # Radial cosine-like falloff
                factor = (1.0 - dist / max_dist)
                val = int(255 * (factor ** 1.8))
            else:
                val = 0
                
            pixels_spot[idx] = val
            pixels_spot[idx+1] = val
            pixels_spot[idx+2] = val
    write_tga("cyber_spotlight.tga", width, height, pixels_spot)
    print("Generated cyber_spotlight.tga")

def generate_robot_obj():
    # Vertices and faces accumulation
    vertices = []
    faces = []

    def add_box(cx, cy, cz, sx, sy, sz):
        start_v = len(vertices) + 1
        # Add 8 vertices
        for dx in [-1, 1]:
            for dy in [-1, 1]:
                for dz in [-1, 1]:
                    vertices.append((cx + dx * sx / 2.0, cy + dy * sy / 2.0, cz + dz * sz / 2.0))
        # Add 6 quad faces (CCW)
        box_faces = [
            (0, 1, 3, 2), # Left (x = -w/2)
            (5, 4, 6, 7), # Right (x = +w/2)
            (0, 4, 5, 1), # Bottom (y = -h/2)
            (2, 3, 7, 6), # Top (y = +h/2)
            (4, 0, 2, 6), # Back (z = -d/2)
            (1, 5, 7, 3)  # Front (z = +d/2)
        ]
        for f in box_faces:
            faces.append([start_v + idx for idx in f])

    def add_sphere(cx, cy, cz, R, lats=10, lons=10):
        start_v = len(vertices) + 1
        # Add vertices
        for i in range(lats + 1):
            lat = math.pi * i / lats
            sin_lat = math.sin(lat)
            cos_lat = math.cos(lat)
            for j in range(lons):
                lon = 2.0 * math.pi * j / lons
                sin_lon = math.sin(lon)
                cos_lon = math.cos(lon)
                x = cx + R * cos_lon * sin_lat
                y = cy + R * cos_lat
                z = cz + R * sin_lon * sin_lat
                vertices.append((x, y, z))
        # Add faces
        for i in range(lats):
            for j in range(lons):
                j_next = (j + 1) % lons
                v00 = start_v + i * lons + j
                v10 = start_v + (i + 1) * lons + j
                v11 = start_v + (i + 1) * lons + j_next
                v01 = start_v + i * lons + j_next
                # Outward facing normal CCW
                faces.append((v00, v01, v11, v10))

    # Construct the Lab 13 robot geometry
    # Total model offset: Y += 60.0 so that the lowest foot bottom is at Y = 0
    # Torso: width=20, height=40, depth=10. Center: (0, 60, 0)
    add_box(0.0, 60.0, 0.0, 20.0, 40.0, 10.0)
    
    # Head: Sphere radius=8. Center: (0, 85, 0) (torso top is 60 + 20 = 80, head center at 85)
    add_sphere(0.0, 85.0, 0.0, 8.0)
    
    # Left Arm Upper: size=(6, 20, 6), Center: (-15, 65, 0)
    add_box(-15.0, 65.0, 0.0, 6.0, 20.0, 6.0)
    # Left Arm Lower: size=(5, 20, 5), Center: (-15, 45, 0)
    add_box(-15.0, 45.0, 0.0, 5.0, 20.0, 5.0)
    
    # Right Arm Upper: size=(6, 20, 6), Center: (15, 65, 0)
    add_box(15.0, 65.0, 0.0, 6.0, 20.0, 6.0)
    # Right Arm Lower: size=(5, 20, 5), Center: (15, 45, 0)
    add_box(15.0, 45.0, 0.0, 5.0, 20.0, 5.0)
    
    # Left Leg Upper: size=(8, 20, 8), Center: (-7, 30, 0) (torso bottom is 60 - 20 = 40)
    add_box(-7.0, 30.0, 0.0, 8.0, 20.0, 8.0)
    # Left Leg Lower: size=(7, 20, 7), Center: (-7, 10, 0) (lowest point is 10 - 10 = 0)
    add_box(-7.0, 10.0, 0.0, 7.0, 20.0, 7.0)
    
    # Right Leg Upper: size=(8, 20, 8), Center: (7, 30, 0)
    add_box(7.0, 30.0, 0.0, 8.0, 20.0, 8.0)
    # Right Leg Lower: size=(7, 20, 7), Center: (7, 10, 0)
    add_box(7.0, 10.0, 0.0, 7.0, 20.0, 7.0)

    # Write OBJ file
    with open("pedestrian.obj", "w") as f:
        f.write("# Robot model exported from Lab 13 shadow.cpp\n")
        f.write("# Scale down by 0.007 to 0.01 in OpenGL\n\n")
        for v in vertices:
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
        f.write("\n")
        for face in faces:
            f.write(f"f {face[0]} {face[1]} {face[2]} {face[3]}\n")
            
    print("Generated pedestrian.obj")

if __name__ == "__main__":
    generate_textures()
    # generate_robot_obj()
