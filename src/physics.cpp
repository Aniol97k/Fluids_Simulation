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
static int resetTime = 20;
static float dtCounter = 0;
static float height = 2.0f;
static float amplitude = 0.3f;
static float frequency = 3.5f;

static int lastTime;
static float lastHeight, lastAmplitude, lastFrequency;

//Mesh arrays
glm::vec3 *nodeVectors;
glm::vec3 *initalVector;
glm::vec3 waveVector = { 1,0,0 };


void GUI() {

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderInt("Reset Time", &resetTime, 0, 20);
	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Mesh height", &height, 0.1f, 9.9f);
	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Amplitude", &amplitude, 0.1f, 0.9f);
	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Frequency", &frequency, 0.1f, 5.f);

	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void PhysicsInit() {

	//Creation of all glm::vec3 arrays
	nodeVectors = new glm::vec3[totalVertex];
	initalVector = new glm::vec3[totalVertex];

	//Declaration of mesh counters
	int columnsCounter = 0;
	int rowsCounter = 0;

	//Applying values to "last" variables for reseting
	lastTime = resetTime;	lastHeight = height;	lastAmplitude = amplitude;	lastFrequency = lastFrequency;

	//For that creates the Mesh with an "L" separation
	for (int i = 0; i < totalVertex; i++) { 
		nodeVectors[i] = { L * columnsCounter - (L*meshColumns / 2) + L / 2,height, L * rowsCounter - (L*meshRows / 2) + L / 2 };
		initalVector[i] = nodeVectors[i];

		if (columnsCounter >= meshColumns - 1) {
			columnsCounter = 0;
			rowsCounter += 1;
		}
		else { columnsCounter += 1; }
	}
}

void checkChanges() { if (lastHeight != height || lastTime != resetTime || lastFrequency != frequency || lastAmplitude != amplitude) PhysicsInit(); }

void PhysicsUpdate(float dt) {
	
	dtCounter += dt;
	checkChanges();

	for (int i = 0; i < totalVertex; i++) { //Applying Gerstner waves

		nodeVectors[i] = initalVector[i] - (waveVector /glm::length(waveVector)) * amplitude * sin(glm::dot(waveVector,initalVector[i]) - frequency * dtCounter); //Calculation of Gernster Waves 'X'
		nodeVectors[i].y = amplitude * cos(glm::dot(waveVector, initalVector[i]) - frequency * dtCounter) + height; //Calculation of Gernster Waves 'Y'
		
	}
	
	if (dtCounter >= resetTime) {PhysicsInit(); dtCounter = 0; } //Reset every "x" seconds

	ClothMesh::updateClothMesh(&nodeVectors[0].x);

}

void PhysicsCleanup() {}