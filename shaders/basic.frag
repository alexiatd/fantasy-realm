#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;


//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//spotlight
uniform int spot;
vec3 luminaShop = vec3(3.62f, 4.6092f, 96.5292f);


//fog
uniform float fogDensity;
uniform vec3 cameraPos;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float computeShadow()
{
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    normalizedCoords = normalizedCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

    float currentDepth = normalizedCoords.z;

    float bias= 0.005f;
    float shadow= currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;

}

vec3 computeSpot()
{

    vec3 dir1 = normalize(luminaShop - fPosition);
    vec3 color1 = vec3(0.2f, 0.2f, 0.0f);
    
    float ambient = 0.3f;
    float specularLight = 0.8f;

    float outerCone = 0.2f;
    float innerCone = 0.55f;

    vec3 normal = normalize(normalMatrix * fNormal);
    float diffuse = max(dot(normal, dir1), 0.0f);

    vec3 viewDirection = normalize(cameraPos - fPosition);
    vec3 reflectionDirection = reflect(-dir1, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8);
    float specular = specAmount * specularLight;


    float angle = dot(vec3(0.0f, -1.0f, 0.0f), -dir1);
    float inten = smoothstep(outerCone, innerCone, angle);

    return (texture(diffuseTexture, fTexCoords).rgb * (diffuse * inten + ambient) + texture(specularTexture, fTexCoords).r * specular * inten) * color1;
}

float computeFog()
{

    vec4 fragmentPosEyeSpace = view * model * vec4(fPosition, 1.0f);

    float fragmentDistance = length(fragmentPosEyeSpace);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);

}

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;


}

void main() 
{
    computeDirLight();
    float shadow = computeShadow();

    //compute fog 
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

    vec3 result;

    ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;


    
    if (spot == 1)
     { 
        result = computeSpot();
     } 
    else result = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f); 


     fColor = fogColor * (1 - fogFactor) + vec4(result,1.0f) * fogFactor;
   
}
