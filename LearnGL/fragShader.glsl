#version 430
in vec2 tc;



in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;


out vec4 color;

const mat3 bayerMatrix = mat3(
    vec3(1.0 / 9.0, 3.0 / 9.0, 5.0 / 9.0),
    vec3(7.0 / 9.0, 8.0 / 9.0, 1.0 / 9.0),
    vec3(3.0 / 9.0, 5.0 / 9.0, 7.0 / 9.0)
);




float dither (vec2 position){
int dith_x = int(mod(int(position.x*64),3));
int dith_y = int(mod(int(position.y*64),3));
return bayerMatrix[dith_x][dith_y];
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

float clr_ind = texture(samp,tc).r * clut_multiplier;

vec4 simple_color=texture(cltsamp,clr_ind);




	if(simple_color.a==0.0){

		discard;

	}
	else{

		float dith = dither(tc);

		
		simple_color-=dith/4;
		

	}

	color=simple_color*light;
	
	
}