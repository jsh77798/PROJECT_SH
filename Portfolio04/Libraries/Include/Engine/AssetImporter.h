#pragma once

#include "AssetImporter.h"

struct ModelBone;
struct ModelMesh;
struct ModelAnimation;

class AssetImporter
{
	DECLARE_SINGLE(AssetImporter);

public:
	wstring MeshImporter(wstring file);
	wstring AnimImporter(wstring file);

private:
	wstring _modelPath = L"../Resources/Models/";
	wstring _texturePath = L"../Resources/Textures/";
};


