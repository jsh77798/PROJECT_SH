#include "pch.h"
#include "Model.h"
#include "Utils.h"
#include "FileUtils.h"
#include "tinyxml2.h"
#include <filesystem>
#include "Material.h"
#include "ModelMesh.h"
#include "ModelAnimation.h"
#include "Converter.h"

Model::Model()
{

}

Model::~Model()
{

}

void Model::ReadMaterial(wstring filename)
{
	wstring fullPath = _texturePath + filename + L"/" + filename + L".xml";
	auto parentPath = filesystem::path(fullPath).parent_path();

	tinyxml2::XMLDocument* document = new tinyxml2::XMLDocument();
	tinyxml2::XMLError error = document->LoadFile(Utils::ToString(fullPath).c_str());
	assert(error == tinyxml2::XML_SUCCESS);

	tinyxml2::XMLElement* root = document->FirstChildElement();
	tinyxml2::XMLElement* materialNode = root->FirstChildElement();

	while (materialNode)
	{
		shared_ptr<Material> material = make_shared<Material>();

		tinyxml2::XMLElement* node = nullptr;

		node = materialNode->FirstChildElement();
		material->SetName(Utils::ToWString(node->GetText()));

		// Diffuse Texture
		//node = node->NextSiblingElement();
		//if (node->GetText())
		//{
		//	wstring textureStr = Utils::ToWString(node->GetText());
		//	if (textureStr.length() > 0)
		//	{
		//		auto texture = RESOURCES->GetOrAddTexture(textureStr, (parentPath / textureStr).wstring());
		//		material->SetDiffuseMap(texture);
		//	}
		//}
		node = node->NextSiblingElement();

		if (node->GetText())
		{
			wstring textureStr = Utils::ToWString(node->GetText());

			if (!textureStr.empty())
			{
				wstring texturePath = filesystem::absolute(
					parentPath / textureStr
				).lexically_normal().wstring();

				auto texture = RESOURCES->GetOrAddTexture(
					texturePath,
					texturePath
				);

				material->SetDiffuseMap(texture);
			}
		}



		// Specular Texture
		//node = node->NextSiblingElement();
		//if (node->GetText())
		//{
		//	wstring texture = Utils::ToWString(node->GetText());
		//	if (texture.length() > 0)
		//	{
		//		wstring textureStr = Utils::ToWString(node->GetText());
		//		if (textureStr.length() > 0)
		//		{
		//			auto texture = RESOURCES->GetOrAddTexture(textureStr, (parentPath / textureStr).wstring());
		//			material->SetSpecularMap(texture);
		//		}
		//	}
		//}
		node = node->NextSiblingElement();

		if (node->GetText())
		{
			wstring textureStr = Utils::ToWString(node->GetText());

			if (!textureStr.empty())
			{
				wstring texturePath = filesystem::absolute(
					parentPath / textureStr
				).lexically_normal().wstring();

				auto texture = RESOURCES->GetOrAddTexture(
					texturePath,
					texturePath
				);

				material->SetSpecularMap(texture);
			}
		}

		// Normal Texture
		//node = node->NextSiblingElement();
		//if (node->GetText())
		//{
		//	wstring textureStr = Utils::ToWString(node->GetText());
		//	if (textureStr.length() > 0)
		//	{
		//		auto texture = RESOURCES->GetOrAddTexture(textureStr, (parentPath / textureStr).wstring());
		//		material->SetNormalMap(texture);
		//	}
		//}
		node = node->NextSiblingElement();

		if (node->GetText())
		{
			wstring textureStr = Utils::ToWString(node->GetText());

			if (!textureStr.empty())
			{
				wstring texturePath = filesystem::absolute(
					parentPath / textureStr
				).lexically_normal().wstring();

				auto texture = RESOURCES->GetOrAddTexture(
					texturePath,
					texturePath
				);

				material->SetNormalMap(texture);
			}
		}

		// Ambient
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->GetMaterialDesc().ambient = color;
		}

		// Diffuse
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->GetMaterialDesc().diffuse = color;
		}

		// Specular
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->GetMaterialDesc().specular = color;
		}

		// Emissive
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->GetMaterialDesc().emissive = color;
		}

		_materials.push_back(material);

		// Next Material
		materialNode = materialNode->NextSiblingElement();
	}

	BindCacheInfo();
}

void Model::ReadModel(wstring filename)
{
	wstring fullPath = _modelPath + filename + L".mesh";

	shared_ptr<FileUtils> file = make_shared<FileUtils>();
	file->Open(fullPath, FileMode::Read);

	// Bones
	{
		const uint32 count = file->Read<uint32>();

		for (uint32 i = 0; i < count; i++)
		{
			shared_ptr<ModelBone> bone = make_shared<ModelBone>();
			bone->index = file->Read<int32>();
			bone->name = Utils::ToWString(file->Read<string>());
			bone->parentIndex = file->Read<int32>();
			bone->transform = file->Read<Matrix>();

			_bones.push_back(bone);
		}
	}

	//// Mesh
	//{
	//	const uint32 count = file->Read<uint32>();
	//
	//	for (uint32 i = 0; i < count; i++)
	//	{
	//		shared_ptr<ModelMesh> mesh = make_shared<ModelMesh>();
	//
	//		mesh->name = Utils::ToWString(file->Read<string>());
	//		mesh->boneIndex = file->Read<int32>();
	//
	//		// Material
	//		mesh->materialName = Utils::ToWString(file->Read<string>());
	//
	//		//VertexData
	//		{
	//			const uint32 count = file->Read<uint32>();
	//			vector<ModelVertexType> vertices;
	//			vertices.resize(count);
	//
	//			void* data = vertices.data();
	//			file->Read(&data, sizeof(ModelVertexType) * count);
	//			mesh->geometry->AddVertices(vertices);
	//		}
	//
	//		//IndexData
	//		{
	//			const uint32 count = file->Read<uint32>();
	//
	//			vector<uint32> indices;
	//			indices.resize(count);
	//
	//			void* data = indices.data();
	//			file->Read(&data, sizeof(uint32) * count);
	//			mesh->geometry->AddIndices(indices);
	//		}
	//
	//		mesh->CreateBuffers();
	//
	//		_meshes.push_back(mesh);
	//	}
	//}
	//
	//BindCacheInfo();


	// Mesh
	{
		_collisionBoxes.clear();
		_collisionSlopes.clear();

		const uint32 meshCount = file->Read<uint32>();

		for (uint32 i = 0; i < meshCount; i++)
		{
			auto mesh = make_shared<ModelMesh>();

			mesh->name = Utils::ToWString(file->Read<string>());
			mesh->boneIndex = file->Read<int32>();
			mesh->materialName =
				Utils::ToWString(file->Read<string>());

			// VertexData
			const uint32 vertexCount = file->Read<uint32>();

			vector<ModelVertexType> vertices(vertexCount);

			if (vertexCount > 0)
			{
				void* data = vertices.data();
				file->Read(
					&data,
					sizeof(ModelVertexType) * vertexCount
				);
			}

			// IndexData
			// 충돌 메시도 반드시 인덱스 데이터까지 읽어야
			// 다음 메시의 파일 위치가 맞음
			const uint32 indexCount = file->Read<uint32>();

			vector<uint32> indices(indexCount);

			if (indexCount > 0)
			{
				void* data = indices.data();
				file->Read(
					&data,
					sizeof(uint32) * indexCount
				);
			}

			//const bool isCollisionMesh =
			//	mesh->name.rfind(L"COL_", 0) == 0;
			//
			//if (isCollisionMesh)
			//{
			//	AddCollisionBox(mesh->boneIndex, vertices);
			//
			//	// 렌더링 메시 목록에는 추가하지 않음
			//	continue;
			//}

			if (mesh->name.rfind(L"COL_Slope_", 0) == 0)
			{
				AddCollisionSlope(mesh->boneIndex, vertices);
				continue;
			}

			if (mesh->name.rfind(L"COL_", 0) == 0)
			{
				AddCollisionBox(mesh->boneIndex, vertices);
				continue;
			}

			mesh->geometry->AddVertices(vertices);
			mesh->geometry->AddIndices(indices);
			mesh->CreateBuffers();

			_meshes.push_back(mesh);
		}
	}

	BindCacheInfo();
}

void Model::ReadAnimation(wstring filename)
{
	wstring fullPath = _modelPath + filename + L".clip";

	shared_ptr<FileUtils> file = make_shared<FileUtils>();
	file->Open(fullPath, FileMode::Read);

	shared_ptr<ModelAnimation> animation = make_shared<ModelAnimation>();

	animation->filePath = _modelPath + filename + L".clip";
	animation->name = Utils::ToWString(file->Read<string>());
	animation->duration = file->Read<float>();
	animation->frameRate = file->Read<float>();
	animation->frameCount = file->Read<uint32>();

	uint32 keyframesCount = file->Read<uint32>();

	for (uint32 i = 0; i < keyframesCount; i++)
	{
		shared_ptr<ModelKeyframe> keyframe = make_shared<ModelKeyframe>();
		keyframe->boneName = Utils::ToWString(file->Read<string>());

		uint32 size = file->Read<uint32>();

		if (size > 0)
		{
			keyframe->transforms.resize(size);
			void* ptr = &keyframe->transforms[0];
			file->Read(&ptr, sizeof(ModelKeyframeData) * size);
		}

		animation->keyframes[keyframe->boneName] = keyframe;
	}

	_stateAnimations[filename] = static_cast<int32>(_animations.size());
	_animations.push_back(animation);
}

std::shared_ptr<Material> Model::GetMaterialByName(const wstring& name)
{
	for (auto& material : _materials)
	{
		if (material->GetName() == name)
			return material;
	}

	return nullptr;
}

std::shared_ptr<ModelMesh> Model::GetMeshByName(const wstring& name)
{
	for (auto& mesh : _meshes)
	{
		if (mesh->name == name)
			return mesh;
	}

	return nullptr;
}

std::shared_ptr<ModelBone> Model::GetBoneByName(const wstring& name)
{
	for (auto& bone : _bones)
	{
		if (bone->name == name)
			return bone;
	}

	return nullptr;
}

std::shared_ptr<ModelAnimation> Model::GetAnimationByName(wstring name)
{
	for (auto& animation : _animations)
	{
		if (animation->name == name)
			return animation;
	}

	return nullptr;
}

std::shared_ptr<ModelAnimation> Model::GetAnimationByFile(wstring filePath)
{
	for (auto& animation : _animations)
	{
		if (animation->filePath == filePath)
			return animation;
	}

	return nullptr;
}

int32 Model::FindAnimation(wstring filename)
{
	auto iter = _stateAnimations.find(filename);

	return (iter != _stateAnimations.end()) ? iter->second : -1;
}

void Model::BindCacheInfo()
{
	// Mesh에 Material 캐싱
	for (const auto& mesh : _meshes)
	{
		// 이미 찾았으면 스킵
		if (mesh->material != nullptr)
			continue;

		mesh->material = GetMaterialByName(mesh->materialName);
	}

	// Mesh에 Bone 캐싱
	for (const auto& mesh : _meshes)
	{
		// 이미 찾았으면 스킵
		if (mesh->bone != nullptr)
			continue;

		mesh->bone = GetBoneByIndex(mesh->boneIndex);
	}

	// Bone 계층 정보 채우기
	if (_root == nullptr && _bones.size() > 0)
	{
		_root = _bones[0];

		for (const auto& bone : _bones)
		{
			if (bone->parentIndex >= 0)
			{
				bone->parent = _bones[bone->parentIndex];
				bone->parent->children.push_back(bone);
			}
			else
			{
				bone->parent = nullptr;
			}
		}
	}
}

void Model::AddCollisionBox(int32 boneIndex, const vector<ModelVertexType>& vertices)
{
	if (boneIndex < 0 ||
		static_cast<uint32>(boneIndex) >= _bones.size() ||
		vertices.empty())
	{
		return;
	}

	const Matrix& nodeMatrix = _bones[boneIndex]->transform;

	ModelCollisionBox box;
	box.boneIndex = boneIndex;
	box.isGround =
		_bones[boneIndex]->name.rfind(L"COL_Floor_", 0) == 0 ||
		_bones[boneIndex]->name.rfind(L"COL_Stair_", 0) == 0;
	box.minPosition = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	box.maxPosition = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (const auto& vertex : vertices)
	{
		Vec3 position = XMVector3TransformCoord(
			vertex.position,
			nodeMatrix
		);

		if (position.x < box.minPosition.x)
			box.minPosition.x = position.x;
		if (position.y < box.minPosition.y)
			box.minPosition.y = position.y;
		if (position.z < box.minPosition.z)
			box.minPosition.z = position.z;

		if (position.x > box.maxPosition.x)
			box.maxPosition.x = position.x;
		if (position.y > box.maxPosition.y)
			box.maxPosition.y = position.y;
		if (position.z > box.maxPosition.z)
			box.maxPosition.z = position.z;
	}

	// 한 노드가 여러 재질의 메시로 분리됐어도
	// 충돌 박스는 하나로 합침
	for (auto& existing : _collisionBoxes)
	{
		if (existing.boneIndex != boneIndex)
			continue;

		if (box.minPosition.x < existing.minPosition.x)
			existing.minPosition.x = box.minPosition.x;
		if (box.minPosition.y < existing.minPosition.y)
			existing.minPosition.y = box.minPosition.y;
		if (box.minPosition.z < existing.minPosition.z)
			existing.minPosition.z = box.minPosition.z;

		if (box.maxPosition.x > existing.maxPosition.x)
			existing.maxPosition.x = box.maxPosition.x;
		if (box.maxPosition.y > existing.maxPosition.y)
			existing.maxPosition.y = box.maxPosition.y;
		if (box.maxPosition.z > existing.maxPosition.z)
			existing.maxPosition.z = box.maxPosition.z;

		return;
	}

	_collisionBoxes.push_back(box);
}

void Model::AddCollisionSlope(int32 boneIndex, const vector<ModelVertexType>& vertices)
{
	if (boneIndex < 0 ||
		static_cast<uint32>(boneIndex) >= _bones.size() ||
		vertices.empty())
	{
		return;
	}

	ModelCollisionSlope* slope = nullptr;

	// 재질 때문에 나뉜 메시도 같은 노드라면 합침
	for (auto& existing : _collisionSlopes)
	{
		if (existing.boneIndex == boneIndex)
		{
			slope = &existing;
			break;
		}
	}

	if (slope == nullptr)
	{
		ModelCollisionSlope data;
		data.boneIndex = boneIndex;

		_collisionSlopes.push_back(data);
		slope = &_collisionSlopes.back();
	}

	const Matrix& nodeMatrix = _bones[boneIndex]->transform;

	for (const auto& vertex : vertices)
	{
		Vec3 point = XMVector3TransformCoord(
			vertex.position,
			nodeMatrix
		);

		// UV나 노멀 분리로 중복된 꼭짓점 제거
		bool duplicate = false;

		for (const Vec3& existing : slope->points)
		{
			if ((existing - point).LengthSquared() < 1e-8f)
			{
				duplicate = true;
				break;
			}
		}

		if (!duplicate)
			slope->points.push_back(point);
	}
}
