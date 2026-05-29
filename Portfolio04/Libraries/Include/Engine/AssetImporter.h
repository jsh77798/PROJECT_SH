#pragma once

#include "AssetImporter.h"

struct ModelBone;
struct ModelMesh;
struct ModelAnimation;

class AssetImporter
{
	DECLARE_SINGLE(AssetImporter);

public:
	void MeshImporter(wstring file);
	shared_ptr<ModelAnimation> AnimImporter(wstring file);

private:
	wstring _modelPath = L"../Resources/Models/";
	wstring _texturePath = L"../Resources/Textures/";
};


