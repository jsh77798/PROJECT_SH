#include "pch.h"
#include "StructuredBufferDemo.h"
#include "RawBuffer.h"
#include "StructuredBuffer.h"

void StructuredBufferDemo::Init()
{
	_shader = make_shared<Shader>(L"27. StructuredBufferDemo.fx");

	vector<Matrix> inputs(500, Matrix::Identity);
	
	auto buffer = make_shared<StructuredBuffer>(inputs.data(), sizeof(Matrix), 500, sizeof(Matrix), 500);

	_shader->GetSRV("Input")->SetResource(buffer->GetSRV().Get());
	_shader->GetUAV("Output")->SetUnorderedAccessView(buffer->GetUAV().Get());

	_shader->Dispatch(0, 0, 1, 1, 1);

	vector<Matrix> outputs(500);
	buffer->CopyFromOutput(outputs.data());


}

void StructuredBufferDemo::Update()
{

}

void StructuredBufferDemo::Render()
{

}