﻿#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <SOIL2/SOIL2.h>
#include <iostream>
#include "ModelLoader.hpp"
#include "Utils.hpp"
#include <glm/gtc/quaternion.hpp>
#include "CustomImageFormat.hpp"
#include "nlohmann/json.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include "Animation.hpp"


#define ROOT_MARKER -1




ImportedModel::ImportedModel(const char* filePath, const char* ImagePath, glm::vec3 pos, glm::quat rot) {

	position = pos;
	rotation = rot;

	clut_multiplier = 1.0f;

	currentAnim = 0;

	root = -1;

	importer.parseGLTF(filePath);

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

	

	GLuint* temptext = importer.loadRPF(ImagePath);

	texture = temptext[0];
	clut = temptext[1];


	unsigned int Mode = temptext[2];

	tex_width = temptext[3];
	tex_height = temptext[4];


	clut_multiplier = 256.0f / (float)Mode;

	

	delete[] temptext;


	importer.clearData();

}



int Mesh::getNumVertices() { return numVertices; }
int Mesh::getNumIndices() { return numIndices; }
std::vector<glm::vec3> Mesh::getVertices() { return vertices; }
std::vector<glm::vec2>Mesh::getTexCoords() { return texCoords; }
std::vector<glm::vec3>Mesh::getNormals() { return normalVecs; }
std::vector<uint16_t> Mesh::getIndices() { return indices; }



void ModelImporter::parseGLTF(const char* filePathRel) {

	num_meshes = 0;
	num_mats = 0;
	num_joints = 0;
	num_anims = 0;

	float temp_float=0.0f;
	uint16_t temp_ushort=0;
	uint8_t temp_ubyte=0;

	const std::string filePath = "Assets/" + (std::string)filePathRel;

	std::ifstream gltf_file(filePath);

	std::string json_string((std::istreambuf_iterator<char>(gltf_file)), std::istreambuf_iterator<char>());

	gltf_file.close();



	j_son = nlohmann::json::parse(json_string);

	num_meshes = int(j_son["meshes"].size());


	std::string bin_file_path = j_son["buffers"][0]["uri"];
	bin_file_path = "Assets/" + bin_file_path;

	std::ifstream bin_file(bin_file_path, std::ios::binary);


	if (j_son["skins"][0]["inverseBindMatrices"].is_number_integer()) {


		int invBind = j_son["skins"][0]["inverseBindMatrices"];
		int invBindv = j_son["accessors"][invBind]["bufferView"];
		int invBindOff = j_son["bufferViews"][invBindv]["byteOffset"];

		num_mats = j_son["accessors"][invBind]["count"];



		bin_file.seekg(invBindOff, std::ios::beg);
		for (int i = 0; i < num_mats; i++)
		{
			glm::mat4x4 mattemp = glm::mat4x4(1.0f);

			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					mattemp[j][k] = temp_float;


				}
			}



			matrices.push_back(mattemp);


		}

	}
	


	if (j_son["skins"][0]["joints"].is_array()) {

		num_joints = (int)j_son["skins"][0]["joints"].size();


		for (int i = 0; i < num_joints; i++)
		{
			Bone tempbone;

			tempbone.fromJointTab = i;

			tempbone.parent = ROOT_MARKER;

			glm::vec3 transl = glm::vec3(0.0f, 0.0f, 0.0f);
			glm::vec3 scal = glm::vec3(1.0f, 1.0f, 1.0f);
			glm::quat rot = glm::quat(0.0f, 0.0f, 1.0f, 0.0f);

			int indexJ = j_son["skins"][0]["joints"][i];

			

			tempbone.fromNode = indexJ;

			glm::mat4 scalMat(1.0f);
			glm::mat4 rotMat(1.0f);
			glm::mat4 transMat(1.0f);

			tempbone.name = j_son["nodes"][indexJ]["name"];

			

			if (j_son["nodes"][indexJ]["translation"].is_array()) {

				transl.x = j_son["nodes"][indexJ]["translation"][0];
				transl.y = j_son["nodes"][indexJ]["translation"][1];
				transl.z = j_son["nodes"][indexJ]["translation"][2];

				tempbone.InitTransform.translation = transl;


			}


			if (j_son["nodes"][indexJ]["rotation"].is_array()) {

				rot.x = j_son["nodes"][indexJ]["rotation"][0];
				rot.y = j_son["nodes"][indexJ]["rotation"][1];
				rot.z = j_son["nodes"][indexJ]["rotation"][2];
				rot.w = j_son["nodes"][indexJ]["rotation"][3];

				tempbone.InitTransform.rotation = rot;


			}


			if (j_son["nodes"][indexJ]["scale"].is_array()) {

				scal.x = j_son["nodes"][indexJ]["scale"][0];
				scal.y = j_son["nodes"][indexJ]["scale"][1];
				scal.z = j_son["nodes"][indexJ]["scale"][2];

				tempbone.InitTransform.scale = scal;


			}




			if (j_son["nodes"][indexJ]["children"].is_array()) {

				tempbone.extremity = false;

				for (int j = 0; j < j_son["nodes"][indexJ]["children"].size(); j++)
				{
					tempbone.children.push_back(j_son["nodes"][indexJ]["children"][j]);

				}


			}

			else
			{
				tempbone.extremity = true;
			}

			

		    tempbone.TransformMat = Utils::transToMat(tempbone.InitTransform);		 
			
			

			bones.push_back(tempbone);




		}

	}



	for (int j = 0; j < num_joints; j++)
	{


		int cnt = (int)bones[j].children.size();




		for (int k = 0; k < cnt; k++)
		{
			bones[j].children[k] = get_child_index(bones[j].children[k]);

		}

	}



	for (int i = 0; i < num_joints; i++)
	{
		


		for (int j = 0; j < num_joints; j++)
		{
			int cnt = (int)bones[j].children.size();


			for (int k = 0; k < cnt; k++)
			{
				if (bones[j].children[k] == i)
				{
					

					bones[i].parent = j;

					break;

				}
			}



		}

		if (bones[i].parent == ROOT_MARKER) {
			root = i;
		}
	}

	



	for (int i = 0; i < num_meshes; i++)
	{
		
		

		clearData();

		MImeshes mtemp;

		
		

		mtemp.name = j_son["meshes"][i]["name"];

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["POSITION"].is_number_integer()) {

			int position = int(j_son["meshes"][i]["primitives"][0]["attributes"]["POSITION"]);
			int pos_bfr_v = int(j_son["accessors"][position]["bufferView"]);
			int pos_byte_off = int(j_son["bufferViews"][pos_bfr_v]["byteOffset"]);
			int pos_byte_len = int(j_son["bufferViews"][pos_bfr_v]["byteLength"]);


			bin_file.seekg(pos_byte_off, std::ios::beg);




			for (int j = 0; j < (pos_byte_len / int(sizeof(float))); j++)
			{




				bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));



				vertVals.push_back(temp_float);


			}

			mtemp.triangleVerts = vertVals;

		}

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["NORMAL"].is_number_integer()) {


			int normal = int(j_son["meshes"][i]["primitives"][0]["attributes"]["NORMAL"]);
			int norm_bfr_v = int(j_son["accessors"][normal]["bufferView"]);

			int norm_byte_off = int(j_son["bufferViews"][norm_bfr_v]["byteOffset"]);
			int norm_byte_len = int(j_son["bufferViews"][norm_bfr_v]["byteLength"]);

			bin_file.seekg(norm_byte_off, std::ios::beg);


			for (int j = 0; j < (norm_byte_len / int(sizeof(float))); j++)
			{
				bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

				normVals.push_back(temp_float);


			}

			mtemp.normals = normVals;
		}

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["TEXCOORD_0"].is_number_integer()) {


			int texcoord = int(j_son["meshes"][i]["primitives"][0]["attributes"]["TEXCOORD_0"]);
			int tc_bfr_v = int(j_son["accessors"][texcoord]["bufferView"]);

			int tc_byte_off = int(j_son["bufferViews"][tc_bfr_v]["byteOffset"]);
			int tc_byte_len = int(j_son["bufferViews"][tc_bfr_v]["byteLength"]);


			bin_file.seekg(tc_byte_off, std::ios::beg);


			for (int j = 0; j < (tc_byte_len / int(sizeof(float))); j++)
			{
				bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

				if (j % 2 == 1) {
					temp_float *= -1.0;
				}

				stVals.push_back(temp_float);


			}

			mtemp.textureCoords = stVals;

		}

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["JOINTS_0"].is_number_integer()) {

			int joint = int(j_son["meshes"][i]["primitives"][0]["attributes"]["JOINTS_0"]);
			int j_bfr_v = int(j_son["accessors"][joint]["bufferView"]);


			int j_byte_off = int(j_son["bufferViews"][j_bfr_v]["byteOffset"]);
			int j_byte_len = int(j_son["bufferViews"][j_bfr_v]["byteLength"]);

			bin_file.seekg(j_byte_off, std::ios::beg);

						
			bin_file.read(reinterpret_cast<char*>(&temp_ubyte), sizeof(uint8_t));

			mtemp.jointIndex = temp_ubyte;
		}

		if (j_son["meshes"][i]["primitives"][0]["indices"].is_number_integer()) {

			int indice = int(j_son["meshes"][i]["primitives"][0]["indices"]);
			int ind_bfr_v = int(j_son["accessors"][indice]["bufferView"]);

			int ind_byte_off = int(j_son["bufferViews"][ind_bfr_v]["byteOffset"]);
			int ind_byte_len = int(j_son["bufferViews"][ind_bfr_v]["byteLength"]);


			bin_file.seekg(ind_byte_off, std::ios::beg);

			

			for (int j = 0; j < (ind_byte_len / int(sizeof(uint16_t))); j++)
			{
				bin_file.read(reinterpret_cast<char*>(&temp_ushort), sizeof(uint16_t));

				


				indices.push_back(temp_ushort);
			}			
			

			
			mtemp.indices = indices;				

			
		}		



		if (j_son["animations"].is_array()) {

			num_anims = (int)j_son["animations"].size();
		}

	
		for (int i = 0; i < num_anims; i++)
		{

			animJoint an;

			int jointInd;


		


			for (int k = 0; k < (int)j_son["animations"][i]["channels"].size(); k += 3)
			{

				std::vector<glm::vec3> translations;
				std::vector<glm::vec3> scales;
				std::vector<glm::quat> rotations;

				std::vector<float> transTimes;
				std::vector<float> scalTimes;
				std::vector<float> rotTimes;


				int node = (int)j_son["animations"][i]["channels"][k]["target"]["node"];

				jointInd = findBoneByNode(node);


				int samplerT = (int)j_son["animations"][i]["channels"][k]["sampler"];
				int samplerR = (int)j_son["animations"][i]["channels"][k + 1]["sampler"];
				int samplerS = (int)j_son["animations"][i]["channels"][k + 2]["sampler"];


				int acsTI = (int)j_son["animations"][i]["samplers"][samplerT]["input"];
				int acsTO = (int)j_son["animations"][i]["samplers"][samplerT]["output"];


				int acsSI = (int)j_son["animations"][i]["samplers"][samplerS]["input"];
				int acsSO = (int)j_son["animations"][i]["samplers"][samplerS]["output"];

				int acsRI = (int)j_son["animations"][i]["samplers"][samplerR]["input"];
				int acsRO = (int)j_son["animations"][i]["samplers"][samplerR]["output"];


				int bfrTI = (int)j_son["accessors"][acsTI]["bufferView"];
				int bfrTO = (int)j_son["accessors"][acsTO]["bufferView"];


				int bfrSI = (int)j_son["accessors"][acsSI]["bufferView"];
				int bfrSO = (int)j_son["accessors"][acsSO]["bufferView"];


				int bfrRI = (int)j_son["accessors"][acsRI]["bufferView"];
				int bfrRO = (int)j_son["accessors"][acsRO]["bufferView"];


				int TI_bytelen = (int)j_son["bufferViews"][bfrTI]["byteLength"];
				int TI_byteoff = (int)j_son["bufferViews"][bfrTI]["byteOffset"];

				int TO_bytelen = (int)j_son["bufferViews"][bfrTO]["byteLength"];
				int TO_byteoff = (int)j_son["bufferViews"][bfrTO]["byteOffset"];



				int RI_bytelen = (int)j_son["bufferViews"][bfrRI]["byteLength"];
				int RI_byteoff = (int)j_son["bufferViews"][bfrRI]["byteOffset"];

				int RO_bytelen = (int)j_son["bufferViews"][bfrRO]["byteLength"];
				int RO_byteoff = (int)j_son["bufferViews"][bfrRO]["byteOffset"];



				int SI_bytelen = (int)j_son["bufferViews"][bfrSI]["byteLength"];
				int SI_byteoff = (int)j_son["bufferViews"][bfrSI]["byteOffset"];

				int SO_bytelen = (int)j_son["bufferViews"][bfrSO]["byteLength"];
				int SO_byteoff = (int)j_son["bufferViews"][bfrSO]["byteOffset"];


				bin_file.seekg(TI_byteoff, std::ios::beg);



				for (int l = 0; l < TI_bytelen / (int)sizeof(float); l++)
				{
					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					transTimes.push_back(temp_float);
				}



				bin_file.seekg(TO_byteoff, std::ios::beg);

				for (int l = 0; l < TO_bytelen / (int)sizeof(glm::vec3); l++)
				{
					glm::vec3 temp_trans;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_trans.x = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_trans.y = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_trans.z = temp_float;

					translations.push_back(temp_trans);
				}



				bin_file.seekg(RI_byteoff, std::ios::beg);

				for (int l = 0; l < RI_bytelen / (int)sizeof(float); l++)
				{
					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					rotTimes.push_back(temp_float);
				}

				bin_file.seekg(RO_byteoff, std::ios::beg);



				for (int l = 0; l < RO_bytelen / (int)sizeof(glm::quat); l++)
				{
					glm::quat temp_rot;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.x = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.y = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.z = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.w = temp_float;

					rotations.push_back(temp_rot);
				}


			


				bin_file.seekg(SI_byteoff, std::ios::beg);

				for (int l = 0; l < SI_bytelen / (int)sizeof(float); l++)
				{
					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					scalTimes.push_back(temp_float);
				}

				bin_file.seekg(SO_byteoff, std::ios::beg);

				for (int l = 0; l < SO_bytelen / (int)sizeof(glm::vec3); l++)
				{
					glm::vec3 temp_scale;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_scale.x = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_scale.y = temp_float;

					bin_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_scale.z = temp_float;

					scales.push_back(temp_scale);
				}

				an.translations = translations;
				an.rotations = rotations;
				an.scales = scales;

				an.transTimes = transTimes;
				an.rotTimes = rotTimes;
				an.scalTimes = scalTimes;				
					

				bones[jointInd].animations.push_back(an);


				translations.clear();
				scales.clear();
				rotations.clear();

				transTimes.clear();
				scalTimes.clear();
				rotTimes.clear();


			}


			

		}

		ImpMeshes.push_back(mtemp);

	}



	bin_file.close();
}

void ImportedModel::compute_pose(int boneI) {

	bones[boneI].TransformMat = compute_transformJ(boneI);

	int proc = (int)bones[boneI].children.size();

	if (proc==0)
	{
		return;
	}

	else
	{
		for (int i = 0; i < proc; i++)
		{
			compute_pose(bones[boneI].children[i]);
		}
	}

}

glm::mat4x4 ImportedModel::compute_transformJ(int boneI) {
	int boneP = bones[boneI].parent;

	if (boneI!=root)
	{
		return bones[boneP].TransformMat * bones[boneI].TransformMat;
	}

	else
	{
		return bones[boneI].TransformMat;
	}

}






GLuint* ModelImporter::loadRPF(const char* filePathRel) {

	std::string filePath = "Assets/" + (std::string)filePathRel;

	std::ifstream rpfloader(filePath, std::ios::binary);

	RPF rpf;

	GLuint Mode = 0;

	rpfloader.read(reinterpret_cast<char*>(&rpf.magic), sizeof(rpf.magic));

	rpf.CLUT = new Pixel32[(int)(rpf.magic[1] + 1)];

	Pixel temp_pixel;


	rpf.data = new uint8_t[((int)rpf.magic[2] + 1) * ((int)rpf.magic[3] + 1)];

	uint16_t reader = 0;

	for (int i = 0; i < ((int)rpf.magic[1] + 1); i++)
	{
		rpfloader.read(reinterpret_cast<char*>(&reader), 1 * sizeof(uint16_t));


		temp_pixel.a = (reader >> 0) & 0x01;
		temp_pixel.r = (reader >> 1) & 0x1F;
		temp_pixel.g = (reader >> 6) & 0x1F;
		temp_pixel.b = (reader >> 11) & 0x1F;

		rpf.CLUT[i].a = temp_pixel.a * 255;
		rpf.CLUT[i].r = temp_pixel.r * 8;
		rpf.CLUT[i].g = temp_pixel.g * 8;
		rpf.CLUT[i].b = temp_pixel.b * 8;

		

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

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA16, tempc, 0, GL_RGBA, GL_UNSIGNED_BYTE, rpf.CLUT);


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

		if (bones[i].name == j_son["nodes"][j_son_index]["name"]) {

			
			return i;

		}

	}


	return -1;
}

void ModelImporter::clearRig() {

	transforms.clear();
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

