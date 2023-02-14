/*
    Geometry Shader 
*/
#version 430
#define M_PI 3.1415926535897932384626433832795


layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

uniform mat4 projMx;
uniform mat4 viewMx;
uniform mat4 modelMx;

in vec2 texCoords_[3];

/*in VS_OUT {
    vec2 texCoords;
} gs_in[];*/

out vec2 texCoords;
out vec3 worldPos;
out vec3 normal;

void emit(int i, vec3 normalInp){
     worldPos = vec3(modelMx *  gl_in[i].gl_Position);
     gl_Position = projMx * viewMx * modelMx *  gl_in[i].gl_Position;
     texCoords = texCoords_[i];
     //texCoords = gs_in[0].texCoords;
     normal = normalInp;
     EmitVertex();

}


void main(void){

    vec3 v1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 v2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

    vec3 normal_ = cross(v2,v1);

    emit(0, normal_);
    emit(1, normal_);
    emit(2, normal_);

    EndPrimitive();
    
}




