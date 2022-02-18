#ifndef EARTH_SCENE_H
#define EARTH_SCENE_H

#include "Sphere.h"
#include "Camera.h"

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

class Sphere;

class EarthScene{

private:

	Sphere						*marbleSphere;

	// Move around the earth with a seperate camera to the main scene camera
	Camera						*earthCamera;
	Camera_settings				camera_settings;

	// Textures for multi-texturing the earth model
	//GLuint							dayTexture;
	//GLuint							nightTexture;
	//GLuint							cloudMaskTexture;


	GLuint							marbleTexture0;
	GLuint							marbleTexture1;
	GLuint							marbleTexture2;

	// Shader for multi-texturing the earth
	GLuint							earthShader;
	GLuint							CustomShader;
	GLuint							marbleShader1;
	GLuint							marbleShader2;
	GLuint							marbleShader3;


	// Unifom locations for earthShader

	// Texture uniforms
	//GLuint							dayTextureUniform;
	//GLuint							nightTextureUniform;
	//GLuint							maskTextureUniform;

	GLuint							marbleTextureUniform0;
	GLuint							marbleTextureUniform1;
	GLuint							marbleTextureUniform2;
	GLuint							marbleTextureUniform3;

	// Camera uniforms
	GLint							modelMatrixLocation;
	GLint							invTransposeMatrixLocation;
	GLint							viewProjectionMatrixLocation;

	// Directional light uniforms
	GLint							lightDirectionLocation;
	GLint							lightDiffuseLocation;
	GLint							lightSpecularLocation;
	GLint							lightSpecExpLocation;
	GLint							cameraPosLocation;
	GLint							blurAmount;
	GLint							screenWidth;
	GLint							screenHeight;


	//
	// Animation state
	//
	float							sunTheta; // Angle to the Sun in the orbital plane of the Earth (the xz plane in the demo)
	float							earthTheta;


	//
	// Framebuffer Object (FBO) variables
	//

	// Actual FBO
	GLuint							demoFBO;

	// Colour texture to render into
	GLuint							fboColourTexture;

	// Depth texture to render into
	GLuint							fboDepthTexture;

	// Flag to indicate that the FBO is valid
	bool							fboOkay;

	GLuint& shader = CustomShader;
	int shaderType;

public:

	EarthScene(SampleSize _sample = standard, Resolution _resolution = x1);
	~EarthScene();

	// Accessor methods
	Camera* getEarthSceneCamera();
	GLuint getEarthSceneTexture();
	float getSunTheta();
	void updateSunTheta(float thetaDelta);

	// Scene update
	void update(const float timeDelta);

	// Rendering methods
	void render();
};

#endif