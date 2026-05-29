#include "pch.h"
#include "AssetImporter.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include <filesystem>
#include "Converter.h"
#include "Model.h"
#include "Material.h"

void AssetImporter::MeshImporter(wstring file)
{
}

shared_ptr<ModelAnimation> AssetImporter::AnimImporter(wstring file)
{
	wstring converterFile;

	size_t pos = file.rfind(L'.');

	if (pos != std::wstring::npos)
	{
		converterFile = file.erase(pos);
	}

	// Importer
	shared_ptr<class Model> model = make_shared<Model>();
	
	if (!filesystem::exists(_modelPath + converterFile + L".clip"))
	{
		shared_ptr<Converter> converter = make_shared<Converter>();

		converter->ReadAssetFile(file);

		converter->ExportAnimationData(converterFile);

	    return model->ReadAnimation(converterFile);
	}

	return model->ReadAnimation(converterFile);
}
