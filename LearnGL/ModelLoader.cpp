#include <fstream>
#include <sstream>
#include <iostream>
#include "ModelLoader.hpp"



#define ROOT_MARKER -1



ImportedModel::~ImportedModel() {
	
	importer.clearData();
	importer.clearRig();
	GLuint tDel[2] = { texture,clut };
	glDeleteTextures(1,tDel);
		
	delete this->material;


	for (Mesh & msh : Meshes)
	{
		msh.vertices.clear();
		msh.indices.clear();
		msh.texCoords.clear();
		msh.normalVecs.clear();

		glDeleteBuffers(numVAOs, msh.vao);
		glDeleteBuffers(numVBOs, msh.vbo);
		glDeleteBuffers(numEBOs, msh.ebo);
	}

	for (Bone & bone : bones)
	{
		for (animJoint& anim : bone.animations) {

			anim.transTimes.clear();
			anim.rotTimes.clear();
			anim.scalTimes.clear();

			anim.translations.clear();
			anim.rotations.clear();
			anim.scales.clear();

		}

		bone.animations.clear();
		bone.children.clear();
	}

}

ImportedModel::ImportedModel(const char* filePath) {



	clut_multiplier = 1.0f;

	currentAnim = 0;

	root = -1;


	GLuint* temptext = importer.loadRPF(filePath);

	texture = temptext[0];
	clut = temptext[1];


	unsigned int Mode = temptext[2];

	tex_width = temptext[3];
	tex_height = temptext[4];

	clut_multiplier = 256.0f / (float)Mode;

	delete[] temptext;


	importer.OpenAST(filePath);
	

	this->material = new Material(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
		glm::vec4(0.25f, 0.25f, 0.25f, 1.0f),
		0.1f, 0.0f, 0.0f);
	
	

	num_joints = importer.num_joints;
	num_mats = importer.num_mats;
	num_anims = importer.num_anims;

	if (num_joints > 0) {
		root = importer.root;

		bones = importer.getBones();

	}

	if (num_mats > 0) {
		INVmatrices = importer.getInverseBinds();
	}

	for (int i = 0; i < importer.num_meshes; i++)
	{
		Mesh mtmp;

		mtmp.numVertices = (int)importer.getNumVertices(i);
		mtmp.numIndices = (int)importer.getNumIndices(i);

		std::vector<float>verts = importer.getVertices(i);
		std::vector<float>tex = importer.getTextureCoords(i);
		std::vector<float>norms = importer.getNormals(i);

		for (int i = 0; i < mtmp.numVertices; i++)
		{
			mtmp.vertices.push_back(glm::vec3(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]));
			mtmp.texCoords.push_back(glm::vec2(tex[i * 2], tex[i * 2 + 1]));
			mtmp.normalVecs.push_back(glm::vec3(norms[i * 3], norms[i * 3 + 1], norms[i * 3 + 2]));

		}
		mtmp.jointIndex=importer.getBoneIndex(i);
		mtmp.indices = importer.getIndices(i);

		Meshes.push_back(mtmp);

		
	}


	importer.clearData();
	importer.clearRig();
	

}



int Mesh::getNumVertices() { return numVertices; }
int Mesh::getNumIndices() { return numIndices; }
std::vector<glm::vec3> Mesh::getVertices() { return vertices; }
std::vector<glm::vec2>Mesh::getTexCoords() { return texCoords; }
std::vector<glm::vec3>Mesh::getNormals() { return normalVecs; }
std::vector<uint16_t> Mesh::getIndices() { return indices; }





void ModelImporter::OpenAST(const char* filePathRel) {

	fix tempFix;
	uint8_t tempByte;
	uint16_t tempShort;
	float tempFloat;
	MImesh tempMesh;
	Bone tempBone;
	animJoint tempAnim;
	glm::vec4 tempVec;

	const std::string Path = "Assets/" + (std::string)filePathRel + ".AST";
	

	std::ifstream astStream;

	astStream.open(Path,std::ios::binary);
	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t scalingFactor = tempByte;

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t x_beg=tempByte;

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t y_beg = tempByte;

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t x_end = tempByte;

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t y_end = tempByte;

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));


	uint16_t x_diff = (uint16_t)(x_end - x_beg) + 1;
	uint16_t y_diff = (uint16_t)(y_end - y_beg) + 1;

	uint8_t mshCount = tempByte;
	num_meshes = mshCount;	

	for (uint8_t i = 0; i < mshCount; i++)
	{
		

		astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
		tempMesh.jointIndex = (int)tempByte;
		astStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
		
		for (uint16_t j = 0; j < tempShort; j++)
		{
			astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFloat = FIX_TO_F(tempFix,scalingFactor);

			tempMesh.triangleVerts.push_back(tempFloat);
		}
		ImpMeshes.push_back(tempMesh);

		tempMesh.triangleVerts.clear();

		astStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
		uint16_t indCount = tempShort;

		for (uint16_t j = 0; j < indCount; j++)
		{
			astStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
			tempMesh.indices.push_back(tempShort);


		}

		ImpMeshes[i].indices.clear();
		ImpMeshes[i].indices = tempMesh.indices;

		tempMesh.indices.clear();

		astStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));

		for (uint16_t j = 0; j < tempShort; j += 2)
		{
			astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));


			tempFloat = ((float(tempByte) * ((float)x_diff / (float)(x_diff - 1))) / (float)(x_diff));


			tempMesh.textureCoords.push_back(tempFloat);

			astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			tempFloat = ((float(tempByte) * ((float)y_diff / (float)(y_diff - 1))) / (float)(y_diff));

			tempMesh.textureCoords.push_back(tempFloat);

		}


		ImpMeshes[i].textureCoords.clear();
		ImpMeshes[i].textureCoords = tempMesh.textureCoords;

		tempMesh.textureCoords.clear();

		astStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));

		for (uint16_t j = 0; j < tempShort; j++)
		{

			astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFloat = FIX_TO_F(tempFix, scalingFactor);

			tempMesh.normals.push_back(tempFloat);

		}


		ImpMeshes[i].normals.clear();
		ImpMeshes[i].normals = tempMesh.normals;

		tempMesh.normals.clear();
	}

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t boneCount = tempByte;

	num_joints = boneCount;

	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	root = (int)tempByte;


	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t animCount = tempByte;

	num_anims = animCount;
	
	astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t targetFPS = tempByte;

	for (uint8_t i = 0; i < boneCount; i++)
	{
		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.translation.x = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.translation.y = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.translation.z = FIX_TO_F(tempFix, scalingFactor);


		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.x = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.y = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.z = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.w = FIX_TO_F(tempFix, scalingFactor);


		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.scale.x = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.scale.y = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.scale.z = FIX_TO_F(tempFix, scalingFactor);

		astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

		mshCount = tempByte;

		if (mshCount == 0) {
			tempBone.extremity = true;
		}

		for (uint8_t j = 0; j < mshCount; j++)
		{
			astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			tempBone.children.push_back((int)tempByte);
		}

		tempBone.TransformMat = tempBone.InitTransform.Matrix();
		

		bones.push_back(tempBone);
		

		tempBone.children.clear();

		

		for (uint8_t j = 0; j < animCount; j++) {

			astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			uint8_t TRSCount = tempByte;

			for (uint8_t k = 0; k < TRSCount; k++)
			{
				astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

				tempFloat = (float(tempByte)) / (float(targetFPS));

				if (k > 0) {

					while (tempFloat < tempAnim.transTimes[k - 1])
					{
						tempFloat += 1.0f;
					}

				}

				tempAnim.transTimes.push_back(tempFloat);

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.x = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.y = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.z = tempFloat;

				tempAnim.translations.push_back(glm::vec3(tempVec.x, tempVec.y, tempVec.z));

			}


			astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			TRSCount = tempByte;

			for (uint8_t k = 0; k < TRSCount; k++)
			{
				astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

				tempFloat = (float(tempByte)) / (float(targetFPS));

				if (k > 0) {

					while (tempFloat < tempAnim.rotTimes[k - 1])
					{
						tempFloat += 1.0f;
					}

				}

				tempAnim.rotTimes.push_back(tempFloat);

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.x = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.y = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.z = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.w = tempFloat;

				tempAnim.rotations.push_back(glm::normalize(glm::quat(tempVec.w, tempVec.x, tempVec.y, tempVec.z)));

			}

			astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			TRSCount = tempByte;

			for (uint8_t k = 0; k < TRSCount; k++)
			{
				astStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

				tempFloat = (float(tempByte)) / (float(targetFPS));

				if (k > 0) {

					while (tempFloat < tempAnim.scalTimes[k - 1])
					{
						tempFloat += 1.0f;
					}

				}

				tempAnim.scalTimes.push_back(tempFloat);

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.x = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.y = tempFloat;

				astStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.z = tempFloat;

				tempAnim.scales.push_back(glm::vec3(tempVec.x, tempVec.y, tempVec.z));

			}

			bones[i].animations.push_back(tempAnim);




			tempAnim.translations.clear();
			tempAnim.rotations.clear();
			tempAnim.scales.clear();

			tempAnim.transTimes.clear();
			tempAnim.rotTimes.clear();
			tempAnim.scalTimes.clear();



		}
	}	
	

	astStream.close();
	
}

void ImportedModel::compute_pose(int boneI, glm::mat4x4 Parent_T) {

	bones[boneI].TransformMat = Parent_T * bones[boneI].TransformMat;

	int proc = (int)bones[boneI].children.size();

	if (proc == 0)
	{
		return;
	}

	else
	{
		for (int i = 0; i < proc; i++)
		{
			compute_pose(bones[boneI].children[i],bones[boneI].TransformMat);
		}
	}

}



GLuint* ModelImporter::loadRPF(const char* filePathRel) {

	std::string filePath = "Assets/" + (std::string)filePathRel +".RPF";

	std::ifstream rpfloader(filePath, std::ios::binary);

	RPF rpf;

	GLuint Mode = 0;

	rpfloader.read(reinterpret_cast<char*>(&rpf.magic), sizeof(rpf.magic));

	rpf.CLUT = new uint16_t[(int)(rpf.magic[1] + 1)];	


	rpf.data = new uint8_t[((int)rpf.magic[2] + 1) * ((int)rpf.magic[3] + 1)];

	uint16_t reader = 0;

	

	for (int i = 0; i < ((int)rpf.magic[1] + 1); i++)
	{
		rpfloader.read(reinterpret_cast<char*>(&reader), 1 * sizeof(uint16_t));

		rpf.CLUT[i] = reader;		
		
		

	}

	


	if ((rpf.magic[1] + 1) > 16)
	{
		rpfloader.read(reinterpret_cast<char*>(rpf.data), sizeof(char) * ((int)rpf.magic[2] + 1) * ((int)rpf.magic[3] + 1));


	}

	else if ((rpf.magic[1] + 1) <= 16 && (rpf.magic[1]+1)>4) {
		uint8_t msb = 0;
		uint8_t lsb = 0;

		

		uint8_t temp_data = 0;

		for (int i = 0; i < (int)((((int)rpf.magic[2] + 1) * ((int)rpf.magic[3] + 1)) / 2); i++)
		{
			rpfloader.read(reinterpret_cast<char*>(&temp_data), sizeof(char));

			msb = (uint8_t)(temp_data >> 4) & 0x0F;
			lsb = (uint8_t)(temp_data) & 0x0F;
			

			rpf.data[(2 * i)] = msb;
			rpf.data[(2 * i) + 1] = lsb;

		}

		
	}

	else if ((rpf.magic[1] + 1) <= 4 && (rpf.magic[1] + 1) > 2)
	{

		uint8_t msb = 0;
		uint8_t hsb = 0;
		uint8_t rsb = 0;
		uint8_t lsb = 0;

		uint8_t temp_data = 0;

		for (int i = 0; i < (int)((((int)rpf.magic[2] + 1) * ((int)rpf.magic[3] + 1)) / 4); i++)
		{
			rpfloader.read(reinterpret_cast<char*>(&temp_data), sizeof(char));

			msb = (uint8_t)(temp_data >> 6) & 0x03;
			hsb = (uint8_t)(temp_data >> 4) & 0x03;
			rsb = (uint8_t)(temp_data >> 2) & 0x03;
			lsb = (uint8_t)(temp_data) & 0x03;


			rpf.data[(4 * i)] = msb;
			rpf.data[(4 * i) + 1] = hsb;
			rpf.data[(4 * i) + 2] = rsb;
			rpf.data[(4 * i) + 3] = lsb;

		}


	

	}

	else
	{
		uint8_t msb = 0;
		uint8_t asb = 0;
		uint8_t bsb = 0;
		uint8_t csb = 0;
		uint8_t dsb = 0;
		uint8_t esb = 0;
		uint8_t fsb = 0;
		uint8_t lsb = 0;


		uint8_t temp_data = 0;

		for (int i = 0; i < (int)((((int)rpf.magic[2] + 1) * ((int)rpf.magic[3] + 1)) / 8); i++)
		{
			rpfloader.read(reinterpret_cast<char*>(&temp_data), sizeof(char));


			msb = (uint8_t)(temp_data >> 7) & 0x01;
			asb = (uint8_t)(temp_data >> 6) & 0x01;
			bsb = (uint8_t)(temp_data >> 5) & 0x01;
			csb = (uint8_t)(temp_data >> 4) & 0x01;
			dsb = (uint8_t)(temp_data >> 3) & 0x01;
			esb = (uint8_t)(temp_data >> 2) & 0x01;
			fsb = (uint8_t)(temp_data >> 1) & 0x01;
			lsb = (uint8_t)(temp_data) & 0x01;


			rpf.data[(8 * i)] = msb ;
			rpf.data[(8 * i) + 1] = asb;
			rpf.data[(8 * i) + 2] = bsb;
			rpf.data[(8 * i) + 3] = csb;
			rpf.data[(8 * i) + 4] = dsb;
			rpf.data[(8 * i) + 5] = esb;
			rpf.data[(8 * i) + 6] = fsb;
			rpf.data[(8 * i) + 7] = lsb;

		}

	

		
	}





	rpfloader.close();



	GLuint text;

	int tempc = (int)rpf.magic[1] + 1;

	int tempw = (int)rpf.magic[2] + 1;

	int temph = (int)rpf.magic[3] + 1;

	Mode = (GLuint)rpf.magic[1]+1;


	int temp = tempw * temph;


	glGenTextures(1, &text);

	glBindTexture(GL_TEXTURE_2D, text);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tempw, temph, 0, GL_RED, GL_UNSIGNED_BYTE, rpf.data);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	GLuint clt;

	glGenTextures(1, &clt);

	glBindTexture(GL_TEXTURE_1D, clt);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, tempc, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,rpf.CLUT);
	

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);


	GLuint* rtn = new GLuint[5];

	rtn[0] = text;
	rtn[1] = clt;
	rtn[2] = Mode;
	rtn[3] = (GLuint)rpf.magic[2] + 1;
	rtn[4] = (GLuint)rpf.magic[3] + 1;

	

	delete[] rpf.CLUT;

	delete[] rpf.data;

	

	

	return rtn;



}



void ModelImporter::clearData() {

	vertVals.clear();
	stVals.clear();
	normVals.clear();
	


	indices.clear();
}



int ModelImporter::get_child_index(int j_son_index) {

	for (int i = 0; i < num_joints; i++)
	{

		if (bones[i].fromNode==j_son_index) {

			
			return i;

		}

	}


	return -1;
}

void ModelImporter::clearRig() {

	
	bones.clear();
	matrices.clear();

	clearData();

	ImpMeshes.clear();
}


std::vector<Bone> ModelImporter::getBones() {

	return bones;


}

std::vector<glm::mat4x4> ModelImporter::getInverseBinds() {

	return matrices;


}

int ModelImporter::getBoneIndex(int m) {

	return ImpMeshes[m].jointIndex;

}

int ModelImporter::getNumVertices(int m) { return ((int)ImpMeshes[m].triangleVerts.size() / 3); }
int ModelImporter::getNumIndices(int m) { return((int)ImpMeshes[m].indices.size()); }
std::vector<float> ModelImporter::getVertices(int m) { return ImpMeshes[m].triangleVerts; }
std::vector<float> ModelImporter::getTextureCoords(int m) { return ImpMeshes[m].textureCoords; }
std::vector<float> ModelImporter::getNormals(int m) { return ImpMeshes[m].normals; }
std::vector<uint16_t> ModelImporter::getIndices(int m) { return ImpMeshes[m].indices; }

int ModelImporter::findBoneByJointVal(int jointVal) {

	for (int i = 0; i < num_joints; i++)
	{
		if ((int)bones[i].fromJointTab == jointVal) {


			return i;
		}


	}

	return -1;
}

int ModelImporter::findBoneByNode(int node) {

	for (int i = 0; i < num_joints; i++)
	{

		if ((int)bones[i].fromNode == node) {

			return i;
		}



	}



	return -1;
}
