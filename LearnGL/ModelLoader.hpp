#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <SOIL2/SOIL2.h>
#include <glm/gtc/quaternion.hpp>
#include "CustomImageFormat.hpp"
#include "nlohmann/json.hpp"
#include "Animation.hpp"

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

		
};

struct MImeshes {

	std::string name;

	std::vector<float> triangleVerts;
	std::vector<float> textureCoords;
	std::vector<float> normals;
	std::vector<uint16_t> indices;

	int jointIndex;

};



class ModelImporter {

private:

	

	std::vector<float> vertVals;
	std::vector<float> stVals;
	std::vector<float> normVals;

	std::vector<MImeshes> ImpMeshes;

	
	
	std::vector<Bone> bones;

	std::vector<glm::mat4x4> matrices;

	std::vector<transform> transforms;

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

	
	void parseGLTF(const char* filePath);
	
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




	GLuint loadRPF(const char* filepath);

	void clearData();

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


	std::vector<float>pvalues;
	std::vector<float>tvalues;
	std::vector<float>nvalues;
	

	int getNumVertices();
	int getNumIndices();

	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<glm::vec3> getNormals();
	std::vector<uint16_t>  getIndices();


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
	
	
};

