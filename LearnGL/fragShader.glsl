#version 430
in vec2 tc;

in vec3 varyingNormal;
in vec3 varyingLightDir;
in vec3 varyingVertPos;
in vec3 varyingHalfVector;

out vec4 color;

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


layout (binding=0) uniform sampler2D samp;

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

vec4 simple_color=texture(samp,tc);

vec3 alpha_clip=vec3(1.0,0.0,1.0);

	if(all(lessThanEqual(abs(simple_color.rgb-alpha_clip),vec3(0.5)))){

		discard;

	}


	
color=simple_color*light;
	


}