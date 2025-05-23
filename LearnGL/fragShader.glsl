#version 460
in vec2 tc;
noperspective in vec2 affine_tc;


in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;


out vec4 color;

const float[8][8] bayerMatrix ={
    { 0, 32, 8, 40, 2, 34, 10, 42   },
    {48, 16, 56, 24, 50, 18, 58, 26 },
    {12, 44, 4, 36, 14, 46, 6, 38   },
    {60, 28, 52, 20, 62, 30, 54, 22 },
    { 3, 35, 11, 43, 1, 33, 9, 41   },
    {51, 19, 59, 27, 49, 17, 57, 25 },
    {15, 47, 7, 39, 13, 45, 5, 37   },
    {63, 31, 55, 23, 61, 29, 53, 21 } 
};



uniform uint tex_width;
uniform uint tex_height;

uniform bool psx_shader;

float dither (vec2 position){
int dith_x = int(mod(int(position.x*tex_width),8));
int dith_y = int(mod(int(position.y*tex_height),8));
return bayerMatrix[dith_x][dith_y]/64;
};



struct GlobalLight{
vec4 ambient;
};
struct PositionalLight{
vec4 ambient;
vec4 diffuse;
vec4 specular;
vec3 position;
};
struct DirectionalLight{
vec4 ambient;
vec4 diffuse;
vec4 specular;
vec3 direction;
};

struct Material{
vec4 ambient;
vec4 diffuse;
vec4 specular;
float shininess;
};


uniform GlobalLight glo_light;
uniform PositionalLight pos_light;
uniform Material material;

uniform mat4 v_matrix;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform float tf;

uniform float clut_multiplier;



layout (binding=0) uniform sampler2D samp;
layout(binding=1) uniform sampler1D cltsamp;

void main(void){




vec3 L = normalize(varyingLightDir);
vec3 N =normalize(varyingNormal);
vec3 V = normalize(-varyingVertPos);
vec3 H = normalize(varyingHalfVector);

vec3 R = normalize(reflect(-L,N));
float cosTheta= dot(L,N);
float cosPhi = dot(H,N);

vec3 ambient=((glo_light.ambient*material.ambient)+(pos_light.ambient*material.ambient)).xyz;
vec3 diffuse=pos_light.diffuse.xyz*material.diffuse.xyz*max(cosTheta,0.0f);
vec3 specular=pos_light.specular.xyz*material.specular.xyz*pow(max(cosPhi,0.0f),material.shininess*3.0);

vec4 light = vec4((ambient+diffuse+specular),1.0f);

float clr_ind = texture(samp,affine_tc).r * clut_multiplier;



vec4 simple_color=texture(cltsamp,clr_ind);


if(simple_color.a==0.0){

		discard;

}

if(psx_shader){


	
		float lum = (simple_color.r+simple_color.g+simple_color.b)/3.0;

		float dith = dither(tc)*(lum/7.5);

		float sat = 0.5;

		
		simple_color-=dith;

		if(simple_color.r>lum){
			simple_color.r -= (simple_color.r-lum)*sat;
		}

		else if(simple_color.r<lum){
			simple_color.r += (simple_color.r-lum)*sat;
		}

		if(simple_color.g>lum){
			simple_color.g -= (simple_color.r-lum)*sat;
		}
		else if(simple_color.g<lum){
			simple_color.g += (simple_color.r-lum)*sat;
		}

		if(simple_color.b>lum){
			simple_color.b -= (simple_color.r-lum)*sat;
		}

		else if(simple_color.b<lum){
			simple_color.b += (simple_color.r-lum)*sat;
		}

		

	

}

color=simple_color*light;

	
}