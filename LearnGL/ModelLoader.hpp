#ifndef MODEL_LOADER
#define MODEL_LOADER

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SOIL2/SOIL2.h>
#include <glm/gtc/quaternion.hpp>
#include "CustomImageFormat.hpp"
#include "nlohmann/json.hpp"
#include "Animation.hpp"
#include "FixedPointMath.h"



#define numVAOs 1
#define numVBOs 3
#define numEBOs 1




struct Bone {

	int fromNode;

	int fromJointTab;

	bool extremity;

	std::string name;

	int parent;

	glm::mat4x4 TransformMat;
	transform InitTransform;

	std::vector<int> children;

	std::vector<animJoint> animations;

	Bone()
	{
		fromNode = 0;
		fromJointTab = 0;
		extremity = false;
		name = "NONE";
		parent = 0;
		TransformMat = glm::mat4x4(1.0f);
	}
	

		
};

struct MImeshes {

	std::string name;

	std::vector<float> triangleVerts;
	std::vector<float> textureCoords;
	std::vector<float> normals;
	std::vector<uint16_t> indices;

	int jointIndex;

	MImeshes()
	{
		jointIndex = 0;
		name = "NONE";
	}

};



class ModelImporter {

private:

	

	std::vector<float> vertVals;
	std::vector<float> stVals;
	std::vector<float> normVals;

	std::vector<MImeshes> ImpMeshes;

	
	
	std::vector<Bone> bones;

	std::vector<glm::mat4x4> matrices;

	

	std::vector<uint16_t> indices;


public:

	int root;

	int num_meshes;
	int num_mats;
	int num_joints;
	int num_anims;

	int getBoneIndex(int m);

	nlohmann::json j_son;

	int get_child_index(int j_son_index);

	
	void parseGLTF(const char* filePathRel);
	void GLTF_To_AST(const char* filePathRel, uint8_t scalingFactorBits, uint8_t FPS);
	void OpenAST(const char* filePathRel);

	int findBoneByNode(int node);
	int findBoneByJointVal(int jointVal);
	int getNumVertices(int m);
	int getNumIndices(int m);
	std::vector<float> getVertices(int m);
	std::vector<float> getTextureCoords(int m);
	std::vector<float> getNormals(int m);
	std::vector<uint16_t> getIndices(int m);

	void clearRig();

	std::vector<Bone> getBones();
	std::vector<glm::mat4x4> getInverseBinds();

	


	GLuint* loadRPF(const char* filePathRel);

	void clearData();


	ModelImporter()
	{
		root = 0;
		num_meshes = 0;
		num_mats = 0;
		num_joints = 0;
		num_anims = 0;

		
	}

};

struct Mesh {


	int numVertices;
	int numIndices;

	int jointIndex;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normalVecs;
	std::vector<uint16_t> indices;

	GLuint vao[numVAOs];
	GLuint vbo[numVBOs];
	GLuint ebo[numEBOs];
	

	int getNumVertices();
	int getNumIndices();

	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<glm::vec3> getNormals();
	std::vector<uint16_t>  getIndices();


	Mesh()
	{
		numVertices = 0;
		numIndices = 0;
		jointIndex = 0;

		for (size_t i = 0; i < numVAOs; i++)
		{
			vao[i] = 0;
		}
		for (size_t i = 0; i < numVBOs; i++)
		{
			vbo[i] = 0;
		}
		for (size_t i = 0; i < numEBOs; i++)
		{
			ebo[i] = 0;
		}
	}

};

struct ImportedModel {

	void compute_pose(int boneI);

	glm::mat4x4 compute_transformJ(int boneI);

	std::vector<Mesh> Meshes;

	int num_mats;
	int num_joints;
	int num_anims;

	std::vector< glm::mat4x4> INVmatrices;
	std::vector<Bone> bones;
	
	int root;

	int currentAnim;

	ModelImporter importer;
	glm::vec3 position;

	glm::quat rotation;

	ImportedModel(const char * filePath,const char* Image,glm::vec3 pos, glm::quat rot);
	

	GLuint texture;
	GLuint clut;

	GLuint tex_width;
	GLuint tex_height;

	float clut_multiplier;

	ImportedModel()
	{
		importer = ModelImporter();
		num_joints = 0;
		num_anims = 0;
		position = glm::vec3(0.0,0.0,0.0);
		rotation = glm::quat(0.0,0.0,0.0,0.0);
		root = 0;
		currentAnim = 0;
		texture = 0;
		tex_height = 0;
		tex_width = 0;
		clut = 0;
		num_mats = 0;
		clut_multiplier = 1.0f;
	}
	
};

#endif // !MODEL_LOADER