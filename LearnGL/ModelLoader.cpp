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

	if (useAST) {
		importer.OpenAST(filePath);
	}
	else
	{
		
		importer.parseGLB(filePath, 12, 60, 0, 0, (uint8_t)(tex_width - 1), (uint8_t)(tex_height - 1));
	}


	
	

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


void ModelImporter::parseGLB(const char* filePathRel, uint8_t scalingFactorBits, uint8_t FPS, uint8_t tPageX_begin, uint8_t tPageY_begin, uint8_t tPageX_end, uint8_t tPageY_end) {

	num_meshes = 0;
	num_mats = 0;
	num_joints = 0;
	num_anims = 0;

	float temp_float = 0.0f;
	uint16_t temp_ushort = 0;
	uint8_t temp_ubyte = 0;

	uint32_t  json_length;

	const std::string filePath = "Assets/" + (std::string)filePathRel + ".glb";


	std::ifstream glb_file(filePath, std::ios::binary);




	glb_file.seekg(12,std::ios::beg);

	glb_file.read(reinterpret_cast<char*>(&json_length), sizeof(json_length));


	std::vector<char> json_string(json_length);

	glb_file.seekg(20,std::ios::beg);

	glb_file.read(json_string.data(), json_length);


	j_son = nlohmann::json::parse(json_string);


	num_meshes = int(j_son["meshes"].size());


	json_length += 28;


	if (j_son["skins"][0]["inverseBindMatrices"].is_number_integer()) {


		int invBind = j_son["skins"][0]["inverseBindMatrices"];
		int invBindv = j_son["accessors"][invBind]["bufferView"];
		int invBindOff = j_son["bufferViews"][invBindv]["byteOffset"] + json_length;

		num_mats = j_son["accessors"][invBind]["count"];



		glb_file.seekg(invBindOff, std::ios::beg);
		for (int i = 0; i < num_mats; i++)
		{
			glm::mat4x4 mattemp = glm::mat4x4(1.0f);

			for (int j = 0; j < 4; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

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
			glm::quat rot = glm::angleAxis(glm::pi<float>(), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

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



			tempbone.TransformMat = tempbone.InitTransform.Matrix();



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

		MImesh mtemp;




		mtemp.name = j_son["meshes"][i]["name"];

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["POSITION"].is_number_integer()) {

			int position = int(j_son["meshes"][i]["primitives"][0]["attributes"]["POSITION"]);
			int pos_bfr_v = int(j_son["accessors"][position]["bufferView"]);
			int pos_byte_off = int(j_son["bufferViews"][pos_bfr_v]["byteOffset"] + json_length);
			int pos_byte_len = int(j_son["bufferViews"][pos_bfr_v]["byteLength"]);


			glb_file.seekg(pos_byte_off, std::ios::beg);




			for (int j = 0; j < (pos_byte_len / int(sizeof(float))); j++)
			{




				glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));



				vertVals.push_back(temp_float);


			}

			mtemp.triangleVerts = vertVals;

		}

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["NORMAL"].is_number_integer()) {


			int normal = int(j_son["meshes"][i]["primitives"][0]["attributes"]["NORMAL"]);
			int norm_bfr_v = int(j_son["accessors"][normal]["bufferView"]);

			int norm_byte_off = int(j_son["bufferViews"][norm_bfr_v]["byteOffset"] + json_length);
			int norm_byte_len = int(j_son["bufferViews"][norm_bfr_v]["byteLength"]);

			glb_file.seekg(norm_byte_off, std::ios::beg);


			for (int j = 0; j < (norm_byte_len / int(sizeof(float))); j++)
			{
				glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

				normVals.push_back(temp_float);


			}

			mtemp.normals = normVals;
		}

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["TEXCOORD_0"].is_number_integer()) {


			int texcoord = int(j_son["meshes"][i]["primitives"][0]["attributes"]["TEXCOORD_0"]);
			int tc_bfr_v = int(j_son["accessors"][texcoord]["bufferView"]);

			int tc_byte_off = int(j_son["bufferViews"][tc_bfr_v]["byteOffset"] + json_length);
			int tc_byte_len = int(j_son["bufferViews"][tc_bfr_v]["byteLength"]);


			glb_file.seekg(tc_byte_off, std::ios::beg);


			for (int j = 0; j < (tc_byte_len / int(sizeof(float))); j++)
			{
				glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));



				stVals.push_back(temp_float);


			}

			mtemp.textureCoords = stVals;

		}

		if (j_son["meshes"][i]["primitives"][0]["attributes"]["JOINTS_0"].is_number_integer()) {

			int joint = int(j_son["meshes"][i]["primitives"][0]["attributes"]["JOINTS_0"]);
			int j_bfr_v = int(j_son["accessors"][joint]["bufferView"]);


			int j_byte_off = int(j_son["bufferViews"][j_bfr_v]["byteOffset"] + json_length);
			int j_byte_len = int(j_son["bufferViews"][j_bfr_v]["byteLength"]);

			glb_file.seekg(j_byte_off, std::ios::beg);


			glb_file.read(reinterpret_cast<char*>(&temp_ubyte), sizeof(uint8_t));

			mtemp.jointIndex = temp_ubyte;
		}

		if (j_son["meshes"][i]["primitives"][0]["indices"].is_number_integer()) {

			int indice = int(j_son["meshes"][i]["primitives"][0]["indices"]);
			int ind_bfr_v = int(j_son["accessors"][indice]["bufferView"]);

			int ind_byte_off = int(j_son["bufferViews"][ind_bfr_v]["byteOffset"] + json_length);
			int ind_byte_len = int(j_son["bufferViews"][ind_bfr_v]["byteLength"]);


			glb_file.seekg(ind_byte_off, std::ios::beg);



			for (int j = 0; j < (ind_byte_len / int(sizeof(uint16_t))); j++)
			{
				glb_file.read(reinterpret_cast<char*>(&temp_ushort), sizeof(uint16_t));




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
				int TI_byteoff = (int)j_son["bufferViews"][bfrTI]["byteOffset"] + json_length;

				int TO_bytelen = (int)j_son["bufferViews"][bfrTO]["byteLength"];
				int TO_byteoff = (int)j_son["bufferViews"][bfrTO]["byteOffset"] + json_length;



				int RI_bytelen = (int)j_son["bufferViews"][bfrRI]["byteLength"];
				int RI_byteoff = (int)j_son["bufferViews"][bfrRI]["byteOffset"] + json_length;

				int RO_bytelen = (int)j_son["bufferViews"][bfrRO]["byteLength"];
				int RO_byteoff = (int)j_son["bufferViews"][bfrRO]["byteOffset"] + json_length;



				int SI_bytelen = (int)j_son["bufferViews"][bfrSI]["byteLength"];
				int SI_byteoff = (int)j_son["bufferViews"][bfrSI]["byteOffset"] + json_length;

				int SO_bytelen = (int)j_son["bufferViews"][bfrSO]["byteLength"];
				int SO_byteoff = (int)j_son["bufferViews"][bfrSO]["byteOffset"] + json_length;


				glb_file.seekg(TI_byteoff, std::ios::beg);



				for (int l = 0; l < TI_bytelen / (int)sizeof(float); l++)
				{
					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					transTimes.push_back(temp_float);
				}



				glb_file.seekg(TO_byteoff, std::ios::beg);

				for (int l = 0; l < TO_bytelen / (int)sizeof(glm::vec3); l++)
				{
					glm::vec3 temp_trans;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_trans.x = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_trans.y = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_trans.z = temp_float;

					translations.push_back(temp_trans);
				}



				glb_file.seekg(RI_byteoff, std::ios::beg);

				for (int l = 0; l < RI_bytelen / (int)sizeof(float); l++)
				{
					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					rotTimes.push_back(temp_float);
				}

				glb_file.seekg(RO_byteoff, std::ios::beg);



				for (int l = 0; l < RO_bytelen / (int)sizeof(glm::quat); l++)
				{
					glm::quat temp_rot;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.x = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.y = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.z = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_rot.w = temp_float;

					rotations.push_back(temp_rot);
				}





				glb_file.seekg(SI_byteoff, std::ios::beg);

				for (int l = 0; l < SI_bytelen / (int)sizeof(float); l++)
				{
					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					scalTimes.push_back(temp_float);
				}

				glb_file.seekg(SO_byteoff, std::ios::beg);

				for (int l = 0; l < SO_bytelen / (int)sizeof(glm::vec3); l++)
				{
					glm::vec3 temp_scale;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_scale.x = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_scale.y = temp_float;

					glb_file.read(reinterpret_cast<char*>(&temp_float), sizeof(float));

					temp_scale.z = temp_float;

					scales.push_back(temp_scale);
				}

				an.translations = translations;
				an.rotations = rotations;
				an.scales = scales;

				an.transTimes = transTimes;
				an.rotTimes = rotTimes;
				an.scalTimes = scalTimes;

				if (jointInd == mtemp.jointIndex) {
					bones[jointInd].animations.push_back(an);
				}



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


	glb_file.close();


	GLTF_To_AST(filePathRel, scalingFactorBits, FPS, tPageX_begin, tPageY_begin, tPageX_end, tPageY_end);


}





void ModelImporter::parseGLTF(const char* filePathRel,uint8_t scalingFactorBits, uint8_t FPS, uint8_t tPageX_begin, uint8_t tPageY_begin, uint8_t tPageX_end, uint8_t tPageY_end) {

	num_meshes = 0;
	num_mats = 0;
	num_joints = 0;
	num_anims = 0;

	float temp_float=0.0f;
	uint16_t temp_ushort=0;
	uint8_t temp_ubyte=0;

	const std::string filePath = "Assets/" + (std::string)filePathRel+ ".gltf";

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
			glm::quat rot = glm::angleAxis(glm::pi<float>(), glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

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

			

		    tempbone.TransformMat = tempbone.InitTransform.Matrix();		 
			
			

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

		MImesh mtemp;

		
		

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
					
				if (jointInd == mtemp.jointIndex) {
					bones[jointInd].animations.push_back(an);					
				}

				

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



	GLTF_To_AST(filePathRel,scalingFactorBits,  FPS,  tPageX_begin,  tPageY_begin,  tPageX_end,  tPageY_end);
}




void ModelImporter::GLTF_To_AST(const char* filePathRel , uint8_t scalingFactorBits, uint8_t FPS, uint8_t tPageX_begin, uint8_t tPageY_begin, uint8_t tPageX_end, uint8_t tPageY_end) {

	fix tempFix;
	uint8_t tempByte;
	uint16_t tempShort;
	glm::mat4x4 tempMat;
	glm::vec4 tempVec;
	float tempFloat;

	const std::string vtPath = "Assets/" + (std::string)filePathRel + ".vt";
	const std::string indPath = "Assets/" + (std::string)filePathRel + ".ind";
	const std::string uvPath = "Assets/" + (std::string)filePathRel + ".uv";
	const std::string nrmPath = "Assets/" + (std::string)filePathRel + ".nrm";
	const std::string fkrPath = "Assets/" + (std::string)filePathRel + ".fkr";
	const std::string anmPath = "Assets/" + (std::string)filePathRel + ".anm";	
	

	std::ofstream vtxStream;
	vtxStream.open(vtPath,std::ios::binary);
	

	tempByte = (uint8_t)ImpMeshes.size();

	vtxStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	vtxStream.write(reinterpret_cast<char*>(&scalingFactorBits), sizeof(scalingFactorBits));

	for (MImesh msh : ImpMeshes)
	{
		tempByte = (uint8_t)msh.jointIndex;
		
		vtxStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

		tempShort = (uint16_t)msh.triangleVerts.size();
		vtxStream.write(reinterpret_cast<char*>(&tempShort),sizeof(tempShort));

		tempMat = glm::mat4x4(1.0f);

		if (bones.size() > 0) {
			tempMat = matrices[msh.jointIndex];
		}

		for (uint16_t i = 0; i < tempShort; i+=3)
		{
			tempVec = glm::vec4(msh.triangleVerts[i], msh.triangleVerts[i+1], msh.triangleVerts[i+2],1.0f);
			tempVec = tempMat * tempVec;

			tempFix = F_TO_FIX(tempVec.x, scalingFactorBits);
			vtxStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(tempVec.y, scalingFactorBits);
			vtxStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(tempVec.z, scalingFactorBits);
			vtxStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		}


	}

	vtxStream.close();

	std::ofstream indStream;

	indStream.open(indPath,std::ios::binary);

	tempByte = (uint8_t)ImpMeshes.size();

	indStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));


	for (MImesh msh : ImpMeshes)
	{
		tempShort = (uint16_t)msh.indices.size();
		indStream.write(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
		uint16_t count = tempShort;
		
		for (uint16_t i = 0; i < count; i++)
		{
			tempShort = msh.indices[i];
			indStream.write(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));

		}


	}


	indStream.close();


	std::ofstream uvStream;
	uvStream.open(uvPath,std::ios::binary);

	uvStream.write(reinterpret_cast<char*>(&tPageX_begin), sizeof(tPageX_begin));
	uvStream.write(reinterpret_cast<char*>(&tPageY_begin), sizeof(tPageY_begin));

	uvStream.write(reinterpret_cast<char*>(&tPageX_end), sizeof(tPageX_end));
	uvStream.write(reinterpret_cast<char*>(&tPageY_end), sizeof(tPageY_end));

	tempByte = (uint8_t)ImpMeshes.size();

	
	

	uvStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));


	uint16_t x_diff = (uint16_t)(tPageX_end - tPageX_begin) + 1;
	uint16_t y_diff = (uint16_t)(tPageY_end - tPageY_begin) + 1;


	for (MImesh msh : ImpMeshes)
	{
		
		
		tempShort = (uint16_t)msh.textureCoords.size();
		uvStream.write(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
		
		

		for (uint16_t i = 0; i < tempShort; i+=2)
		{
		
			tempByte = (uint8_t)((uint16_t)round(msh.textureCoords[i] * (float)(x_diff))%x_diff);

			uvStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
			
			tempByte = (uint8_t)((uint16_t)round(msh.textureCoords[i + 1] * (float)(y_diff))%y_diff);
			
			uvStream.write(reinterpret_cast<char*>(&tempByte),sizeof(tempByte));
		}
		

	}

	uvStream.close();


	std::ofstream nrmStream;
	nrmStream.open(nrmPath, std::ios::binary);
	

	tempByte = (uint8_t)ImpMeshes.size();

	nrmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	nrmStream.write(reinterpret_cast<char*>(&scalingFactorBits), sizeof(scalingFactorBits));

	for (MImesh msh : ImpMeshes)
	{
		tempShort = (uint16_t)msh.normals.size();
		nrmStream.write(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));

		tempMat = glm::mat4x4(1.0f);

		if (bones.size() > 0) {

			tempMat = matrices[msh.jointIndex];

		}

		for (uint16_t i = 0; i < tempShort; i += 3)
		{
			tempVec = glm::vec4(msh.normals[i], msh.normals[i+1], msh.normals[i+2],1.0f);
			tempVec = tempMat * tempVec;

			tempFix = F_TO_FIX(tempVec.x, scalingFactorBits);
			nrmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(tempVec.y, scalingFactorBits);
			nrmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(tempVec.z, scalingFactorBits);
			nrmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		}


	}

	nrmStream.close();

	std::ofstream fkrStream;
	fkrStream.open(fkrPath, std::ios::binary);

	tempByte = (uint8_t)bones.size();
	fkrStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	fkrStream.write(reinterpret_cast<char*>(&scalingFactorBits), sizeof(scalingFactorBits));

	if (bones.size() > 0) {

		for (Bone b : bones)
		{
			tempFix = F_TO_FIX(b.InitTransform.translation.x, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.translation.y, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.translation.z, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.rotation.x, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.rotation.y, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.rotation.z, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.rotation.w, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.scale.x, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.scale.y, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFix = F_TO_FIX(b.InitTransform.scale.z, scalingFactorBits);
			fkrStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));


			tempByte = (uint8_t)b.children.size();
			fkrStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			uint8_t count = tempByte;

			for (uint8_t i = 0; i < count; i++)
			{
				tempByte = (uint8_t)b.children[i];
				fkrStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));


			}

		}

	}

	fkrStream.close();

	std::ofstream anmStream;

	anmStream.open(anmPath,std::ios::binary);

	tempByte = (uint8_t)bones.size();

	anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	anmStream.write(reinterpret_cast<char*>(&scalingFactorBits), sizeof(scalingFactorBits));

	anmStream.write(reinterpret_cast<char*>(&FPS), sizeof(FPS));

	if (bones.size() > 0) {

		for (Bone b : bones)
		{
			tempByte = (uint8_t)b.animations.size();

			anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			uint8_t count = tempByte;

			for (uint8_t i = 0; i < count; i++)
			{

				tempByte = (uint8_t)b.animations[i].translations.size();
				anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
				uint8_t TRS_count = tempByte;

				for (uint8_t j = 0; j < TRS_count; j++)
				{
					tempFloat = (b.animations[i].transTimes[j] * (float)(FPS - 1));



					tempByte = (uint8_t)(((int)tempFloat) % (int)(FPS));
					anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));




					tempFix = F_TO_FIX(b.animations[i].translations[j].x, scalingFactorBits);

					

					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].translations[j].y, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].translations[j].z, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				}


				tempByte = (uint8_t)b.animations[i].rotations.size();
				anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
				TRS_count = tempByte;

				for (uint8_t j = 0; j < TRS_count; j++)
				{
					tempFloat = (b.animations[i].rotTimes[j] * (float)(FPS - 1));



					tempByte = (uint8_t)(((int)tempFloat) % (int)(FPS));
					anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));




					tempFix = F_TO_FIX(b.animations[i].rotations[j].x, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].rotations[j].y, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].rotations[j].z, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].rotations[j].w, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));
				}






				tempByte = (uint8_t)b.animations[i].scales.size();
				anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
				TRS_count = tempByte;

				for (uint8_t j = 0; j < TRS_count; j++)
				{
					tempFloat = (b.animations[i].scalTimes[j] * (float)(FPS - 1));



					tempByte = (uint8_t)(((int)tempFloat) % (int)(FPS));
					anmStream.write(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));



					tempFix = F_TO_FIX(b.animations[i].scales[j].x, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].scales[j].y, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

					tempFix = F_TO_FIX(b.animations[i].scales[j].z, scalingFactorBits);
					anmStream.write(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				}



			}


		}

	}
	anmStream.close();

}

void ModelImporter::OpenAST(const char* filePathRel) {

	fix tempFix;
	uint8_t tempByte;
	uint16_t tempShort;
	float tempFloat;
	MImesh tempMesh;
	Bone tempBone;
	animJoint tempAnim;
	glm::vec4 tempVec;

	const std::string vtPath = "Assets/" + (std::string)filePathRel + ".vt";
	const std::string indPath = "Assets/" + (std::string)filePathRel + ".ind";
	const std::string uvPath = "Assets/" + (std::string)filePathRel + ".uv";
	const std::string nrmPath = "Assets/" + (std::string)filePathRel + ".nrm";
	const std::string fkrPath = "Assets/" + (std::string)filePathRel + ".fkr";
	const std::string anmPath = "Assets/" + (std::string)filePathRel + ".anm";

	std::ifstream vtStream;

	vtStream.open(vtPath,std::ios::binary);
	vtStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t mshCount = tempByte;
	num_meshes = mshCount;

	vtStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t scalingFactor = tempByte;

	for (uint8_t i = 0; i < mshCount; i++)
	{
		

		vtStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));
		tempMesh.jointIndex = (int)tempByte;
		vtStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
		
		for (uint16_t j = 0; j < tempShort; j++)
		{
			vtStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFloat = FIX_TO_F(tempFix,scalingFactor);

			tempMesh.triangleVerts.push_back(tempFloat);
		}
		ImpMeshes.push_back(tempMesh);

		tempMesh.triangleVerts.clear();

		
	}

	vtStream.close();

	std::ifstream indStream;

	indStream.open(indPath,std::ios::binary);

	

	indStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	mshCount = tempByte;


	for (uint8_t i = 0; i < mshCount; i++)
	{
		indStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
		uint16_t indCount = tempShort;

		for (uint16_t j = 0; j < indCount; j++)
		{
			indStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));
			tempMesh.indices.push_back(tempShort);


		}

		ImpMeshes[i].indices.clear();
		ImpMeshes[i].indices = tempMesh.indices;

		tempMesh.indices.clear();

		

	}


	indStream.close();

	std::ifstream uvStream;

	uvStream.open(uvPath,std::ios::binary);

	uint8_t x_beg;
	uint8_t x_end;

	uint8_t y_beg;
	uint8_t y_end;

	uvStream.read(reinterpret_cast<char*>(&x_beg), sizeof(x_beg));
	uvStream.read(reinterpret_cast<char*>(&y_beg), sizeof(y_beg));
	
	uvStream.read(reinterpret_cast<char*>(&x_end), sizeof(x_end));
	uvStream.read(reinterpret_cast<char*>(&y_end), sizeof(y_end));



	uvStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	mshCount = tempByte;

	uint16_t x_diff = (uint16_t)(x_end - x_beg) +1;
	uint16_t y_diff = (uint16_t)(y_end - y_beg) +1;

	for (uint8_t i = 0; i < mshCount; i++)
	{
		uvStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));

		for (uint16_t j = 0; j < tempShort; j+=2)
		{
			uvStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			tempFloat = (float(tempByte) / (float)(x_diff));			


			tempMesh.textureCoords.push_back(tempFloat);

			uvStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			tempFloat = (float(tempByte) / (float)(y_diff));

			tempMesh.textureCoords.push_back(tempFloat);

		}


		ImpMeshes[i].textureCoords.clear();
		ImpMeshes[i].textureCoords = tempMesh.textureCoords;

		tempMesh.textureCoords.clear();

		

	}
	uvStream.close();


	std::ifstream nrmStream;

	nrmStream.open(nrmPath,std::ios::binary);

	nrmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	mshCount = tempByte;

	nrmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	scalingFactor = tempByte;

	for (uint8_t i = 0; i < mshCount; i++)
	{

		nrmStream.read(reinterpret_cast<char*>(&tempShort), sizeof(tempShort));

		for (uint16_t j = 0; j < tempShort; j++)
		{

			nrmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

			tempFloat = FIX_TO_F(tempFix,scalingFactor);

			tempMesh.normals.push_back(tempFloat);

		}


		ImpMeshes[i].normals.clear();
		ImpMeshes[i].normals = tempMesh.normals;

		tempMesh.normals.clear();

		
	}


	nrmStream.close();


	std::ifstream fkrStream;

	fkrStream.open(fkrPath,std::ios::binary);

	fkrStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t boneCount = tempByte;

	num_joints = boneCount;

	fkrStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	scalingFactor = tempByte;

	

	for (uint8_t i = 0; i < boneCount; i++)
	{
		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.translation.x = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.translation.y = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.translation.z = FIX_TO_F(tempFix, scalingFactor);


		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.x = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.y = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.z = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.rotation.w = FIX_TO_F(tempFix, scalingFactor);


		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.scale.x = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.scale.y = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

		tempBone.InitTransform.scale.z = FIX_TO_F(tempFix, scalingFactor);

		fkrStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

		mshCount = tempByte;

		if (mshCount == 0) {
			tempBone.extremity = true;
		}

		for (uint8_t j = 0; j < mshCount; j++)
		{
			fkrStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			tempBone.children.push_back((int)tempByte);
		}

		tempBone.TransformMat = tempBone.InitTransform.Matrix();

		bones.push_back(tempBone);
		

		tempBone.children.clear();

		
	}	
	
	fkrStream.close();


	for (uint8_t i = 0; i < (uint8_t)num_joints; i++)
	{
		bones[i].parent = ROOT_MARKER;

		for (uint8_t j = 0; j < (uint8_t)num_joints; j++)
		{

			for (uint8_t k = 0; k < (uint8_t)bones[j].children.size(); k++) {

				if (bones[j].children[k]==i)
				{
					bones[i].parent = j;
				}
			}

		}

		if (bones[i].parent==ROOT_MARKER) {

			root = i;

		}

	}



	std::ifstream anmStream;

	anmStream.open(anmPath, std::ios::binary);

	anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	boneCount = tempByte;

	

	anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	scalingFactor = tempByte;

	

	anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

	uint8_t targetFPS = tempByte;

	



	for (uint8_t i = 0; i < boneCount; i++)
	{
		anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

		uint8_t animCount = tempByte;

		num_anims = animCount;

		for (uint8_t j = 0; j < animCount; j++) {

			anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			uint8_t TRSCount = tempByte;

			for (uint8_t k = 0; k < TRSCount; k++)
			{
				anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

				tempFloat = (float(tempByte))/(float(targetFPS));

				if (k > 0) {

					while (tempFloat < tempAnim.transTimes[k-1])
					{
						tempFloat += 1.0f;
					}

				}

				tempAnim.transTimes.push_back(tempFloat);

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.x = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.y = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.z = tempFloat;

				tempAnim.translations.push_back(glm::vec3(tempVec.x,tempVec.y,tempVec.z));

			}


			anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			TRSCount = tempByte;

			for (uint8_t k = 0; k < TRSCount; k++)
			{
				anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

				tempFloat = (float(tempByte)) / (float(targetFPS));

				if (k > 0) {

					while (tempFloat < tempAnim.rotTimes[k - 1])
					{
						tempFloat += 1.0f;
					}

				}

				tempAnim.rotTimes.push_back(tempFloat);

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.x = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.y = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.z = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.w = tempFloat;

				tempAnim.rotations.push_back(glm::normalize(glm::quat(tempVec.w,tempVec.x,tempVec.y,tempVec.z)));

			}

			anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

			TRSCount = tempByte;

			for (uint8_t k = 0; k < TRSCount; k++)
			{
				anmStream.read(reinterpret_cast<char*>(&tempByte), sizeof(tempByte));

				tempFloat = (float(tempByte)) / (float(targetFPS));

				if (k > 0) {

					while (tempFloat < tempAnim.scalTimes[k - 1])
					{
						tempFloat += 1.0f;
					}

				}

				tempAnim.scalTimes.push_back(tempFloat);

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.x = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

				tempFloat = FIX_TO_F(tempFix, scalingFactor);

				tempVec.y = tempFloat;

				anmStream.read(reinterpret_cast<char*>(&tempFix), sizeof(tempFix));

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

	
	


	anmStream.close();

	
}

void ImportedModel::compute_pose(int boneI) {

	bones[boneI].TransformMat = compute_transformJ(boneI);

	int proc = (int)bones[boneI].children.size();

	if (proc == 0)
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

	if (boneI != root)
	{
		return bones[boneP].TransformMat * bones[boneI].TransformMat;
	}

	else
	{
		return bones[boneI].TransformMat;
	}

}



GLuint* ModelImporter::loadRPF(const char* filePathRel) {

	std::string filePath = "Assets/" + (std::string)filePathRel +".rpf";

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

		if (bones[i].name == j_son["nodes"][j_son_index]["name"]) {

			
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
