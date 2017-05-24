#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <time.h>
#include <math.h>
#include <map>

bool show_test_window = false;

namespace ClothMesh {
	void setupClothMesh();
	void cleanupClothMesh();
	void updateClothMesh(float *array_data);
	void drawClothMesh();
};

namespace Sphere {
	void setupSphere(glm::vec3 pos, float radius);
	void cleanupSphere();
	void updateSphere(glm::vec3 pos, float radius);
	void drawSphere();
}

//Mesh variables
const int meshRows = 40;
const int meshColumns = 40;
const int totalVertex = meshRows * meshColumns;
static float L = 0.25f;

//Wave variables
static int resetTime = 20;
static float dtCounter = 0;
static float height = 2.0f;
static float amplitude = 0.3f;
static float amplitude2 = 0.5f;
static float frequency = 3.5f;
static float frequency2 = 3.5f;
static bool multipleWaves = false;
static int totalWaves = 3;

//Variables for reseting
static float lastHeight, lastAmplitude, lastAmplitude2, lastFrequency, lastMass, lastRadius, lastDensity, lastFrequency2;
static int lastTotalWaves, lastTime;
static bool lastMultipleWaves;

//Arrays for the waves
static float *frequencyArray,*amplitudeArray;
glm::vec3 *waveVectorArray,*nodeVectors,*initalVector;
glm::vec3 waveVector = { 1,0,0 };

//Sphere variables
static float mass = 1.0f;
static float gravity = 9.81f;
static float radius = 0.5f;
static float density = 10.f;
static float v_sub, fluidHeight, sphereCorrectPosition;
glm::vec3 y = { 0,1,0 };
glm::vec3 sphereVector, lastSphereVector, startingVector, buoyancy, sphereVelocity;
float *closePos,sphereFluidDistance, sinkedVolume;

void GUI() {

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

	//Gerstner wave tweakable variables
	ImGui::Text("GERSTNER WAVE");
	ImGui::SliderInt("Reset Time", &resetTime, 0, 20); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Mesh height", &height, 0.1f, 9.9f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Amplitude", &amplitude, 0.1f, 0.9f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Frequency", &frequency, 0.1f, 5.f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat3("Wave vector", &waveVector.x, 0.f,0.9f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

	//Multiple Gerstner waves activation and variables
	ImGui::Text("MULTIPLE GERSTNER WAVES");
	ImGui::Checkbox("Activate multiple waves", &multipleWaves); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderInt("Total Waves", &totalWaves, 1, 5); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Amplitude", &amplitude2, 0.1f, 0.9f);	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Frequency", &frequency2, 0.1f, 5.f);	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

	//Sphere tweakable variables
	ImGui::Text("BUOYANCY SPHERE");
	ImGui::SliderFloat("Sphere mass", &mass, 0.01f, 10.f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Sphere density", &density, 0.1f, 20.f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat("Sphere radius", &radius, 0.1f, 10.f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
	ImGui::SliderFloat3("Sphere vector", &startingVector.x, 0.5f, 9.f); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

//Insertion sort algorithm to sort highest and lowest values of an array
void insertionSort(float arr[], int length) {

	int i, j, tmp;

	for (i = 1; i < length; i++) {
		j = i;
		while (j > 0 && arr[j - 1] > arr[j]) {
			tmp = arr[j];
			arr[j] = arr[j - 1];
			arr[j - 1] = tmp;
			j--;
		}
	}
}

void getfluidHeight(int closestNodes) {

	//Calculus of the nearest nodes to the sphere
	for (int i = 0; i < totalVertex; i++) { closePos[i] = glm::distance(sphereVector.y,nodeVectors[i].y); } //Calculus of the distance between center of the sphere and nodes of the mesh

	insertionSort(closePos, totalVertex); //Sort the distance array to access closest nodes values

										  //Get the "x" most closest values
	for (int i = totalVertex - 1; i > totalVertex - closestNodes - 1; i--) { fluidHeight += closePos[i]; }
	fluidHeight = fluidHeight / closestNodes; //Generate an average value

}

void calculateBuoyancy(float dt) {

	sphereCorrectPosition = radius - sphereVector.y; //Calculate distance between the sphere and the fluid
	sphereFluidDistance = glm::distance(fluidHeight, sphereCorrectPosition);

	//If the sphere is below the fluid apply buoyancy
	std::cout << sphereFluidDistance << std::endl;
	if (sphereFluidDistance < 0) {
		float diameter = (radius * 2)*(radius * 2);
		sinkedVolume = diameter*sphereFluidDistance; //Calculation of v_sub
		buoyancy = (density * gravity * sinkedVolume) * y; //Total calculation of buoyancy 
		sphereVelocity += (buoyancy/mass) * dt; //Apply buoyancy to sphere vector
		std::cout << buoyancy.y << std::endl;
	}

}


void initMultipleWaves() {

	//Creation of all the waves arrays
	waveVectorArray = new glm::vec3[totalWaves];
	frequencyArray = new float[totalWaves];
	amplitudeArray = new float[totalWaves];

	//Applying values to "last" variables for reseting
	lastTotalWaves = totalWaves; lastMultipleWaves = multipleWaves; lastAmplitude2 = amplitude2; lastFrequency2 = frequency2;

	//For that creates wave vectors, frequency and amplitude for each of the total waves
	for (int i = 0; i < totalWaves; i++) {
		waveVectorArray[i] = glm::vec3(-rand() % 3, 0, rand() % 3); 
		frequencyArray[i] = frequency;
		amplitudeArray[i] = amplitude2 / (totalWaves*2);
	}
}

void PhysicsInit() {

	//Creation of all glm::vec3 arrays
	nodeVectors = new glm::vec3[totalVertex];
	initalVector = new glm::vec3[totalVertex];
	closePos = new float[totalVertex];
	sphereVector = startingVector;
	sphereVelocity = { 0,0,0 };

	//Sphere setup
	Sphere::setupSphere(sphereVector, radius);

	//Declaration of mesh counters
	int columnsCounter = 0;
	int rowsCounter = 0;

	//Applying values to "last" variables for reseting
	lastTime = resetTime; lastHeight = height; lastAmplitude = amplitude; lastFrequency = frequency;
	lastRadius = radius; lastMass = mass; lastDensity = density; lastSphereVector = startingVector;

	//For that creates the Mesh with an "L" separation
	for (int i = 0; i < totalVertex; i++) { 
		nodeVectors[i] = { L * columnsCounter - (L*meshColumns / 2) + L / 2,height, L * rowsCounter - (L*meshRows / 2) + L / 2 };
		initalVector[i] = nodeVectors[i];
		if (columnsCounter >= meshColumns - 1) { columnsCounter = 0; rowsCounter += 1;}
		else { columnsCounter += 1; }
	}

	//Sphere calculus
	getfluidHeight(10);
}

//Check variables changes to reset the simulation
void checkChanges() { if (lastHeight != height || lastTime != resetTime || lastFrequency != frequency || lastAmplitude != amplitude  || lastMass != mass || lastRadius != radius  || lastDensity != density || lastSphereVector != startingVector) PhysicsInit(); }

void checkWavesChanges() { if (lastTotalWaves != totalWaves || lastAmplitude2 != amplitude2 || lastFrequency2 != frequency2) initMultipleWaves(); }

void PhysicsUpdate(float dt) {
	
	dtCounter += dt;

	checkChanges(); //Checks for variables changes
	checkWavesChanges(); //Checks for waves variable changes
	
	//Calculus of Gerstner Waves
	switch (multipleWaves) {

		case true:  //Applying more than one Gerstner waves

			for (int i = 0; i < totalVertex; i++) { //For each of the nodes

				nodeVectors[i] = initalVector[i] - (waveVectorArray[0] / glm::length(waveVectorArray[0])) * amplitudeArray[0] * sin(glm::dot(waveVectorArray[0], initalVector[i]) - frequencyArray[0] * dtCounter); //Calculation of Gernster Waves 'X' and 'Z'
				nodeVectors[i].y = amplitudeArray[0] * cos(glm::dot(waveVectorArray[0], initalVector[i]) - frequencyArray[0] * dtCounter) + height; //Calculation of Gernster Waves 'Y'

				for (int j = 1; j < totalWaves; j++) { //For each of the total waves we defined

					nodeVectors[i] -= (waveVectorArray[j] / glm::length(waveVectorArray[j])) * amplitudeArray[j] * sin(glm::dot(waveVectorArray[j], initalVector[i]) - frequencyArray[j] * dtCounter); //Apply the calculus of new waves to the original one for 'X' and 'Z'
					nodeVectors[i].y += amplitudeArray[j] * cos(glm::dot(waveVectorArray[j], initalVector[i]) - frequencyArray[j] * dtCounter);  //Apply the calculus of new waves to the original one for 'Y'

				}	
			}

		break;
		 
		case false:  //Applying one Gerstner wave
		
			for (int i = 0; i < totalVertex; i++) { //For each of the nodes

				nodeVectors[i] = initalVector[i] - (waveVector / glm::length(waveVector)) * amplitude * sin(glm::dot(waveVector, initalVector[i]) - frequency * dtCounter); //Calculation of Gernster Waves 'X'
				nodeVectors[i].y = amplitude * cos(glm::dot(waveVector, initalVector[i]) - frequency * dtCounter) + height; //Calculation of Gernster Waves 'Y'

			}
			
		break;

		}
	
	//Calculus of external forces on a Sphere
	sphereVelocity.y += -gravity * dt; //Apply gravity to the sphere
	calculateBuoyancy(dt); //Calculate buoyancy force and apply it if necessary
	sphereVector += sphereVelocity * dt;
	if (dtCounter >= resetTime) {PhysicsInit(); dtCounter = 0; } //Reset every "x" seconds

	ClothMesh::updateClothMesh(&nodeVectors[0].x); //Update the mesh positions
	Sphere::updateSphere(sphereVector, radius);
}

void PhysicsCleanup() {}