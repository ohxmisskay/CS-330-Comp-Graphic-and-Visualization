#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Camera.h" // Camera class

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"    // Image loading Utility functions



using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "7-1 Final Project"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;     // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nVertices;   // Number of vertices of the mesh
		GLuint nIndices;
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	GLMesh gMesh;
	// Texture IDs
	GLuint texture0, texture1, texture2, texture3, texture4, texture5,
		texture6, texture7, texture8, texture9, texture10, texture11,
		texture12, texture13, texture14, texture15, texture16;
	// Defining both shader programs
	GLuint gProgramId;
	GLuint gRedFaceTextureId;
	GLuint gYellowFaceTextureId;
	GLuint gWhiteFaceTextureId;
	GLuint gGreenFaceTextureId;
	GLuint gBlueFaceTextureId;
	GLuint gOrangeFaceTextureId;

	bool perspective = false;

	// camera
	Camera gCamera(glm::vec3(0.0f, 1.5f, 5.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

	//Shape Meshes from Professor Brian
	Meshes meshes;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void flipImageVertically(unsigned char* image, int width, int height, int channels);
void URender();
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);


////////////////////////////////////////////////////////////////////////////////////////
// SHADER CODE
/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 vertexPosition; // VAP position 0 for vertex position data
layout(location = 1) in vec3 vertexNormal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexFragmentNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(vertexPosition, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(vertexPosition, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexFragmentNormal = mat3(transpose(inverse(model))) * vertexNormal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);


/* Surface Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

	in vec3 vertexFragmentNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec4 objectColor;
uniform vec3 ambientColor;
uniform vec3 light1Color;
uniform vec3 light1Position;
uniform vec3 light2Color;
uniform vec3 light2Position;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform bool ubHasTexture;
uniform float ambientStrength = 0.1f; // Set ambient or global lighting strength
uniform float specularIntensity1 = 0.8f;
uniform float highlightSize1 = 16.0f;
uniform float specularIntensity2 = 0.8f;
uniform float highlightSize2 = 16.0f;

struct FlashLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
};

struct Material {
	vec3 diffuseColor;
	vec3 specularColor;
	float shininess;
};

uniform FlashLight flashLight;
uniform Material currentMaterial;

// function prototypes
vec3 CalcFlashLight(FlashLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting
	vec3 ambient = ambientStrength * ambientColor; // Generate ambient light color

	//**Calculate Diffuse lighting**
	vec3 norm = normalize(vertexFragmentNormal); // Normalize vectors to 1 unit
	vec3 light1Direction = normalize(light1Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact1 = max(dot(norm, light1Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse1 = impact1 * light1Color; // Generate diffuse light color

	vec3 light2Direction = normalize(light2Position - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact2 = max(dot(norm, light2Direction), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse2 = impact2 * light2Color; // Generate diffuse light color

	//**Calculate Specular lighting**
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction

	vec3 reflectDir1 = reflect(-light1Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent1 = pow(max(dot(viewDir, reflectDir1), 0.0), highlightSize1);
	vec3 specular1 = specularIntensity1 * specularComponent1 * light1Color;

	vec3 reflectDir2 = reflect(-light2Direction, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
	vec3 specular2 = specularIntensity2 * specularComponent2 * light2Color;


	//**Calculate phong result**
	//Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate);
	vec3 phong1;
	vec3 phong2;
	vec3 flashlightResult;

	if (ubHasTexture == true)
	{
		phong1 = (ambient + diffuse1 + specular1) * textureColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;
		flashlightResult = CalcFlashLight(flashLight, norm, vertexFragmentPos, viewDir);
	}
	else
	{
		phong1 = (ambient + diffuse1 + specular1) * objectColor.xyz;
		phong2 = (ambient + diffuse2 + specular2) * objectColor.xyz;
		//phong3 = (ambient + diffuseColor3 + specular3) * objectColor.xyz;
	}

	fragmentColor = vec4(phong1 + phong2 + flashlightResult, 1.0); // Send lighting results to GPU
	//fragmentColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

// calculates the color when using a flash light.
vec3 CalcFlashLight(FlashLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), currentMaterial.shininess);
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// spotlight intensity
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	// combine results
	vec3 ambient = light.ambientColor * vec3(texture(uTexture, vertexTextureCoordinate));
	vec3 diffuse = (light.diffuseColor + currentMaterial.diffuseColor) * diff * vec3(texture(uTexture, vertexTextureCoordinate));
	vec3 specular = (light.specularColor + currentMaterial.specularColor) * spec * vec3(texture(uTexture, vertexTextureCoordinate));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	return (ambient + diffuse + specular);
}
);
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	//UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
	meshes.CreateMeshes();

	// Create the shader program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);

	// Sets the background color of the window to white (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Load plane
	const char* texFilename = "../7-1 Final Project_Winnie Kwong/Texture/foam_board.jpg";
	if (!UCreateTexture(texFilename, texture0))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load ornament clasp & hook,triforce hook
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/metal.jpg";
	if (!UCreateTexture(texFilename, texture1))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load ornament body
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/ornament_body.jpg";
	if (!UCreateTexture(texFilename, texture2))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Triforce
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/metal_triforce.jpg";
	if (!UCreateTexture(texFilename, texture3))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut bottom body
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/bottom donut.jpg";
	if (!UCreateTexture(texFilename, texture4))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut top body
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/top donut.jpg";
	if (!UCreateTexture(texFilename, texture5))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Rubiks Cube
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/orange.jpg";
	if (!UCreateTexture(texFilename, gOrangeFaceTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Rubiks Cube
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/red.jpg";
	if (!UCreateTexture(texFilename, gRedFaceTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Rubiks Cube
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/green.jpg";
	if (!UCreateTexture(texFilename, gGreenFaceTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Rubiks Cube
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/blue.jpg";
	if (!UCreateTexture(texFilename, gBlueFaceTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Rubiks Cube
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/yellow.jpg";
	if (!UCreateTexture(texFilename, gYellowFaceTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load Rubiks Cube
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/white.jpg";
	if (!UCreateTexture(texFilename, gWhiteFaceTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut sprinkle yellow
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/yellow sprinkle.jpg";
	if (!UCreateTexture(texFilename, texture12))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut sprinkle red
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/red sprinkle.jpg";
	if (!UCreateTexture(texFilename, texture13))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut sprinkle pink
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/pink sprinkle.jpg";
	if (!UCreateTexture(texFilename, texture14))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut sprinkle green
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/green sprinkle.jpg";
	if (!UCreateTexture(texFilename, texture15))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}
	// Load donut sprinkle blue
	texFilename = "../7-1 Final Project_Winnie Kwong/Texture/blue sprinkle.jpg";
	if (!UCreateTexture(texFilename, texture16))
	{
		cout << "Failed to load texture " << texFilename << endl;
	}

	// bind texture on corresponding texture unit
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture5);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gOrangeFaceTextureId);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, gRedFaceTextureId);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, gGreenFaceTextureId);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, gBlueFaceTextureId);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, gYellowFaceTextureId);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, gWhiteFaceTextureId);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, texture12);
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, texture13);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture(GL_TEXTURE_2D, texture14);
	glActiveTexture(GL_TEXTURE15);
	glBindTexture(GL_TEXTURE_2D, texture15);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, texture16);

	gCamera.Position = glm::vec3(0.0f, 1.0f, 16.0f);
	gCamera.Front = glm::vec3(0.0, 0.0, -1.0f);
	gCamera.Up = glm::vec3(0.0, 1.0, 0.0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// input
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	//UDestroyMesh(gMesh);
	meshes.DestroyMeshes();

	// release textures
	UDestroyTexture(texture0);
	UDestroyTexture(texture1);
	UDestroyTexture(texture2);
	UDestroyTexture(texture3);
	UDestroyTexture(texture4);
	UDestroyTexture(texture5);
	UDestroyTexture(gRedFaceTextureId);
	UDestroyTexture(gYellowFaceTextureId);
	UDestroyTexture(gWhiteFaceTextureId);
	UDestroyTexture(gGreenFaceTextureId);
	UDestroyTexture(gBlueFaceTextureId);
	UDestroyTexture(gOrangeFaceTextureId);
	UDestroyTexture(texture12);
	UDestroyTexture(texture13);
	UDestroyTexture(texture14);
	UDestroyTexture(texture15);
	UDestroyTexture(texture16);


	// Release shader program
	UDestroyShaderProgram(gProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		perspective = false;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		perspective = true;


}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}

// Functioned called to render a frame
void URender()
{
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 rotation1;
	glm::mat4 rotation2;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	bool ubHasTextureVal;
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint viewPosLoc;
	GLint ambStrLoc;
	GLint ambColLoc;
	GLint light1ColLoc;
	GLint light1PosLoc;
	GLint light2ColLoc;
	GLint light2PosLoc;
	GLint objColLoc;
	GLint specInt1Loc;
	GLint highlghtSz1Loc;
	GLint specInt2Loc;
	GLint highlghtSz2Loc;
	GLint uHasTextureLoc;

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view = gCamera.GetViewMatrix();

	// Creates a orthographic projection
	if (!perspective) {
		projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}
	else
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	viewPosLoc = glGetUniformLocation(gProgramId, "viewPosition");
	ambStrLoc = glGetUniformLocation(gProgramId, "ambientStrength");
	ambColLoc = glGetUniformLocation(gProgramId, "ambientColor");
	light1ColLoc = glGetUniformLocation(gProgramId, "light1Color");
	light1PosLoc = glGetUniformLocation(gProgramId, "light1Position");
	light2ColLoc = glGetUniformLocation(gProgramId, "light2Color");
	light2PosLoc = glGetUniformLocation(gProgramId, "light2Position");
	objColLoc = glGetUniformLocation(gProgramId, "objectColor");
	specInt1Loc = glGetUniformLocation(gProgramId, "specularIntensity1");
	highlghtSz1Loc = glGetUniformLocation(gProgramId, "highlightSize1");
	specInt2Loc = glGetUniformLocation(gProgramId, "specularIntensity2");
	highlghtSz2Loc = glGetUniformLocation(gProgramId, "highlightSize2");
	uHasTextureLoc = glGetUniformLocation(gProgramId, "ubHasTexture");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the camera view location
	glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

	// pre-set flashlight settings
	glUniform3f(glGetUniformLocation(gProgramId, "flashLight.position"), gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);
	glUniform3f(glGetUniformLocation(gProgramId, "flashLight.direction"), gCamera.Front.x, gCamera.Front.y, gCamera.Front.z);
	glUniform1f(glGetUniformLocation(gProgramId, "flashLight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(gProgramId, "flashLight.outerCutOff"), glm::cos(glm::radians(17.5f)));
	glUniform1f(glGetUniformLocation(gProgramId, "flashLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(gProgramId, "flashLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(gProgramId, "flashLight.quadratic"), 0.032f);
	glUniform3f(glGetUniformLocation(gProgramId, "flashLight.ambientColor"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(gProgramId, "flashLight.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "flashLight.specularColor"), 0.8f, 0.8f, 0.8f);

	//set ambient lighting strength
	glUniform1f(ambStrLoc, 0.4f);
	//set ambient color
	glUniform3f(ambColLoc, 0.5f, 0.5f, 0.5f);
	glUniform3f(light1ColLoc, 0.26f, 0.05f, 0.38f);
	glUniform3f(light1PosLoc, -2.0f, 3.0f, 2.0f);
	glUniform3f(light2ColLoc, 0.0f, 0.20f, 0.44f);
	glUniform3f(light2PosLoc, 5.0f, 3.0f, 2.0f);

	//set specular intensity
	glUniform1f(specInt1Loc, 1.0f);
	glUniform1f(specInt2Loc, 1.0f);

	//set specular highlight size
	glUniform1f(highlghtSz1Loc, 12.0f);
	glUniform1f(highlghtSz2Loc, 12.0f);

	ubHasTextureVal = true;
	glUniform1i(uHasTextureLoc, ubHasTextureVal);

	// White Styrfoam Information (Plane)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(13.0f, 13.0f, 13.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.3f, 0.3f, 0.3f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Christmas Ornament Clasp (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.35f, 0.45f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(100.0f, glm::vec3(1.0, 1.0f, 270.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(3.35f, 0.35f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 1);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Christmas Ornament Hook (Torus)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTorusMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(3.35f, 0.33f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Christmas Ornament Body (Sphere)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gSphereMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(2.2f, 2.2f, 2.2f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(5.0f, 2.2f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 2);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.3f, 0.3f, 0.5f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Triforce Left (Prism)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPrismMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.1f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(4.0f, 0.06f, -3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 3);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.3f, 0.3f, 0.3f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPrismMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Triforce Center (Prism)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPrismMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.1f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(4.5f, 0.06f, -3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPrismMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Triforce Right (Prism)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPrismMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.5f, 0.1f, 0.5f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(4.25f, 0.06f, -2.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_STRIP, 0, meshes.gPrismMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Triforce Hook 1 (Torus)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTorusMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.05f, 0.05f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(4.19f, glm::vec3(1.0f, 1.0f, 1.f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(4.78f, 0.05f, -3.26f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 1);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.4f, 0.4f, 0.4f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.3f, 0.3f, 0.3f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Triforce Hook 2 (Torus)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTorusMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(3.0f, glm::vec3(1.0f, 270.0f, 180.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(4.90f, 0.06f, -3.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gTorusMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Main Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.9f, 1.2f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.26f, 0.5f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Get texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 4);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.2f, 0.2f, 0.2f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle LH Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.26f, 0.65f, 8.45f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Corner Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.9f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.96f, 0.5f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle LH Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.66f, 0.65f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Main Down Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.2f, 0.9f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.26f, 0.5f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle RH Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.76f, 0.65f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Corner Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.9f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.46f, 0.5f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle RH Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.16f, 0.65f, 8.450f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Main Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.9f, 1.2f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.16f, 0.5f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle RH Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.16f, 0.65f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Corner Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.9f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.46f, 0.5f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle RH Up-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.76f, 0.65f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Main Up Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.2f, 0.9f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.21f, 0.5f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle LH Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.66f, 0.65f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Corner Up-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.9f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.96f, 0.5f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Bottom Middle LH Up-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.6f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.26f, 0.65f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Main Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.3f, 1.2f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.21f, 1.1f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Get texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 5);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.2f, 0.2f, 0.2f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top LH Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.26f, 1.1f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Corner Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.96f, 1.1f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Inner Corner Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.66f, 1.1f, 8.45f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top LH Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.96f, 1.1f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Main Down Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.2f, 0.3f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.21f, 1.1f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top RH Down-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.46f, 1.1f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Corner Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.46f, 1.1f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Inner Corner Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.76f, 1.1f, 8.45f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top RH Down-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.16f, 1.1f, 8.75));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Main Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.9f, 0.3f, 1.2f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.16f, 1.1f, 8.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top RH Up-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.46f, 1.1f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Corner Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.46f, 1.1f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Inner Corner Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.76f, 1.1f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top RH Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.16f, 1.1f, 7.25));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top LH Up-Left Box)
// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.26f, 1.1f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Corner Up-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.96f, 1.1f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Inner Corner Up-Left Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.66f, 1.1f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top LH Up-Right Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.3f, 0.3f, 0.3f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.96f, 1.1f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut (Top Main Up Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.2f, 0.3f, 0.9f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.21f, 1.1f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Sprinkles start from top-down, left-right
	//  
	// Donut sprinkle yellow 1/4 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.05f, 1.2f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 12);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 16.0f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle yellow 2/4 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.25f, 1.2f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle yellow 3/4 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.73f, 1.2f, 8.45f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle yellow 4/4 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.35f, 1.2f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle red 1/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.95f, 1.2f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 13);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 16.0f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle red 2/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.43f, 1.2f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle red 3/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.25f, 1.2f, 7.85f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle red 4/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.95f, 1.2f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle red 5/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.45f, 1.2f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle pink 1/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.73f, 1.2f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 14);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 16.0f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle pink 2/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.68f, 1.2f, 7.55f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle pink 3/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.13f, 1.2f, 7.85f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle pink 4/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-5.25f, 1.2f, 8.45f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle pink 5/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.05f, 1.2f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle green 1/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.65f, 1.2f, 6.95f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 15);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 16.0f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle green 2/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.43f, 1.2f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle green 3/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.95f, 1.2f, 8.15f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle green 4/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.13f, 1.2f, 8.45f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle green 5/5 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.75f, 1.2f, 9.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle blue 1/3 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.35f, 1.2f, 7.25f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws texture
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 16);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.6f, 0.6f, 0.6f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 16.0f);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle blue 2/3 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.43f, 1.2f, 8.15f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Donut sprinkle blue 3/3 (Cylinder)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.15f, 0.3f, 0.15f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-4.65f, 1.2f, 8.75f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// Rubiks Cube (Box)
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);
	// 1. Scales the object
	scale = glm::scale(glm::vec3(5.0f, 5.0f, 5.0f));
	// 2. Rotate the object
	rotation = glm::rotate(1.0f, glm::vec3(0.1f, 25.0f, 0.1f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-3.90f, 2.52f, -3.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.diffuseColor"), 0.4f, 0.4f, 0.4f);
	glUniform3f(glGetUniformLocation(gProgramId, "currentMaterial.specularColor"), 0.5f, 0.5f, 0.5f);
	glUniform1f(glGetUniformLocation(gProgramId, "currentMaterial.shininess"), 32.f);
	// Draws texture
	// back
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 11);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
	// front
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 10);
	glDrawArrays(GL_TRIANGLE_FAN, 6, 6);
	// left
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 9);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 6);
	// right
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 8);
	glDrawArrays(GL_TRIANGLE_FAN, 18, 6);
	// bottom
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 7);
	glDrawArrays(GL_TRIANGLE_FAN, 24, 6);
	// top
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 6);
	glDrawArrays(GL_TRIANGLE_FAN, 30, 6);
	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, meshes.gBoxMesh.nVertices);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.

}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}

void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);

	if (image)
	{
		flipImageVertically(image, width, height, channels);

		// generates texture names
		glGenTextures(1, &textureId);
		// binding texure to 2D texture
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set texture wrapping params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Assigning texture to pointer, and defining how it will be stored in memory
		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << "channels" << endl;
			return false;
		}

		// generating mipmap for GL_TEXTURE_2D
		glGenerateMipmap(GL_TEXTURE_2D);

		// free loaded image
		stbi_image_free(image);
		// rebinding GL_TEXTURE_2D to nothing
		glBindTexture(GL_TEXTURE_2D, 0);

		return true;
	}

	return false;
}

void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}