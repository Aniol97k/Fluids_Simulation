#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <time.h>
#include <math.h>

bool show_test_window = false;

namespace ClothMesh {
	void setupClothMesh();
	void cleanupClothMesh();
	void updateClothMesh(float *array_data);
	void drawClothMesh();
};

//Mesh variables
const int meshRows = 20;
const int meshColumns = 20;
const int totalVertex = meshRows * meshColumns;

static float L = 0.5f;
static int resetTime = 10;
static float dtCounter = 0;
static float height = 2.0f;
static int lastTime;
static float lastHeight;

//Mesh arrays
glm::vec3 *nodeVectors;
glm::vec3 *lastVectors;
glm::vec3 *velVectors;
glm::vec3 *newVectors;
glm::vec3 *forceVectors;

//Cube planes
glm::vec3 groundN = { 0,1,0 };
glm::vec3 roofN = { 0,-1,0 };
glm::vec3 leftN = { 1,0,0 };
glm::vec3 rightN = { -1,0,0 };
glm::vec3 backN = { 0,0,1 };
glm::vec3 frontN = { 0,0,-1 };

void GUI() {

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::SliderInt("Reset Time", &resetTime, 0, 20);
	ImGui::SliderFloat("Mesh height", &height, 0.1f, 9.9f);

	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void reset() {

	//Function that resets to the beggining all the positions, forces and velocites of the mesh
	int columnsCounter = 0;
	int rowsCounter = 0;
	lastTime = resetTime;
	lastHeight = height;

	for (int i = 0; i < totalVertex; i++) {
		nodeVectors[i] = { L * columnsCounter - (L*meshColumns / 2) + L / 2,height, L * rowsCounter - (L*meshRows / 2) + L / 2 };
		velVectors[i] = { 0,0,0 };
		newVectors[i] = nodeVectors[i];
		forceVectors[i] = { 0,0,0 };
		if (columnsCounter >= meshColumns - 1) {
			columnsCounter = 0;
			rowsCounter += 1;
		}
		else { columnsCounter += 1; }
	}
}

void checkChanges() { if (lastHeight != height || lastTime != resetTime) reset(); }

void PhysicsInit() {

	//Creation of all glm::vec3 arrays
	nodeVectors = new glm::vec3[totalVertex];
	velVectors = new glm::vec3[totalVertex];
	newVectors = new glm::vec3[totalVertex];
	forceVectors = new glm::vec3[totalVertex];
	lastVectors = new glm::vec3[totalVertex];

	//Declaration of mesh counters
	int columnsCounter = 0;
	int rowsCounter = 0;

	//Applying values to "last" variables for reseting
	lastTime = resetTime;
	lastHeight = height;

	//For that creates the Mesh with an "L" separation
	for (int i = 0; i < totalVertex; i++) {
		nodeVectors[i] = { L * columnsCounter - (L*meshColumns / 2) + L / 2,height, L * rowsCounter - (L*meshRows / 2) + L / 2 };
		velVectors[i] = { 0,0,0 };
		newVectors[i] = nodeVectors[i];
		forceVectors[i] = { 0,0,0 };
		if (columnsCounter >= meshColumns - 1) {
			columnsCounter = 0;
			rowsCounter += 1;
		}
		else { columnsCounter += 1; }
	}
}


void PhysicsUpdate(float dt) {

	for (int i = 0; i < totalVertex; i++) { //Applying Euler's solver and updating

		lastVectors[i] = nodeVectors[i]; //Store last position vector

		//Velocities and gravity
		velVectors[i].x = velVectors[i].x + dt * forceVectors[i].x;
		velVectors[i].z = velVectors[i].z + dt * forceVectors[i].z;

		forceVectors[i] = glm::vec3(0, 0, 0); //Reset forces
		
		newVectors[i] = nodeVectors[i] + dt * velVectors[i]; //Euler 

		nodeVectors[i] = newVectors[i]; //Update position

		}
	
	checkChanges();
	dtCounter += dt;

	if (dtCounter >= resetTime) { reset(); dtCounter = 0; } //Reset every "x" seconds

	ClothMesh::updateClothMesh(&nodeVectors[0].x);

}


void PhysicsCleanup() {}