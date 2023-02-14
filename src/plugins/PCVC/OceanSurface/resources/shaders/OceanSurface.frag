#version 430

layout(location = 0) out vec4 fragColor;

layout(std430, binding = 1) buffer data{
     float maxMin[];
};

in vec3 worldPos;
in vec3 normal;

uniform samplerCube skybox;

uniform mat4 invViewMx;
uniform vec3 camPos;
uniform vec3 lightDir; // directional light

// each vertex, I = I_amb + I_diff + I_spec + I_refl + I_refrac

vec3 reflection(vec3 N, vec3 I){ // same as built-in function
    return vec3(I - 2.0 * dot(N, I) * N);
} 

vec3 refraction(vec3 I, vec3 N){ // same as built-in function

    float cosi = clamp(dot(I, N), -1.0, 1.0);
    float etai = 1.0; // air
    float etat = 1.33; // refractive index for water is 1.33

    if(cosi < 0.0) cosi = -cosi;
    else {
        etai = 1.33;
        etat = 1.0;
    }
    float eta = etai / etat;
    float k = 1.0 - eta * eta*(1.0 - cosi * cosi);

    if(k < 0.0) return vec3(0.0);
    else return vec3(eta*I + (eta * cosi - sqrt(k)) * N);
}

//fresnel equation from the paper Synthesis of Ocean Wave Models and Simulation Using GPU, D.Lee and S.Lee
float fresnel(vec3 N, vec3 V, float F0){

    float cosTheta = max(dot(N, V), -dot(N,V));
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


// water is both reflective and refractive, so the amount is computed with Fresnel equations
// returns the amount of reflection, so the amount of refraction can be achieved by 1 - return value
float fresnelFull(float thethaI, float thethaT){

    float waterIOR = 1.334;
    if(thethaI < 0.00001){
        float at0 = (waterIOR - 1.0) / (waterIOR + 1.0);
        return at0 * at0;
    }

    float t1 = sin(thethaI-thethaT) / sin(thethaI+thethaT);
    float t2 = tan(thethaI-thethaT) / tan(thethaI+thethaT);
    return 0.5 * (t1*t1 + t2*t2);
}


void main() {

    //vec3 oceanColor =  vec3(0.04, 0.19, 0.34);

    vec3 deepOcean = vec3(0.02, 0.13, 0.21);
    vec3 shallowOcean = vec3(0.05, 0.36, 0.58);
    float relativeH = clamp((worldPos.y - maxMin[1]) / (maxMin[0] - maxMin[1]), 0.0, 1.0);
    vec3 oceanColor = (relativeH*shallowOcean) + ((1.0-relativeH)*deepOcean);

    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - worldPos); // view direction (direction to camera)
    vec3 I = -V; // reverse of view dir is the incident ray dir
    vec3 R = reflect(-V, N); 
    // global reflection is achieved by using the reflection vector as the texture coordinate to skybox texture
    vec3 sky = texture(skybox, R).rgb;
   
    // fresnel test1
    float F = fresnel(N, V, 0.6);
    vec3 albedo = mix(sky, oceanColor, F);
    //vec3 color = max(dot(N, lightDir), 0.0) * albedo;

    // fresnel test2
    vec3 refracted = refract(-lightDir,N, 1.0/1.33);
    float fresnelfull =  fresnelFull(dot(N, V), dot(-N,refracted));
    vec3 color = mix(sky, oceanColor, 1.0-fresnelfull); // sky * fresnell + oceanColor *(1-fresnellfull)
    
    /* additional lighting settings but commented out */
    // HDR tonemapping to transform points to range [0,1]
    //color = color / (color + vec3(1.0));
    // HDR exposure
    //color =  1.0 - exp(-color * 2.0);
    // gamma correction to match human-perceived brightness
    //color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0f);
   
}
