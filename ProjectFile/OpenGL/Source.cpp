#include "Includes.h"
#include "EarthScene.h"


// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Camera settings
//							  width, height, near plane, far plane
Camera_settings camera_settings{ 960, 540, 0.1, 100.0 };

//Timer
Timer timer;

// Instantiate the camera object with basic data
Camera camera(camera_settings, glm::vec3(0.0f, 0.5f, 4.0f));

double lastX = camera_settings.screenWidth / 2.0f;
double lastY = camera_settings.screenHeight / 2.0f;

bool			showEarthQuad = false;
EarthScene		*earthScene = nullptr;

int currentSetting = 0;
int maxSetting = 3;
TexturedQuad* earthQuad = nullptr;

#ifndef DETAIL_STRUCTS
#define DETAIL_STRUCTS
enum SampleSize
{
	standard = 0,
	simple = 5,
	high = 9,
};

enum Resolution
{
	x1 = 1,
	x2 = 2,
	x3 = 3,
	x4 = 4
};

#endif DETAIL_STRUCTS


SampleSize sample;
Resolution resolution;

int main()
{
	sample = standard;
	resolution = x1;

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(camera_settings.screenWidth, camera_settings.screenHeight, "Real-Time Rendering: DEMO 3", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set the callback functions
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Rendering settings
	glfwSwapInterval(1);		// glfw enable swap interval to match screen v-sync
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE); //Enables face culling
	glFrontFace(GL_CCW);//Specifies which winding order if front facing
	//glEnable(GL_FRAMEBUFFER_SRGB);

	PrincipleAxes	*principleAxes = new PrincipleAxes();

	////	Shaders - Textures - Models	////
	GLuint phongShader;

	GLSL_ERROR glsl_err = ShaderLoader::createShaderProgram(
		string("Resources\\Shaders\\Basic_shader.vert"),
		string("Resources\\Shaders\\Basic_shader.frag"),
		&phongShader);


	//
	// Earth scene
	//
	bool			leftCtrlPressed = false;

	earthScene = new EarthScene(sample, resolution);
	earthQuad = new TexturedQuad(earthScene->getEarthSceneTexture(), true);


	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);
		timer.tick();

		// This demo performs 2 rendering passes...
		// Pass 1) Render the Earth scene to a texture

		// Pass 2) Render the basic demo scene with the principle axes and textured quad, where the texture on the quad is the Earth scene rendered in pass 1.
	
		//
		// Pass 1. Render the Earth scene
		//
		earthScene->render();


		//
		// Pass 2. Render the basic demo scene to the screen
		//

		// Clear the screen
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Reset the viewport
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(0.28, 0.28, 0.28));
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix();

		// Update earthScene state
		if (earthScene)
			earthScene->update(timer.getDeltaTimeSeconds());

		if (earthQuad)
			earthQuad->render(glm::mat4(1.0));

		glUseProgram(0);

		string title = "FPS: " + std::to_string(timer.averageFPS()) + " SPF: " + std::to_string(timer.currentSPF());
		glfwSetWindowTitle(window, title.c_str());

		// glfw: swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	timer.updateDeltaTime();

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(FORWARD, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(BACKWARD, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(LEFT, timer.getDeltaTimeSeconds());
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(RIGHT, timer.getDeltaTimeSeconds());

	int setting0 = sample;

	if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
		sample = standard;

	if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
		sample = simple;

	if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
		sample = high;


	int setting1 = resolution;

	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
		resolution = x1;

	if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
		resolution = x2;

	if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
		resolution = x3;

	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		resolution = x4;

	if (setting0 != sample || setting1 != resolution)
	{
		cout << "--New Settings--\nSample Size: " << sample << "\nResolution: " << resolution << "\n\n";
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		earthScene = new EarthScene(sample, resolution);
		earthQuad = new TexturedQuad(earthScene->getEarthSceneTexture(), true);

		if (sample == standard)
		{
			glDisable(GL_MULTISAMPLE);
		}
		else
		{
			glEnable(GL_MULTISAMPLE);
			glfwWindowHint(GLFW_SAMPLES, 4);
		}

	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		showEarthQuad = !showEarthQuad;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	glViewport(0, 0, width, height);
	camera.updateScreenSize(width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		Camera *ecam = earthScene->getEarthSceneCamera();
		ecam->processMouseMovement(xoffset, yoffset);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		camera.processMouseMovement(xoffset, yoffset);
	}

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		Camera *ecam = earthScene->getEarthSceneCamera();
		ecam->processMouseScroll(yoffset);
	}
	else
	{
		camera.processMouseScroll(yoffset);
	}
}