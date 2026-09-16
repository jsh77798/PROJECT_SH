#include "pch.h"
#include "SnowBillboard.h"
#include "Material.h"
#include "Camera.h"
#include "MathUtils.h"

SnowBillboard::SnowBillboard(Vec3 extent, int32 drawCount /*= 100*/)
	: Super(ComponentType::SnowBillBoard)
{
	
	_desc.extent = extent;
	_desc.drawDistance = _desc.extent.z * 2.0f;
	_drawCount = drawCount;

	const int32 vertexCount = _drawCount * 4;
	_vertices.resize(vertexCount);

	for (int32 i = 0; i < _drawCount * 4; i += 4)
	{
		Vec2 scale = MathUtils::RandomVec2(0.1f, 0.5f);

		Vec3 position;
		position.x = MathUtils::Random(
			-_desc.extent.x * 0.5f,
			_desc.extent.x * 0.5f
		);

		position.y = MathUtils::Random(
			-_desc.extent.y * 0.5f,
			_desc.extent.y * 0.5f
		);

		position.z = MathUtils::Random(
			-_desc.extent.z * 0.5f,
			_desc.extent.z * 0.5f
		);

		Vec2 random = MathUtils::RandomVec2(0.0f, 1.0f);

		_vertices[i + 0].position = position;
		_vertices[i + 1].position = position;
		_vertices[i + 2].position = position;
		_vertices[i + 3].position = position;

		_vertices[i + 0].uv = Vec2(0, 1);
		_vertices[i + 1].uv = Vec2(0, 0);
		_vertices[i + 2].uv = Vec2(1, 1);
		_vertices[i + 3].uv = Vec2(1, 0);

		_vertices[i + 0].scale = scale;
		_vertices[i + 1].scale = scale;
		_vertices[i + 2].scale = scale;
		_vertices[i + 3].scale = scale;

		_vertices[i + 0].random = random;
		_vertices[i + 1].random = random;
		_vertices[i + 2].random = random;
		_vertices[i + 3].random = random;
	}

	_vertexBuffer = make_shared<VertexBuffer>();
	_vertexBuffer->Create(_vertices, 0);

	const int32 indexCount = _drawCount * 6;
	_indices.resize(indexCount);

	for (int32 i = 0; i < _drawCount; i++)
	{
		_indices[i * 6 + 0] = i * 4 + 0;
		_indices[i * 6 + 1] = i * 4 + 1;
		_indices[i * 6 + 2] = i * 4 + 2;
		_indices[i * 6 + 3] = i * 4 + 2;
		_indices[i * 6 + 4] = i * 4 + 1;
		_indices[i * 6 + 5] = i * 4 + 3;
	}

	_indexBuffer = make_shared<IndexBuffer>();
	_indexBuffer->Create(_indices);
}

SnowBillboard::~SnowBillboard()
{

}

void SnowBillboard::Update()
{
	//_desc.origin = CUR_SCENE->GetMainCamera()->GetTransform()->GetPosition();
	//_desc.time = _elpasedTime;
	//_elpasedTime += DT;
	//
	//auto shader = _material->GetShader();
	//
	//// Transform
	//auto world = GetTransform()->GetWorldMatrix();
	//shader->PushTransformData(TransformDesc{ world });
	//
	//// GlobalData
	//shader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);
	//
	//// SnowData
	//shader->PushSnowData(_desc);
	//
	//// Light
	//_material->Update();
	//
	//// IA
	//_vertexBuffer->PushData();
	//_indexBuffer->PushData();
	//DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//
	//shader->DrawIndexed(0, _pass, _drawCount * 6);

	_elpasedTime += DT;
	_desc.time = _elpasedTime;
}

void SnowBillboard::Render(const Vec3& cameraPosition)
{
	if (_material == nullptr)
		return;

	auto shader = _material->GetShader();

	if (shader == nullptr)
		return;

	// 추적 처리가 끝난, 현재 렌더링 카메라 위치
	_desc.origin = cameraPosition;

	shader->PushTransformData(
		TransformDesc{ GetTransform()->GetWorldMatrix() }
	);

	shader->PushGlobalData(
		Camera::S_MatView,
		Camera::S_MatProjection
	);

	shader->PushSnowData(_desc);

	_material->Update();

	// 눈 셰이더가 변경할 렌더 상태 보관
	ComPtr<ID3D11BlendState> oldBlend;
	FLOAT oldBlendFactor[4] = {};
	UINT oldSampleMask = 0;

	DC->OMGetBlendState(
		oldBlend.GetAddressOf(),
		oldBlendFactor,
		&oldSampleMask
	);

	ComPtr<ID3D11DepthStencilState> oldDepth;
	UINT oldStencilRef = 0;

	DC->OMGetDepthStencilState(
		oldDepth.GetAddressOf(),
		&oldStencilRef
	);

	ComPtr<ID3D11RasterizerState> oldRasterizer;
	DC->RSGetState(oldRasterizer.GetAddressOf());

	D3D11_PRIMITIVE_TOPOLOGY oldTopology;
	DC->IAGetPrimitiveTopology(&oldTopology);

	_vertexBuffer->PushData();
	_indexBuffer->PushData();

	DC->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	);

	shader->DrawIndexed(
		0,
		_pass,
		_indexBuffer->GetCount()
	);

	// 다음 물체에 눈의 블렌딩·깊이 설정이 남지 않도록 복원
	DC->OMSetBlendState(
		oldBlend.Get(),
		oldBlendFactor,
		oldSampleMask
	);

	DC->OMSetDepthStencilState(
		oldDepth.Get(),
		oldStencilRef
	);

	DC->RSSetState(oldRasterizer.Get());
	DC->IASetPrimitiveTopology(oldTopology);
}
