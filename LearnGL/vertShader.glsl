#version 460
layout (location=0) in vec3 position;
layout (location=1) in vec2 texCoord;
layout (location=2) in vec3 normals;

out vec2 tc;
out noperspective vec2 affine_tc;



out vec3 varyingNormal;
out vec3 varyingLightDir;
out vec3 varyingVertPos;
out vec3 varyingHalfVector;



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

uniform mat4 inv_bind_matrix;
uniform mat4 transform_matrix;

uniform bool psx_shader;


uniform float tf;


mat4 buildRotateX(float rad); 
mat4 buildRotateY(float rad); 
mat4 buildRotateZ(float rad);
mat4 buildTranslate(float x, float y, float z);


float roundToPrecision(float value, float precis) {
    return round(value * precis) / precis;
}


void main(void)
{

vec4 nrmFour = vec4(normals,1.0);

nrmFour = inv_bind_matrix * nrmFour;

nrmFour = transform_matrix * nrmFour;

varyingVertPos=round(mv_matrix*vec4(position,1.0f)).xyz;
varyingLightDir=pos_light.position-varyingVertPos;
varyingNormal= normalize(mat3(transpose(inverse(mv_matrix))) * nrmFour.xyz);

varyingHalfVector = (varyingLightDir+(-varyingVertPos)).xyz;

vec4 posFour = vec4(position,1.0);

posFour = inv_bind_matrix * posFour;

posFour = transform_matrix * posFour;



vec4 tmpPos = proj_matrix * mv_matrix * posFour;








if(psx_shader){

tmpPos.x=roundToPrecision(tmpPos.x,16);
tmpPos.y=roundToPrecision(tmpPos.y,16);



}

gl_Position = tmpPos;



tc=texCoord;
affine_tc = texCoord;

}

mat4 buildTranslate(float x, float y, float z)
{ mat4 trans = mat4(1.0, 0.0, 0.0, 0.0,
 0.0, 1.0, 0.0, 0.0,
 0.0, 0.0, 1.0, 0.0,
 x, y, z, 1.0 );
return trans;
}

mat4 buildRotateX(float rad)
{ mat4 xrot = mat4(1.0, 0.0, 0.0, 0.0,
 0.0, cos(rad), -sin(rad), 0.0,
 0.0, sin(rad), cos(rad), 0.0,
 0.0, 0.0, 0.0, 1.0 );
return xrot;
}

mat4 buildRotateY(float rad)
{ mat4 yrot = mat4(cos(rad), 0.0, sin(rad), 0.0,
 0.0, 1.0, 0.0, 0.0,
 -sin(rad), 0.0, cos(rad), 0.0,
 0.0, 0.0, 0.0, 1.0 );
return yrot;
}

mat4 buildRotateZ(float rad)
{ mat4 zrot = mat4(cos(rad), -sin(rad), 0.0, 0.0,
 sin(rad), cos(rad), 0.0, 0.0,
 0.0, 0.0, 1.0, 0.0,
 0.0, 0.0, 0.0, 1.0 );
return zrot;
}

mat4 buildScale(float x, float y, float z)
{ mat4 scale = mat4(x, 0.0, 0.0, 0.0,
 0.0, y, 0.0, 0.0,
 0.0, 0.0, z, 0.0,
 0.0, 0.0, 0.0, 1.0 );
return scale;
}