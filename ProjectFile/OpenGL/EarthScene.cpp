#include "EarthScene.h"
#include "TextureLoader.h"
#include "ShaderLoader.h"
#include <iostream>


using namespace std;



EarthScene::EarthScene(SampleSize _sample, Resolution _resolution)
{

	// Camera settings
	//							  width, heigh, near plane, far plane
	camera_settings = { 960  * (unsigned int)_resolution, 540 * (unsigned int)_resolution, 0.1, 100.0 };

	marbleSphere = new Sphere(32, 16, 0.1f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), CG_RIGHTHANDED);

	// Instanciate the camera object with basic data
	earthCamera = new Camera(camera_settings, glm::vec3(0.0, 0.0, 5.0));


	//
	// Setup textures for rendering the Earth model
	//

	marbleTexture0 = TextureLoader::loadTexture(string("Resources\\Models\\Marbles\\Marble0.jpg"), TextureGenProperties(GL_SRGB8_ALPHA8));
	marbleTexture1 = TextureLoader::loadTexture(string("Resources\\Models\\Marbles\\Marble1.jpg"), TextureGenProperties(GL_SRGB8_ALPHA8));
	marbleTexture2 = TextureLoader::loadTexture(string("Resources\\Models\\Marbles\\Marble2.jpg"), TextureGenProperties(GL_SRGB8_ALPHA8));


	GLSL_ERROR glsl_err = ShaderLoader::createShaderProgram(
		"Resources\\Shaders\\CustomShader.vert",
		"Resources\\Shaders\\CustomShader.frag",
		&CustomShader);

	shader = CustomShader;

	marbleTextureUniform0 = glGetUniformLocation(shader, "modelTexture");
	modelMatrixLocation = glGetUniformLocation(shader, "modelMatrix");
	invTransposeMatrixLocation = glGetUniformLocation(shader, "invTransposeModelMatrix");
	viewProjectionMatrixLocation = glGetUniformLocation(shader, "viewProjectionMatrix");
	lightDirectionLocation = glGetUniformLocation(shader, "lightDirection");
	lightDiffuseLocation = glGetUniformLocation(shader, "lightDiffuseColour");
	lightSpecularLocation = glGetUniformLocation(shader, "lightSpecularColour");
	lightSpecExpLocation = glGetUniformLocation(shader, "lightSpecularExponent");
	cameraPosLocation = glGetUniformLocation(shader, "cameraPos");
	blurAmount = glGetUniformLocation(shader, "blurAmount");
	screenWidth = glGetUniformLocation(shader, "screenWidth");
	screenHeight = glGetUniformLocation(shader, "screenHeight");
	
	// Set constant uniform data (uniforms that will not change while the application is running)
	// Note: Remember we need to bind the shader before we can set uniform variables!
	glUseProgram(shader);

	glUniform1i(marbleTextureUniform0, 0);

	glUseProgram(0);


	//
	// Setup FBO (which Earth rendering pass will draw into)
	//

	glGenFramebuffers(1, &demoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, demoFBO);


	//
	// Setup textures that will be drawn into through the FBO
	//

	// Setup colour buffer texture.
	// Note:  The texture is stored as linear RGB values (GL_RGBA8).  
	//There is no need to pass a pointer to image data - 
	//we're going to fill in the image when we render the Earth scene at render time!
	
	glGenTextures(1, &fboColourTexture);

	glBindTexture(GL_TEXTURE_2D, fboColourTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, camera_settings.screenWidth, camera_settings.screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	if (shaderType >= 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Setup depth texture

	glGenTextures(1, &fboDepthTexture);
	glBindTexture(GL_TEXTURE_2D, fboDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, camera_settings.screenWidth, camera_settings.screenHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

	if (shaderType >= 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	// Attach textures to the FBO
	//
	
	// Attach the colour texture object to the framebuffer object's colour attachment point #0
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		fboColourTexture,
		0);

	// Attach the depth texture object to the framebuffer object's depth attachment point
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D,
		fboDepthTexture,
		0);	

	//
	// Before proceeding make sure FBO can be used for rendering
	//

	GLenum demoFBOStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (demoFBOStatus != GL_FRAMEBUFFER_COMPLETE) {

		fboOkay = false;
		cout << "Could not successfully create framebuffer object to render texture!" << endl;

	}
	else {

		fboOkay = true;
		cout << "FBO successfully created" << endl;
	}

	// Unbind FBO for now! (Plug main framebuffer back in as rendering destination)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	//
	// Setup demo / animation variables
	//

	sunTheta = 0.0f;
	earthTheta = 0.0f;
}


EarthScene::~EarthScene() {

}


// Accessor methods

Camera* EarthScene::getEarthSceneCamera() {

	return earthCamera;
}


GLuint EarthScene::getEarthSceneTexture() {

	return fboColourTexture;
}


float EarthScene::getSunTheta() {

	return sunTheta;
}


void EarthScene::updateSunTheta(float thetaDelta) {

	sunTheta += thetaDelta;
}


// Scene update
void EarthScene::update(const float timeDelta) {

	// Update rotation angle ready for next frame
	earthTheta += 5.0f * float(timeDelta); 
	updateSunTheta(timeDelta * 5.0f);
	marbleSphere->render();
}



// Rendering methods

void EarthScene::render() {

	if (!fboOkay)
		return; // Don't render anything if the FBO was not created successfully

	// Bind framebuffer object so all rendering redirected to attached images (i.e. our texture)
	glBindFramebuffer(GL_FRAMEBUFFER, demoFBO);

	// All rendering from this point goes to the bound textures (setup at initialisation time) and NOT the actual screen!!!!!

	// Clear the screen (i.e. the texture)
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_DEPTH_TEST);

	// Set viewport to specified texture size (see above)
	glViewport(0, 0, camera_settings.screenWidth, camera_settings.screenHeight);

	// Get view-projection transform as a CGMatrix4
	glm::mat4 T = earthCamera->getProjectionMatrix() * earthCamera->getViewMatrix();

	if (shader)
	{
		const int cubeAmount = 1600;
		vector<glm::vec3> cubePositions;
		cubePositions.reserve(cubeAmount);

		for (int i = 0; i < 40; i++)
		{
			float y = 3 - (i * 0.2f);
			for (int j = 0; j < 40; j++)
			{
				float x = -4 + (j * 0.2f);
				cubePositions.emplace_back(glm::vec3(x, y, 0.0f));
			}
		}

		glUseProgram(shader);

		int j = 0;
		for (int i = 0; i < cubeAmount; i++)
		{
			// Modelling transform
			glm::mat4 modelTransform;
			modelTransform = glm::rotate(glm::mat4(1.0), glm::radians(23.44f), glm::vec3(0.0, 0.0, 1.0));		//Earth tilt
			modelTransform = glm::translate(glm::mat4(1.0), cubePositions[i]);									// Move position
			modelTransform = glm::rotate(modelTransform, glm::radians(earthTheta), glm::vec3(0.0, 1.0, 0.0));	//Earth rotation

			// Calculate inverse transpose of the modelling transform for correct transformation of normal vectors
			glm::mat4 inverseTranspose = glm::transpose(glm::inverse(modelTransform));;

			// Get the location of the camera in world coords and set the corresponding uniform in the shader
			glm::vec3 cameraPos = earthCamera->getCameraPosition();
			glUniform3fv(cameraPosLocation, 1, (GLfloat*)&cameraPos);

			// Set the model, view and projection matrix uniforms (from the camera data obtained above)
			glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelTransform));
			glUniformMatrix4fv(invTransposeMatrixLocation, 1, GL_FALSE, glm::value_ptr(inverseTranspose));
			glUniformMatrix4fv(viewProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(T));

			// Set the light direction uniform vector in world coordinates based on the Sun's position
			glUniform4f(lightDirectionLocation, cosf(glm::radians(sunTheta)), 0.0f, sinf(glm::radians(sunTheta)), 0.0f);

			glUniform4f(lightDiffuseLocation, 1.0f, 1.0f, 1.0f, 1.0f); // white diffuse light
			glUniform4f(lightSpecularLocation, 0.4f, 0.4f, 0.4f, 1.0f); // white specular light
			glUniform1f(lightSpecExpLocation, 8.0f); // specular exponent / falloff
			glUniform1f(blurAmount, 8.0f); // specular exponent / falloff
			glUniform1f(screenWidth, (float)camera_settings.screenWidth); // specular exponent / falloff
			glUniform1f(screenHeight, (float)camera_settings.screenHeight); // specular exponent / falloff

			// Activate and Bind the textures to texture units
			glActiveTexture(GL_TEXTURE0);
			if (j == 0)
			{
				glBindTexture(GL_TEXTURE_2D, marbleTexture0); 
				j++;
			}
			else if (j == 1)
			{
				glBindTexture(GL_TEXTURE_2D, marbleTexture1);
				j++;
			}
			else if (j == 2)
			{
				glBindTexture(GL_TEXTURE_2D, marbleTexture2);
				j = 0;
			}

			//Render the model
			marbleSphere->render();
		}

	}

	// Restore default OpenGL shaders (Fixed function operations)
	glUseProgram(0);

	// Set OpenGL to render to the MAIN framebuffer (ie. the screen itself!!)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
