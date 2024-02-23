#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
//in vec4 fragPosLightSpace;
out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform mat4 projection;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec4 ambient;
float ambientStrength = 0.2f;
vec4 diffuse;
vec4 specular;
float specularStrength = 0.5f;

//iluminare
float constant = 1.0f;
float linear = 0.10f;
float quadratic = 0.05f;
vec3 pozitieLumina1;
vec3 culoareLumina1;

//punctiform
vec3 normal;
uniform vec3 lightPunctiform1;
uniform vec3 lightPunctiform2;

//fog
uniform float lumina;
uniform float fogDensity;

//punctiform
uniform bool punctiform = false;

//street light
uniform vec3 spotLight2;
uniform vec3 target2;
vec3 lightDir2;
vec3 spotDir;
float cutOff;
float cutOff2;
float unghi;
float epsilon;
float intensitate;
vec3 reflectDirection;
float coeff;
vec4 rezultat;

//umbra
//uniform sampler2D shadowMap;

vec4 computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = vec4(ambientStrength * lightColor, 1.0f);

    //compute diffuse light
    diffuse = vec4((max(dot(normalEye, lightDirN), 0.0f) * lightColor), 1.0f);

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = vec4((specularStrength * specCoeff * lightColor), 1.0f);
    return min((ambient + diffuse) * texture(diffuseTexture, fTexCoords) + specular * texture(specularTexture, fTexCoords), 1.0f);
}

float computeFog()
{
 float fragmentDistance = length(fPosition);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 return clamp(fogFactor, 0.0f, 1.0f);
}

//street light()
vec4 computeLight1(vec3 spotLight, vec3 target){
    culoareLumina1 = vec3(1.0f, 1.0f, 1.0f);
    spotDir = normalize(spotLight - target);
    lightDir2 = normalize(spotLight - fPosition);
    cutOff = radians(55.0f);
    cutOff2 = radians(40.0f);
    unghi = dot(lightDir2, normalize(spotDir));
    epsilon = cutOff - cutOff2;
    intensitate = clamp((unghi - cutOff2) / (epsilon*2.0f), 0.0f, 1.0f);
    ambient = 0.00001 * vec4(1.0f, 1.0f, 1.0f, 1.0f);
    diffuse = (max(dot(lightDir2, normalize(spotDir)), 0.0f) * vec4(1.0f, 1.0f, 1.0f, 1.0f)) * intensitate;
    reflectDirection = reflect(-lightDir2, spotDir);
    coeff = pow(max(dot(lightDir2, normalize(spotDir)), 0.0f), 32);
    specular = vec4((specularStrength * coeff * culoareLumina1), 1.0f) * intensitate;

    return (min((ambient + diffuse) * texture(diffuseTexture, fTexCoords) + specular * texture(specularTexture, fTexCoords), 1.0f));
}

//punctiform
vec4 computeLight2(vec3 lightPunctiform){
    normal = normalize(normalMatrix * fNormal);
    culoareLumina1 = vec3(1.0f, 1.0f, 1.0f);
    vec3 lightDirN = normalize(lightPunctiform - fPosition);
    float constant = 1.0f;
    float linear = 0.01f;
    float quadratic = 0.02f;
    float dist = length(lightPunctiform - fPosition);
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
    ambient = vec4( (att * ambientStrength * culoareLumina1), 1.0);
    diffuse = vec4( att * (max(dot(normal, lightDirN), 0.0f) * culoareLumina1), 1.0f);
    vec3 viewDirection = normalize(-fPosition);
    reflectDirection = reflect(-lightDirN, normal);
    specular = vec4((att * specularStrength * culoareLumina1), 1.0f);
    return min((ambient + diffuse) * texture(diffuseTexture, fTexCoords)+specular*texture(specularTexture, fTexCoords), 1.0f);
}

/*
float computeShadow(){
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (normalizedCoords.z > 1.0f)
        return 0.0f;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    float bias = 0.005f;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    return shadow;
}
*/
void main() 
{
   // computeDirLight();

    //compute final vertex color
    vec4 color;
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    if(lumina == 1.0f)
        color = computeLight1(spotLight2, target2) + computeLight2(lightPunctiform1) + computeLight2(lightPunctiform2);
    else
       color = computeDirLight();
    fColor = mix(fogColor, color, fogFactor);

  //  float shadow = computeShadow();

}
