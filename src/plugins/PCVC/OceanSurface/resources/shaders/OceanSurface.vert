#version 430

uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texCoords;

uniform sampler2D dispX;
uniform sampler2D dispY;
uniform sampler2D dispZ;

uniform sampler2D normalMap;

uniform float choppiness;
uniform float waveHeight;

out vec3 worldPos;
out vec3 normal;


void main() {

    float height = in_position.y + waveHeight * texture(dispY, in_texCoords).r;
    float xPos = in_position.x - texture(dispX, in_texCoords).r * choppiness;
    float zPos = in_position.z - texture(dispZ, in_texCoords).r * choppiness;

    vec3 normalVec = texture(normalMap, in_texCoords).rgb;

    // apply model transform to normals (Local to World) but remove translate and apply only scale and rotation 
    normal = mat3(transpose(inverse(modelMx))) * normalVec;
    worldPos =  vec3(modelMx * vec4(xPos, height, zPos, 1.0));
   
    gl_Position = projMx * viewMx * modelMx * vec4(xPos, height, zPos, 1.0);

    // geom shader version
    //texCoords= in_texCoords;
    //gl_Position = vec4(xPos, waveHeight * height, zPos, 1.0);
}
