#include "pch.h"
#include "AssetImporter.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include <filesystem>
#include "Converter.h"
#include "Model.h"
#include "Material.h"

wstring AssetImporter::MeshImporter(wstring file)
{
	wstring converterFile = file;

	size_t pos = converterFile.rfind(L'/');

	if (pos != std::wstring::npos)
	{
		converterFile.erase(pos);
	}

	// Importer
	shared_ptr<class Model> model = make_shared<Model>();

	if (!filesystem::exists(_modelPath + converterFile + L".mesh"))
	{
		shared_ptr<Converter> converter = make_shared<Converter>();

		converter->ReadAssetFile(file);

		converter->ExportModelData(converterFile);
	}

	if (!filesystem::exists(_texturePath + converterFile + L"/" + converterFile + L".xml"))
	{
		shared_ptr<Converter> converter = make_shared<Converter>();

		converter->ReadAssetFile(file);

		converter->ExportMaterialData(converterFile);
	}
	
	return converterFile;
}

wstring AssetImporter::AnimImporter(wstring file)
{
	wstring converterFile = file;

	size_t pos = converterFile.rfind(L'.');

	if (pos != std::wstring::npos)
	{
		converterFile.erase(pos);
	}

	// Importer
	shared_ptr<class Model> model = make_shared<Model>();
	
	if (!filesystem::exists(_modelPath + converterFile + L".clip"))
	{
		shared_ptr<Converter> converter = make_shared<Converter>();

		converter->ReadAssetFile(file);

		converter->ExportAnimationData(converterFile);
	}

	return converterFile;
}
