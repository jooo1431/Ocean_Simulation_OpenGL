#version 430

layout(location = 0) in vec3 in_position;

uniform mat4 projMx;
uniform mat4 viewMx;

out vec3 texCoords;

void main(){
        // local cube coordinate centered on origin is also the direction vector from the origin, thus used as texcoords for the 3D cubemap
        texCoords = in_position;

        vec4 pos = projMx * viewMx * vec4(in_position, 1.0);
        gl_Position = pos.xyww;
        // set z to be 1.0 after perspective division to trick depth buffer that the skybox has max depth of 1.0
        // so it fails the depth test whenever a object is infront of it
}




