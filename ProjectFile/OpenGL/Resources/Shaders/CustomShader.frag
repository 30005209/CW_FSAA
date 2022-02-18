
#version 330

//
// basic directional light model
//
uniform vec4		lightDirection; // direction light comes FROM (specified in World Coordinates)
uniform vec4		lightDiffuseColour;
uniform vec4        lightSpecularColour;
uniform float       lightSpecularExponent;

uniform vec3        cameraPos; // to calculate specular lighting in world coordinate space, we need the location of the camera since the specular light
    // term is viewer dependent

uniform sampler2D modelTexture;

uniform float screenWidth;
uniform float screenHeight;
uniform int sampleSize;

//
// input fragment packet (contains interpolated values for the fragment calculated by the rasteriser)
//
in vec4 posWorldCoord;
in vec4 colour;
in vec3 normalWorldCoord;
in vec2 texCoord;

//
// output fragment colour
//
layout (location = 0) out vec4 fragColour;


void main(void) {

    float tx = 1/screenWidth;
    float ty = 1/screenHeight;

	// make sure light direction vector is unit length (store in L)
	vec4 L = normalize(lightDirection);
    
	// important to normalise length of normal otherwise shading artefacts occur
	vec3 N = normalize(normalWorldCoord);
	
    // calculate lambertian term
    float dp = dot(L.xyz, N);
    float lambertian = clamp(dp, 0.0, 1.0);

    //
	// calculate diffuse light colour
    //vec4 texColour = texture(modelTexture, texCoord);

    vec4 texColour = texture(modelTexture, vec2(texCoord.x, texCoord.y));
    vec3 diffuseColour;
    if(sampleSize == 0)
    {
        diffuseColour = texColour.rgb * lightDiffuseColour.rgb * lambertian; // input colour actually diffuse colour
    }
    else
    {
        vec3 col = vec3(1.0);

        if( sampleSize == 5)
        {   
            // work out offsets
             vec2 offsets[5] = vec2[]
             (
                vec2(-tx,       ty),    // TL
                vec2( tx,       ty),    // TR
                vec2( 0.0f,     0.0f),  // CM
                vec2(-tx,      -ty),    // BL
                vec2( tx,       -ty)    // BR
            );

            //make sure to weight things appropriately
            float weightMatrix[5] = float[](
                -1,  -1,
                   5,
                -1,  -1
            );
            
             // get textures using offsets
            vec3 sampleTex[5];
            for(int i = 0; i < 5; i++)
            {
                sampleTex[i] = vec3(texture(modelTexture, texCoord.st + offsets[i]));
            }

            vec3 col = vec3(0.0);

            // obtain colour (with weightMatrix)
            for(int i = 0; i < 5; i++)
                col += sampleTex[i] * weightMatrix[i];
            
            vec3 diffuseColour = col.rgb * lightDiffuseColour.rgb * lambertian; // input colour actually diffuse colour

        }
        if (sampleSize == 9)
        {
            vec2 offsets[9] = vec2[]
             (
                vec2(-tx,       ty),    // TL
                vec2( 0.0f,     ty),    // CR
                vec2( tx,       ty),    // TR
                vec2(-tx,       0.0f),  // CL
                vec2( 0.0f,     0.0f),  // CM
                vec2( tx,       0.0f),  // CR
                vec2(-tx,      -ty),    // BL
                vec2( 0.0f,     -ty),   // BC
                vec2( tx,       -ty)    // BR
            );

            float weightMatrix[9] = float[](
                -1, -1, -1,
                -1,  9, -1,
                -1, -1, -1
            );
            
            vec3 sampleTex[9];
            for(int i = 0; i < 9; i++)
            {
                sampleTex[i] = vec3(texture(modelTexture, texCoord.st + offsets[i]));
            }

            vec3 col = vec3(0.0);

            for(int i = 0; i < 9; i++)
                col += sampleTex[i] * weightMatrix[i];
            
            vec3 diffuseColour = col.rgb * lightDiffuseColour.rgb * lambertian; // input colour actually diffuse colour
        }

    }   
    

    //
    // calculate specular light colour
    //

    // vectors needed for specular light calculation...
    vec3 E = cameraPos - posWorldCoord.xyz; // vector from point on object surface in world coords to camera
    E = normalize(E);
    vec3 R = reflect(-L.xyz, N); // reflected light vector about normal N

    float specularIntensity = pow(max(dot(R, E), 0.0), lightSpecularExponent);
    vec3 specularColour = vec3(1.0f, 1.0f, 1.0f) * lightSpecularColour.rgb * specularIntensity * lambertian;
     
    vec3 dayTime = (diffuseColour + (specularColour)) * lambertian;


    //
    // combine colour components to get final pixel / fragment colour
    //
    

    vec3 finalColour = dayTime;

    // Output final gamma corrected colour to framebuffer
    vec3 P = vec3(1.0 / 2.2);
    fragColour = vec4(pow(finalColour, P), 1.0);
}