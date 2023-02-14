#version 430

layout(location = 0) out vec4 fragColor;

in vec3 texCoords;

uniform samplerCube skybox;

void main(){

    fragColor = texture(skybox, texCoords);

}




