#include <cstdint>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "rcamera.h"
#include <vector>
#include <cmath>
#include <array>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>


std::vector<int> permSingle = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };
uint32_t mulberry32(uint32_t& state) {
    uint32_t z = (state += 0x6D2B79F5UL);
    z = (z ^ (z >> 15)) * (z | 1UL);
    z ^= z + (z ^ (z >> 7)) * (z | 61UL);
    return z ^ (z >> 14);
}
std::vector<int> seedPerm(uint32_t& seed){
	for(int i = 0; i<permSingle.size(); i++){
		const int temp = std::floor(mulberry32(seed) * (i));
		int current = permSingle[i];
		permSingle[i] = permSingle[temp];
		permSingle[temp] = current;
}
std::vector<int> perm = permSingle;
perm.insert(perm.end(), permSingle.begin(), permSingle.end());
return perm;
}
int hashValue(double x, double y, const std::vector<int>& perm) {
    int floorX = static_cast<int>(std::floor(x)) & 255;
    int floorY = static_cast<int>(std::floor(y)) & 255;
    return perm[perm[floorX] + floorY];
}
double myLerp(double value1, double value2, double interpolationFactor){
return ((1-interpolationFactor)*value1)+(interpolationFactor*value2);
}
double smootherStep(double x){
  return (x * x * x * (x * (x * 6.0 - 15.0) + 10.0));
}
const std::array<std::array<int, 2>, 8> gradients = {{
    { 1,  1}, {-1,  1}, { 1, -1}, {-1, -1},
    { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1}
}};
std::array<int, 2> getGradient(int x, int y, std::vector<int> perm){
  int hash = hashValue(x, y, perm);
  return gradients[hash % 8];
}
double perlinNoise(int squareSize, int x, int y, const std::vector<int>& perm) {
    double doubleX = static_cast<double>(x) / squareSize;
    double doubleY = static_cast<double>(y) / squareSize;

    int gridX = static_cast<int>(std::floor(doubleX));
    int gridY = static_cast<int>(std::floor(doubleY));
    
    double decamalX = doubleX - gridX;
    double decamalY = doubleY - gridY;

    std::array<int, 2> topLeft     = getGradient(gridX,     gridY,     perm);
    std::array<int, 2> bottomLeft  = getGradient(gridX,     gridY + 1, perm);
    std::array<int, 2> bottomRight = getGradient(gridX + 1, gridY + 1, perm);
    std::array<int, 2> topRight    = getGradient(gridX + 1, gridY,     perm);

    double distanceTopLeftX = decamalX;
    double distanceTopLeftY = decamalY;
    
    double distanceBottomLeftX = decamalX;
    double distanceBottomLeftY = decamalY - 1.0;
    
    double distanceBottomRightX = decamalX - 1.0;
    double distanceBottomRightY = decamalY - 1.0;
    
    double distanceTopRightX = decamalX - 1.0;
    double distanceTopRightY = decamalY;

    double dotProductTopLeft     = (distanceTopLeftX     * topLeft[0])     + (distanceTopLeftY     * topLeft[1]);
    double dotProductBottomLeft  = (distanceBottomLeftX  * bottomLeft[0])  + (distanceBottomLeftY  * bottomLeft[1]);
    double dotProductBottomRight = (distanceBottomRightX * bottomRight[0]) + (distanceBottomRightY * bottomRight[1]);
    double dotProductTopRight    = (distanceTopRightX    * topRight[0])    + (distanceTopRightY    * topRight[1]);

    double u = smootherStep(decamalX);
    double v = smootherStep(decamalY);

    double lerpTop    = myLerp(dotProductTopLeft,    dotProductTopRight,    u);
    double lerpBottom = myLerp(dotProductBottomLeft, dotProductBottomRight, u);
    double lerpFinal  = myLerp(lerpTop,              lerpBottom,            v);

    return (lerpFinal + 1.0) / 2.0;
}
int random(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}
uint32_t seed = random(0,100000);
std::vector<int> perm = seedPerm(seed);
const int CHUNK_SIZE = 500;
const int RENDER_DISTANCE = 3;

Vector3 roundToChunk(Vector3 position){
	return Vector3{std::floorf(position.x / CHUNK_SIZE), std::floorf(position.y / CHUNK_SIZE), 0};
}
const int HEIGHT_SCALE = 50;
const int ZOOM_AMOUNT = 10;
//MAKE SURE SPACING IS A DIVISOR OF CHUNK_SIZE
const int SPACING = 4;
const int SEGMENTS = std::floor(CHUNK_SIZE / SPACING);
struct chunk{
    int x;
    int y;
    bool operator==(const chunk& other) const {
        return x == other.x && y == other.y;
    }
};
struct ChunkHash {
    std::size_t operator()(const chunk& c) const {
        return std::hash<int>{}(c.x) ^ (std::hash<int>{}(c.y) << 1);
    }
};
std::unordered_map<chunk, Model, ChunkHash> chunks;

std::optional<Mesh> generateChunkAt(int xc, int yc){
    chunk temp;
    temp.x = xc;
    temp.y = yc;
    if (chunks.find(temp) != chunks.end()) return std::nullopt;
    int vertX = SEGMENTS + 1;
    int vertY = SEGMENTS + 1;
    Mesh mesh = {0};
    mesh.vertexCount = vertX * vertY;
    mesh.triangleCount = SEGMENTS * SEGMENTS * 2;
    mesh.vertices = (float*)calloc(mesh.vertexCount*3, sizeof(float));
    mesh.texcoords = (float*)calloc(mesh.vertexCount*2, sizeof(float));
    mesh.normals = (float*)calloc(mesh.vertexCount*3, sizeof(float));
    mesh.indices = (unsigned short*)malloc(mesh.triangleCount * 3 * sizeof(unsigned short));
    int vIndex = 0;
    int tIndex = 0;
	for(int x = 0; x<SEGMENTS+1; x++){
		for(int y = 0; y<SEGMENTS+1; y++){
            int worldX = xc * CHUNK_SIZE + x * SPACING;
            int worldY = yc * CHUNK_SIZE + y * SPACING;
			double height = perlinNoise(ZOOM_AMOUNT, worldX, worldY, perm) * 50;
            mesh.vertices[vIndex] = worldX;
            mesh.vertices[vIndex+1] = height;
            mesh.vertices[vIndex+2] = worldY;
            mesh.texcoords[tIndex] = (float)x/SEGMENTS;
            mesh.texcoords[tIndex+1] = (float)y/SEGMENTS;
            mesh.normals[vIndex] = 0.0f;
            mesh.normals[vIndex + 1] = 1.0f;
            mesh.normals[vIndex + 2] = 0.0f;
            vIndex+=3;
            tIndex+=2;
       }
    }
    int iIndex = 0;
	for(int x = 0; x<SEGMENTS; x++){
		for(int y = 0; y<SEGMENTS; y++){
            int topLeft = (y * vertX) + x;
            int topRight = topLeft +1;
            int bottomLeft = ((y + 1) * vertX) + x;
            int bottomRight = bottomLeft+1;

            mesh.indices[iIndex++] = topLeft;
            mesh.indices[iIndex++] = bottomLeft;
            mesh.indices[iIndex++] = topRight;
            mesh.indices[iIndex++] = topRight;
            mesh.indices[iIndex++] = bottomLeft;
            mesh.indices[iIndex++] = bottomRight;
        }
    }
    chunks[temp] = LoadModelFromMesh(mesh);
    free(mesh.vertices);
    free(mesh.texcoords);
    free(mesh.normals);
    free(mesh.indices);

    return mesh;
}
int main(){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 1000, "view");
    SetExitKey(KEY_NULL);
    Camera3D camera = {0};
    camera.position = Vector3 {10.0f, 10.0f, 10.0f};
    camera.target = Vector3 {0.0f, 0.0f, 0.0f};
    camera.up = Vector3 {0.0f, 1.0f, 0.0f};
    camera.fovy = 66.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetTargetFPS(240);
    DisableCursor();
    bool cursorDisabled = true;
    while (!WindowShouldClose()) {
        Vector3 chunkPos = roundToChunk(camera.position);
        int cx = static_cast<int>(chunkPos.x);
        int cy = static_cast<int>(chunkPos.y);
        std::unordered_set<chunk, ChunkHash> neededChunks;
        for(int i = -1*RENDER_DISTANCE; i<=RENDER_DISTANCE; i++){
            for(int j = -1*RENDER_DISTANCE; j<=RENDER_DISTANCE; j++){
                chunk c{ cx - i, cy - j };
                neededChunks.insert(c);
                generateChunkAt(c.x, c.y);
            }
        }
        std::vector<chunk> toRemove;
        for (const auto& [c, model] : chunks) {
            if (neededChunks.find(c) == neededChunks.end()) {
                toRemove.push_back(c);
            }
        }
        for (auto& c : toRemove) {
            UnloadModel(chunks[c]);
            chunks.erase(c);
        }
        if(IsKeyPressed(KEY_ESCAPE)){
            EnableCursor();
            cursorDisabled = false;
        } 
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && cursorDisabled==false){
            DisableCursor();
            cursorDisabled = true;
        }  
        if(cursorDisabled){
            UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        }
        BeginDrawing();
        ClearBackground(BLACK);
            BeginMode3D(camera);
                for (auto const& [coord, model] : chunks) {
                    DrawModel(model, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, PURPLE);
                    DrawModelWires(model, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, SKYBLUE);
                }
                DrawFPS(10, 10);
        EndDrawing();
    }
    return 0;
}
