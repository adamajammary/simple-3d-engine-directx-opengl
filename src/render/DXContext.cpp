#if defined _WINDOWS

#include "DXContext.h"

DXContext::DXContext(GraphicsAPI api, bool vsync)
{
	switch (api) {
		case GRAPHICS_API_DIRECTX11: this->isOK = this->init11(vsync); break;
		case GRAPHICS_API_DIRECTX12: this->isOK = this->init12(vsync); break;
		default: throw;
	}

	if (!this->isOK)
		this->release();
}

DXContext::~DXContext()
{
	this->release();
}

void DXContext::Bind11(FBOType fboType, Texture* texture)
{
	UINT           nrOfRenderTargets = (fboType == FBO_DEPTH ? 0 : 1);
	D3D11_VIEWPORT viewPort          = texture->BufferViewPort11();

	this->deviceContext->RSSetViewports(1, &viewPort);
	this->deviceContext->OMSetRenderTargets(nrOfRenderTargets, &texture->ColorBuffer11, texture->DepthBuffer11);
}

void DXContext::Bind12(FBOType fboType, Texture* texture)
{
	this->commandsInit();

	UINT                           nrOfRenderTargets;
	D3D12_RESOURCE_STATES          resourceStateOld   = (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CD3DX12_CPU_DESCRIPTOR_HANDLE* colorBufferHandle  = nullptr;
	CD3DX12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle = nullptr;
	D3D12_RECT                     scissor            = {};
	D3D12_VIEWPORT                 viewPort           = texture->BufferViewPort12();

	scissor.right  = (LONG)viewPort.Width;
	scissor.bottom = (LONG)viewPort.Height;

	switch (fboType) {
	case FBO_COLOR:
		colorBufferHandle = &CD3DX12_CPU_DESCRIPTOR_HANDLE(texture->ColorBuffer12->GetCPUDescriptorHandleForHeapStart());
		nrOfRenderTargets = 1;

		this->transitionResource(texture->Resource12, resourceStateOld, D3D12_RESOURCE_STATE_RENDER_TARGET);

		break;
	case FBO_DEPTH:
		depthStencilHandle = &CD3DX12_CPU_DESCRIPTOR_HANDLE(texture->DepthBuffer12->GetCPUDescriptorHandleForHeapStart());
		nrOfRenderTargets  = 0;

		this->transitionResource(texture->Resource12, resourceStateOld, D3D12_RESOURCE_STATE_DEPTH_WRITE);

		break;
	default:
		throw;
	}

	this->commandList->RSSetViewports(1, &viewPort);
	this->commandList->RSSetScissorRects(1, &scissor);
	this->commandList->OMSetRenderTargets(nrOfRenderTargets, colorBufferHandle, TRUE, depthStencilHandle);
}

void DXContext::Unbind11()
{
	this->deviceContext->RSSetViewports(1,     &this->viewPort11);
	this->deviceContext->OMSetRenderTargets(1, &this->colorBuffer, this->depthStencilBuffer11);
}

void DXContext::Unbind12(FBOType fboType, Texture* texture)
{
	D3D12_RESOURCE_STATES resourceStateNew = (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	switch (fboType) {
	case FBO_COLOR:
		this->transitionResource(texture->Resource12, D3D12_RESOURCE_STATE_RENDER_TARGET, resourceStateNew);
		break;
	case FBO_DEPTH:
		this->transitionResource(texture->Resource12, D3D12_RESOURCE_STATE_DEPTH_WRITE, resourceStateNew);
		break;
	default:
		throw;
	}

	this->commandsExecute();
	this->wait();
}

void DXContext::Clear11(const glm::vec4 &colorRGBA)
{
	this->deviceContext->RSSetViewports(1,     &this->viewPort11);
	this->deviceContext->OMSetRenderTargets(1, &this->colorBuffer, this->depthStencilBuffer11);

	this->deviceContext->ClearRenderTargetView(this->colorBuffer, &colorRGBA[0]);
	this->deviceContext->ClearDepthStencilView(this->depthStencilBuffer11, (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL), 1.0f, 0);
}

void DXContext::Clear11(const glm::vec4 &colorRGBA, FrameBuffer* fbo)
{
	if (fbo == nullptr)
		this->Clear11(colorRGBA);
	else if (fbo->Type() == FBO_COLOR)
		this->deviceContext->ClearRenderTargetView(fbo->GetTexture()->ColorBuffer11, &colorRGBA[0]);
	else if (fbo->Type() == FBO_DEPTH)
		this->deviceContext->ClearDepthStencilView(fbo->GetTexture()->DepthBuffer11, D3D11_CLEAR_DEPTH, 1.0f, 0);
		//this->deviceContext->ClearDepthStencilView(fbo->GetTexture()->DepthBuffer11, (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL), 1.0f, 0);
}

void DXContext::Clear12(const glm::vec4 &colorRGBA)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE colorBufferHandle(this->colorBufferHeap->GetCPUDescriptorHandleForHeapStart(), this->colorBufferIndex, this->colorBufferSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilHandle(this->depthStencilBufferHeap->GetCPUDescriptorHandleForHeapStart());

	this->commandsInit();

	this->transitionResource(
		this->colorBuffers[this->colorBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	this->commandList->RSSetViewports(1,     &this->viewPort12);
	this->commandList->RSSetScissorRects(1,  &this->scissorRect);
	this->commandList->OMSetRenderTargets(1, &colorBufferHandle, TRUE, &depthStencilHandle);

	this->commandList->ClearRenderTargetView(colorBufferHandle, &colorRGBA[0], 0, nullptr);
	this->commandList->ClearDepthStencilView(depthStencilHandle, (D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL), 1.0f, 0, 0, nullptr);
}

void DXContext::Clear12(const glm::vec4 &colorRGBA, FrameBuffer* fbo)
{
	if (fbo == nullptr)
	{
		this->Clear12(colorRGBA);
	}
	else if (fbo->Type() == FBO_COLOR)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE colorBufferHandle(fbo->GetTexture()->ColorBuffer12->GetCPUDescriptorHandleForHeapStart());

		this->commandList->ClearRenderTargetView(colorBufferHandle, &colorRGBA[0], 0, nullptr);
	}
	else if (fbo->Type() == FBO_DEPTH)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilHandle(fbo->GetTexture()->DepthBuffer12->GetCPUDescriptorHandleForHeapStart());

		this->commandList->ClearDepthStencilView(depthStencilHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//this->commandList->ClearDepthStencilView(depthStencilHandle, (D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL), 1.0f, 0, 0, nullptr);
	}
}

int DXContext::commandsExecute()
{
	HRESULT result = this->commandList->Close();

	if (FAILED(result))
		return -1;

	ID3D12CommandList* commandLists[] = { this->commandList };
	this->commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	return 0;
}

int DXContext::commandsInit()
{
	HRESULT result = -1;

	result = this->commandAllocator->Reset();

	if (FAILED(result))
		return -1;

	result = this->commandList->Reset(this->commandAllocator, nullptr);

	if (FAILED(result))
		return -1;

	return 0;
}

int DXContext::compileShader(const wxString &file, ID3DBlob** vs, ID3DBlob** fs)
{
	ID3DBlob* error = nullptr;
	DWORD     flags = 0;

	#if defined _DEBUG
		flags |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ENABLE_STRICTNESS);
    #endif

	HRESULT result = D3DCompileFromFile(file.wc_str(), nullptr, nullptr, "VS", "vs_5_0", flags, 0, vs, &error);

	if (FAILED(result))
	{
		#if defined _DEBUG
		if (error != nullptr) {
			wxMessageBox((char*)error->GetBufferPointer());
			wxLogDebug("%s\n", (char*)error->GetBufferPointer());
			_RELEASEP(error);
		}
		#endif

		return -1;
	}

	result = D3DCompileFromFile(file.wc_str(), nullptr, nullptr, "PS", "ps_5_0", flags, 0, fs, &error);

	if (FAILED(result))
	{
		#if defined _DEBUG
		if (error != nullptr) {
			wxMessageBox((char*)error->GetBufferPointer());
			wxLogDebug("%s\n", (char*)error->GetBufferPointer());
			_RELEASEP(error);
		}
		#endif

		return -2;
	}

	return 0;
}

int DXContext::CreateConstantBuffers11(Buffer* buffer)
{
	if (buffer == nullptr)
		return -1;

	for (int i = 0; i < NR_OF_SHADERS; i++) {
		_RELEASEP(buffer->ConstantBuffersDX11[i]);
	}

	UINT bufferSizes[NR_OF_SHADERS];

	bufferSizes[SHADER_ID_COLOR]     = sizeof(CBColorDX);
	bufferSizes[SHADER_ID_DEFAULT]   = sizeof(CBDefaultDX);
	bufferSizes[SHADER_ID_DEPTH]     = sizeof(CBSkyboxDX);
	bufferSizes[SHADER_ID_HUD]       = sizeof(CBHUDDX);
	bufferSizes[SHADER_ID_SKYBOX]    = sizeof(CBSkyboxDX);
	bufferSizes[SHADER_ID_TERRAIN]   = sizeof(CBDefaultDX);
	bufferSizes[SHADER_ID_WATER]     = sizeof(CBWaterDX);
	bufferSizes[SHADER_ID_WIREFRAME] = sizeof(CBColorDX);

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	for (int i = 0; i < NR_OF_SHADERS; i++)
	{
		bufferDesc.ByteWidth = bufferSizes[i];

		if (FAILED(this->renderDevice11->CreateBuffer(&bufferDesc, nullptr, &buffer->ConstantBuffersDX11[i])))
			return -2;
	}

	return 0;
}

int DXContext::CreateConstantBuffers12(Buffer* buffer)
{
	if (buffer == nullptr)
		return -1;

	for (int i = 0; i < NR_OF_SHADERS; i++) {
		_RELEASEP(buffer->ConstantBuffersDX12[i]);
	}

	for (int i = 0; i < NR_OF_SHADERS; i++) {
		_RELEASEP(buffer->ConstantBufferHeapsDX12[i]);
	}

	D3D12_DESCRIPTOR_HEAP_DESC bufferHeapDesc  = {};

	bufferHeapDesc.NumDescriptors = (1 + MAX_TEXTURES + (2 * MAX_LIGHT_SOURCES));
	bufferHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	bufferHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};

	samplerHeapDesc.NumDescriptors = (MAX_TEXTURES + 2);
	samplerHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	for (int i = 0; i < NR_OF_SHADERS; i++)
	{
		HRESULT result = this->renderDevice12->CreateDescriptorHeap(
			&bufferHeapDesc, IID_PPV_ARGS(&buffer->ConstantBufferHeapsDX12[i])
		);

		if (FAILED(result))
			return -2;

		result = this->renderDevice12->CreateDescriptorHeap(
			&samplerHeapDesc, IID_PPV_ARGS(&buffer->SamplerHeapsDX12[i])
		);

		if (FAILED(result))
			return -3;

		// https://social.msdn.microsoft.com/Forums/sqlserver/en-US/78dec419-6827-40fc-a048-01c3cccb92ef/directx-10-11-and-12-constant-buffer-alignment?forum=vsga
		// Buffer View = 64 KB aligned = 65536 bytes
		// Buffer Data = 256 byte aligned
		result = this->renderDevice12->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(BYTE_ALIGN_BUFFER_DATA), D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&buffer->ConstantBuffersDX12[i])
		);

		if (FAILED(result))
			return -4;

		UINT bufferSizes[NR_OF_SHADERS];

		bufferSizes[SHADER_ID_COLOR]     = sizeof(CBColorDX);
		bufferSizes[SHADER_ID_DEFAULT]   = sizeof(CBDefaultDX);
		bufferSizes[SHADER_ID_DEPTH]     = sizeof(CBSkyboxDX);
		bufferSizes[SHADER_ID_HUD]       = sizeof(CBHUDDX);
		bufferSizes[SHADER_ID_SKYBOX]    = sizeof(CBSkyboxDX);
		bufferSizes[SHADER_ID_TERRAIN]   = sizeof(CBDefaultDX);
		bufferSizes[SHADER_ID_WATER]     = sizeof(CBWaterDX);
		bufferSizes[SHADER_ID_WIREFRAME] = sizeof(CBColorDX);

		D3D12_CONSTANT_BUFFER_VIEW_DESC bufferDesc = {};

		bufferDesc.BufferLocation = buffer->ConstantBuffersDX12[i]->GetGPUVirtualAddress();
		bufferDesc.SizeInBytes    = (bufferSizes[i] + (BYTE_ALIGN_BUFFER_VIEW - (bufferSizes[i] % BYTE_ALIGN_BUFFER_VIEW)));

		this->renderDevice12->CreateConstantBufferView(
			&bufferDesc, buffer->ConstantBufferHeapsDX12[i]->GetCPUDescriptorHandleForHeapStart()
		);

		this->wait();
	}

	return 0;
}

int DXContext::CreateIndexBuffer11(std::vector<unsigned int> &indices, Buffer* buffer)
{
	D3D11_SUBRESOURCE_DATA bufferData = {};
	D3D11_BUFFER_DESC      bufferDesc = {};

	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = (indices.size() * sizeof(unsigned int));
	bufferData.pSysMem   = &indices[0];

	if (FAILED(this->renderDevice11->CreateBuffer(&bufferDesc, &bufferData, &buffer->VertexBufferDX11)))
		return -1;

	return 0;
}

int DXContext::CreateIndexBuffer12(std::vector<unsigned int> &indices, Buffer* buffer)
{
	UINT bufferSize = (indices.size() * sizeof(unsigned int));

	HRESULT result = this->renderDevice12->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer->VertexBufferDX12)
	);

	if (FAILED(result))
		return -1;

	ID3D12Resource* bufferResource = nullptr;

	result = this->renderDevice12->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&bufferResource)
	);

	if (FAILED(result))
		return -2;

	D3D12_SUBRESOURCE_DATA bufferData = {};

	bufferData.pData      = &indices[0];
	bufferData.RowPitch   = bufferSize;
	bufferData.SlicePitch = bufferSize;

	buffer->IndexBufferViewDX12.BufferLocation = buffer->VertexBufferDX12->GetGPUVirtualAddress();
	buffer->IndexBufferViewDX12.Format         = DXGI_FORMAT_R32_UINT;
	buffer->IndexBufferViewDX12.SizeInBytes    = bufferSize;

	this->commandsInit();

	UpdateSubresources(this->commandList, buffer->VertexBufferDX12, bufferResource, 0, 0, 1, &bufferData);

	this->commandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer->VertexBufferDX12, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
	);

	this->commandsExecute();
	this->wait();

	_RELEASEP(bufferResource);

	return 0;
}

int DXContext::createPipeline(
	ShaderProgram*        shaderProgram,
	ID3D12PipelineState** pipeline,
	ID3D12RootSignature** rootSignature,
	FBOType               fboType,
	const std::vector<D3D12_INPUT_ELEMENT_DESC> &attribsDescs
)
{
	if (shaderProgram == nullptr)
		return -1;

	ID3D10Blob* fs       = shaderProgram->FS();
	ID3D10Blob* vs       = shaderProgram->VS();
	ShaderID    shaderID = shaderProgram->ID();

	if ((vs == nullptr) || (fs == nullptr))
		return -2;

	if (FAILED(this->createRootSignature(shaderProgram, rootSignature)))
		return -3;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};

	pipelineStateDesc.pRootSignature = *rootSignature;

	pipelineStateDesc.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	pipelineStateDesc.PS = { fs->GetBufferPointer(), fs->GetBufferSize() };

	pipelineStateDesc.DSVFormat        = DXGI_FORMAT_D32_FLOAT;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0]    = (fboType == FBO_UNKNOWN ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	if (fboType == FBO_COLOR)
		pipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	if (shaderID == SHADER_ID_DEPTH) {
		pipelineStateDesc.NumRenderTargets = 0;
		pipelineStateDesc.RTVFormats[0]    = DXGI_FORMAT_UNKNOWN;
	}

	pipelineStateDesc.InputLayout      = { attribsDescs.data(), (UINT)attribsDescs.size() };
	pipelineStateDesc.SampleMask       = UINT_MAX;
	pipelineStateDesc.SampleDesc.Count = 1;
	//pipelineStateDesc.SampleDesc.Count = this->multiSampleCount;	// TODO: MSAA DX12

	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	switch(shaderID) {
	case SHADER_ID_HUD:
		pipelineStateDesc.BlendState        = this->initColorBlending12(TRUE);
		pipelineStateDesc.DepthStencilState = this->initDepthStencilBuffer12(FALSE);
		pipelineStateDesc.RasterizerState   = this->initRasterizer12(D3D12_CULL_MODE_NONE);
		break;
	case SHADER_ID_SKYBOX:
		pipelineStateDesc.BlendState        = this->initColorBlending12(FALSE);
		pipelineStateDesc.DepthStencilState = this->initDepthStencilBuffer12(TRUE, D3D12_COMPARISON_FUNC_LESS_EQUAL);
		pipelineStateDesc.RasterizerState   = this->initRasterizer12(D3D12_CULL_MODE_NONE);
		break;
	default:
		pipelineStateDesc.BlendState        = this->initColorBlending12(FALSE);
		pipelineStateDesc.DepthStencilState = this->initDepthStencilBuffer12(TRUE);
		pipelineStateDesc.RasterizerState   = this->initRasterizer12();
		break;
	}

	switch (shaderID) {
	case SHADER_ID_WIREFRAME:
		pipelineStateDesc.PrimitiveTopologyType    = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		break;
	case SHADER_ID_DEPTH:
		pipelineStateDesc.RasterizerState.DepthClipEnable = FALSE;
		pipelineStateDesc.RasterizerState.CullMode        = D3D12_CULL_MODE_FRONT;
		break;
	default:
		break;
	}

	if (FAILED(this->renderDevice12->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipeline))))
		return -4;

	return 0;
}

int DXContext::createRootSignature(ShaderProgram* shader, ID3D12RootSignature** rootSignature)
{
	// ROOT SIGNATURE
	ID3DBlob*                           error                                  = nullptr;
	D3D12_FEATURE_DATA_ROOT_SIGNATURE   featureData                            = {};
	UINT                                nrOfParameters                         = 0;
	UINT                                nrOfSRVs                               = 0;
	UINT                                nrOfSamplers                           = 0;
	D3D12_DESCRIPTOR_RANGE1             ranges[NR_OF_ROOT_SIGNATURE_TYPES]     = {};
	D3D12_ROOT_PARAMETER1               rootParams[NR_OF_ROOT_SIGNATURE_TYPES] = {};
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc                      = {};
	ShaderID                            shaderID                               = shader->ID();
	ID3DBlob*                           signature                              = nullptr;

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	HRESULT result = this->renderDevice12->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));

	if (FAILED(result))
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

	// MESH TEXTURES
	switch (shaderID) {
	case SHADER_ID_DEFAULT:
	case SHADER_ID_HUD:
	case SHADER_ID_SKYBOX:
	case SHADER_ID_TERRAIN:
	case SHADER_ID_WATER:
		nrOfSRVs     += MAX_TEXTURES;
		nrOfSamplers += MAX_TEXTURES;
		break;
	default:
		break;
	}

	// DEPTH MAP TEXTURES
	if (shaderID == SHADER_ID_DEFAULT) {
		nrOfSRVs     += (2 * MAX_LIGHT_SOURCES);
		nrOfSamplers += 2;
	}

	for (uint32_t i = 0; i < NR_OF_ROOT_SIGNATURE_TYPES; i++)
	{
		ranges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		switch (i) {
		case ROOT_CBV:
			ranges[i].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			ranges[i].NumDescriptors     = 1;
			ranges[i].Flags              = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
			break;
		case ROOT_TEXTURE_SRV:
			ranges[i].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[i].NumDescriptors     = nrOfSRVs;
			ranges[i].Flags              = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
			break;
		case ROOT_TEXTURE_SAMPLER:
			ranges[i].RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
			ranges[i].NumDescriptors     = nrOfSamplers;
			ranges[i].Flags              = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
			break;
		default:
			throw;
		}

		rootParams[i].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[i].DescriptorTable  = { 1, &ranges[i] };
		rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		nrOfParameters++;

		if ((nrOfSRVs == 0) || (nrOfSamplers == 0))
			break;
	}

	// ALLOW INPUT LAYOUT AND DENY ACCESS UNNECESSARY TO PIPELINE STAGES
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = (
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS       |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS     |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
		//D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
		//D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT
	);
	//rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	rootSignatureDesc.Version = featureData.HighestVersion;

	if (rootSignatureDesc.Version == D3D_ROOT_SIGNATURE_VERSION_1_1) {
		rootSignatureDesc.Desc_1_1.NumParameters = nrOfParameters;
		rootSignatureDesc.Desc_1_1.pParameters   = &rootParams[0];
		rootSignatureDesc.Desc_1_1.Flags         = rootSignatureFlags;
	} else {
		rootSignatureDesc.Desc_1_0.NumParameters = nrOfParameters;
		rootSignatureDesc.Desc_1_0.pParameters   = reinterpret_cast<D3D12_ROOT_PARAMETER*>(&rootParams[0]);
		rootSignatureDesc.Desc_1_0.Flags         = rootSignatureFlags;
	}

	result = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);

	if (FAILED(result))
		return -1;

	result = this->renderDevice12->CreateRootSignature(
		0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature)
	);

	if (FAILED(result))
		return -2;

	return 0;
}

int DXContext::CreateShader11(const wxString &file, ID3DBlob** vs, ID3DBlob** fs, ID3D11VertexShader** shaderVS, ID3D11PixelShader** shaderFS)
{
	if (FAILED(this->compileShader(file, vs, fs)))
		return -1;

	if (FAILED(this->renderDevice11->CreateVertexShader((*vs)->GetBufferPointer(), (*vs)->GetBufferSize(), NULL, shaderVS)))
		return -2;

	if (FAILED(this->renderDevice11->CreatePixelShader((*fs)->GetBufferPointer(), (*fs)->GetBufferSize(), NULL, shaderFS)))
		return -3;

	return 0;
}

int DXContext::CreateShader12(const wxString &file, ID3DBlob** vs, ID3DBlob** fs)
{
	return this->compileShader(file, vs, fs);
}

int DXContext::CreateTexture11(FBOType fboType, const std::vector<BYTE*> &pixels, DXGI_FORMAT format, D3D11_SAMPLER_DESC &samplerDesc, Texture* texture)
{
	if (texture == nullptr)
		return -1;

	size_t      dataCount;
	UINT        flags;
	UINT        mipLevels;
	UINT        arraySize   = (fboType == FBO_UNKNOWN ? (UINT)pixels.size() : 1);
	UINT        bindFlags   = D3D11_BIND_SHADER_RESOURCE;
	TextureType textureType = texture->GetTextureType();

	switch (fboType) {
		case FBO_DEPTH: bindFlags |= D3D11_BIND_DEPTH_STENCIL; break;
		default:        bindFlags |= D3D11_BIND_RENDER_TARGET; break;
	}

	switch (textureType) {
	case TEXTURE_2D:
		dataCount = (fboType == FBO_UNKNOWN ? texture->MipLevels()              : 1);
		flags     = (fboType == FBO_UNKNOWN ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
		mipLevels = dataCount;
		break;
	case TEXTURE_CUBEMAP:
		dataCount = pixels.size();
		flags     = D3D11_RESOURCE_MISC_TEXTURECUBE;
		mipLevels = 1;
		break;
	default:
		throw;
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	wxSize               textureSize = texture->Size();

	textureDesc.ArraySize        = arraySize;
	textureDesc.BindFlags        = bindFlags;
	textureDesc.Format           = format;
	textureDesc.MipLevels        = mipLevels;
	textureDesc.MiscFlags        = flags;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Width            = (UINT)textureSize.GetWidth();
	textureDesc.Height           = (UINT)textureSize.GetHeight();

	if ((textureDesc.Width == 0) || (textureDesc.Height == 0))
	{
		D3D11_VIEWPORT bufferViewPort = texture->BufferViewPort11();

		textureDesc.Width  = (UINT)bufferViewPort.Width;
		textureDesc.Height = (UINT)bufferViewPort.Height;
	}

	HRESULT result;

	if (fboType == FBO_UNKNOWN)
	{
		std::vector<D3D11_SUBRESOURCE_DATA> textureData(dataCount);

		for (size_t i = 0; i < pixels.size(); i++)
		{
			for (uint32_t j = 0; j < textureDesc.MipLevels; j++)
			{
				textureData[i + j].pSysMem          = pixels[i];
				textureData[i + j].SysMemPitch      = (textureDesc.Width * 4);
				textureData[i + j].SysMemSlicePitch = (textureDesc.Width * textureDesc.Height * 4);
			}
		}

		result = this->renderDevice11->CreateTexture2D(&textureDesc, textureData.data(), &texture->Resource11);
	} else {
		result = this->renderDevice11->CreateTexture2D(&textureDesc, nullptr, &texture->Resource11);
	}

	if (FAILED(result))
		return -2;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = (fboType == FBO_DEPTH ? DXGI_FORMAT_R32_FLOAT : textureDesc.Format);

	switch (textureType) {
	case TEXTURE_2D:
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = (fboType == FBO_UNKNOWN ? -1 : 1);
		break;
	case TEXTURE_CUBEMAP:
		srvDesc.ViewDimension         = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		break;
	default:
		throw;
	}

	result = this->renderDevice11->CreateShaderResourceView(texture->Resource11, &srvDesc, &texture->SRV11);

	if (FAILED(result))
		return -3;

	if ((textureType == TEXTURE_2D) && (fboType == FBO_UNKNOWN))
		this->deviceContext->GenerateMips(texture->SRV11);

	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxAnisotropy  = 16;
	samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

	result = this->renderDevice11->CreateSamplerState(&samplerDesc, &texture->SamplerState11);

	if (FAILED(result))
		return -4;

	return 0;
}

int DXContext::CreateTextureBuffer11(FBOType fboType, DXGI_FORMAT format, D3D11_SAMPLER_DESC &samplerDesc, Texture* texture)
{
	if (texture == nullptr)
		return -1;

	std::vector<BYTE*> pixels;
	HRESULT            result = this->CreateTexture11(fboType, pixels, format, samplerDesc, texture);

	if (FAILED(result))
		return -2;

	// FRAME BUFFER
	D3D11_RENDER_TARGET_VIEW_DESC colorBufferDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC depthBufferDesc = {};

	switch (fboType) {
	case FBO_COLOR:
		colorBufferDesc.Format        = format;
		colorBufferDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		result = this->renderDevice11->CreateRenderTargetView(texture->Resource11, &colorBufferDesc, &texture->ColorBuffer11);

		break;
	case FBO_DEPTH:
		depthBufferDesc.Format        = DXGI_FORMAT_D32_FLOAT;
		depthBufferDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		result = this->renderDevice11->CreateDepthStencilView(texture->Resource11, &depthBufferDesc, &texture->DepthBuffer11);

		break;
	default:
		throw;
	}

	if (FAILED(result))
		return -3;

	return 0;
}

// TODO: Generate MipMaps
int DXContext::CreateTexture12(FBOType fboType, const std::vector<BYTE*> &pixels, DXGI_FORMAT format, Texture* texture)
{
	if (texture == nullptr)
		return -1;

	size_t                dataCount;
	UINT                  mipLevels;
	UINT16                arraySize     = (fboType == FBO_UNKNOWN ? (UINT16)pixels.size() : 1);
	D3D12_RESOURCE_STATES shaderState   = (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	D3D12_RESOURCE_STATES resourceState = shaderState;
	D3D12_RESOURCE_FLAGS  resourceFlags = D3D12_RESOURCE_FLAG_NONE;
	TextureType           textureType   = texture->GetTextureType();

	switch (fboType) {
		case FBO_COLOR: resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; break;
		case FBO_DEPTH: resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; break;
		default:        resourceState = D3D12_RESOURCE_STATE_COPY_DEST;          break;
	}

	switch (textureType) {
	case TEXTURE_2D:
		//dataCount = (fboType == FBO_UNKNOWN ? texture->MipLevels() : 1);
		dataCount = 1;
		mipLevels = dataCount;
		break;
	case TEXTURE_CUBEMAP:
		dataCount = pixels.size();
		mipLevels = 1;
		break;
	default:
		throw;
	}

	D3D12_RESOURCE_DESC textureResourceDesc = {};
	wxSize              textureSize         = texture->Size();

	textureResourceDesc.DepthOrArraySize = arraySize;
	textureResourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureResourceDesc.Flags            = resourceFlags;
	textureResourceDesc.Format           = format;
	textureResourceDesc.MipLevels        = mipLevels;
	textureResourceDesc.SampleDesc.Count = 1;
	textureResourceDesc.Width            = (UINT64)textureSize.GetWidth();
	textureResourceDesc.Height           = (UINT64)textureSize.GetHeight();

	if ((textureResourceDesc.Width == 0) || (textureResourceDesc.Height == 0))
	{
		D3D12_VIEWPORT bufferViewPort = texture->BufferViewPort12();

		textureResourceDesc.Width  = (UINT64)bufferViewPort.Width;
		textureResourceDesc.Height = (UINT64)bufferViewPort.Height;
	}

	D3D12_CLEAR_VALUE* clearValue;

	switch (fboType) {
		case FBO_COLOR: clearValue = &this->getClearValueDX12(fboType, textureResourceDesc.Format); break;
		case FBO_DEPTH: clearValue = &this->getClearValueDX12(fboType, DXGI_FORMAT_D32_FLOAT);      break;
		default:        clearValue = nullptr; break;
	}

	HRESULT result = this->renderDevice12->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &textureResourceDesc,
		resourceState, clearValue, IID_PPV_ARGS(&texture->Resource12)
	);

	if (FAILED(result))
		return -2;

	//if (!pixels.empty())
	if (fboType == FBO_UNKNOWN)
	{
		std::vector<D3D12_SUBRESOURCE_DATA> textureData(dataCount);
		ID3D12Resource*                     textureResource = nullptr;

		result = this->renderDevice12->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(GetRequiredIntermediateSize(texture->Resource12, 0, textureData.size())),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureResource)
		);

		if (FAILED(result))
			return -3;

		for (size_t i = 0; i < pixels.size(); i++)
		{
			textureData[i].pData      = pixels[i];
			textureData[i].RowPitch   = (textureResourceDesc.Width * 4);
			textureData[i].SlicePitch = (textureResourceDesc.Width * textureResourceDesc.Height * 4);
		}

		this->commandsInit();

		UpdateSubresources(
			this->commandList, texture->Resource12, textureResource, 0, 0, textureData.size(), textureData.data()
		);

		this->transitionResource(texture->Resource12, resourceState, shaderState);

		this->commandsExecute();
		this->wait();

		_RELEASEP(textureResource);
	}

	texture->SRVDesc12.Format                  = (fboType == FBO_DEPTH ? DXGI_FORMAT_R32_FLOAT : textureResourceDesc.Format);
	texture->SRVDesc12.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	switch (textureType) {
	case TEXTURE_2D:
		texture->SRVDesc12.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE2D;
		//texture->SRVDesc12.Texture2D.MipLevels = (fboType == FBO_UNKNOWN ? -1 : 1);
		texture->SRVDesc12.Texture2D.MipLevels = 1;
		break;
	case TEXTURE_CUBEMAP:
		texture->SRVDesc12.ViewDimension         = D3D12_SRV_DIMENSION_TEXTURECUBE;
		texture->SRVDesc12.TextureCube.MipLevels = 1;
		break;
	default:
		throw;
	}
	
	texture->SamplerDesc12.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	//texture->SamplerDesc12.MaxAnisotropy   = 16;
	texture->SamplerDesc12.MaxAnisotropy  = 1;
	texture->SamplerDesc12.MaxLOD         = D3D12_FLOAT32_MAX;

	return 0;
}

int DXContext::CreateTextureBuffer12(FBOType fboType, DXGI_FORMAT format, Texture* texture)
{
	HRESULT result = this->CreateTexture12(fboType, {}, format, texture);

	if (FAILED(result))
		return -1;

	// FRAME BUFFER
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
	ID3D12DescriptorHeap**        descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC    descriptorHeapDesc = {};
	D3D12_RENDER_TARGET_VIEW_DESC colorBufferDesc    = {};
	D3D12_DEPTH_STENCIL_VIEW_DESC depthBufferDesc    = {};

	descriptorHeapDesc.NumDescriptors = 1;

	switch (fboType) {
	case FBO_COLOR:
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeap          = &texture->ColorBuffer12;
		break;
	case FBO_DEPTH:
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeap          = &texture->DepthBuffer12;
		break;
	default:
		throw;
	}

	result = this->renderDevice12->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap));

	if (FAILED(result))
		return -2;

	descriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE((*descriptorHeap)->GetCPUDescriptorHandleForHeapStart());

	switch (fboType) {
	case FBO_COLOR:
		colorBufferDesc.Format        = format;
		colorBufferDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		this->renderDevice12->CreateRenderTargetView(texture->Resource12, &colorBufferDesc, descriptorHandle);

		break;
	case FBO_DEPTH:
		depthBufferDesc.Format        = DXGI_FORMAT_D32_FLOAT;
		depthBufferDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		this->renderDevice12->CreateDepthStencilView(texture->Resource12, &depthBufferDesc, descriptorHandle);

		break;
	default:
		throw;
	}

	return 0;
}

int DXContext::CreateVertexBuffer11(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, Buffer* buffer)
{
	D3D11_SUBRESOURCE_DATA bufferData       = {};
	D3D11_BUFFER_DESC      bufferDesc       = {};
	std::vector<float>     vertexBufferData = Utils::ToVertexBufferData(vertices, normals, texCoords);

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = (vertexBufferData.size() * sizeof(float));
	bufferData.pSysMem   = &vertexBufferData[0];

	if (FAILED(this->renderDevice11->CreateBuffer(&bufferDesc, &bufferData, &buffer->VertexBufferDX11)))
		return -1;

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[NR_OF_ATTRIBS] = {};

	// NORMALS
	inputElementDesc[ATTRIB_NORMAL].SemanticName      = "NORMAL";
	inputElementDesc[ATTRIB_NORMAL].Format            = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[ATTRIB_NORMAL].AlignedByteOffset = 0;
	inputElementDesc[ATTRIB_NORMAL].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
	//inputElementDesc[ATTRIB_NORMAL].SemanticIndex   = 0;
	//inputElementDesc[ATTRIB_NORMAL].InputSlot       = 0;
	//inputElementDesc[ATTRIB_NORMAL].InstanceDataStepRate = 0;

	// POSITIONS
	inputElementDesc[ATTRIB_POSITION].SemanticName      = "POSITION";
	inputElementDesc[ATTRIB_POSITION].Format            = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc[ATTRIB_POSITION].AlignedByteOffset = (3 * sizeof(float));
	inputElementDesc[ATTRIB_POSITION].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

	// TEXTURE COORDINATES
	inputElementDesc[ATTRIB_TEXCOORDS].SemanticName      = "TEXCOORD";
	inputElementDesc[ATTRIB_TEXCOORDS].Format            = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc[ATTRIB_TEXCOORDS].AlignedByteOffset = (6 * sizeof(float));
	inputElementDesc[ATTRIB_TEXCOORDS].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

	buffer->BufferStride = (8 * sizeof(float));

	D3D11_BLEND_DESC         blendDesc        = {};
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	D3D11_RASTERIZER_DESC    rasterizerDesc   = {};

	for (int i = 0; i < NR_OF_SHADERS; i++)
	{
		if (ShaderManager::Programs[i] == nullptr)
			return -2;

		ID3DBlob* vs = ShaderManager::Programs[i]->VS();

		if (vs == nullptr)
			return -3;

		HRESULT result = this->renderDevice11->CreateInputLayout(
			inputElementDesc, NR_OF_ATTRIBS, vs->GetBufferPointer(), vs->GetBufferSize(), &buffer->InputLayoutsDX11[i]
		);

		if (FAILED(result))
			return -4;

		ShaderID shaderID = (ShaderID)i;

		switch(shaderID) {
		case SHADER_ID_HUD:
			blendDesc        = this->initColorBlending11(TRUE);
			depthStencilDesc = this->initDepthStencilBuffer11(FALSE);
			rasterizerDesc   = this->initRasterizer11(D3D11_CULL_NONE);
			break;
		case SHADER_ID_SKYBOX:
			blendDesc        = this->initColorBlending11(FALSE);
			depthStencilDesc = this->initDepthStencilBuffer11(TRUE, D3D11_COMPARISON_LESS_EQUAL);
			rasterizerDesc   = this->initRasterizer11(D3D11_CULL_NONE);
			break;
		default:
			blendDesc        = this->initColorBlending11(FALSE);
			depthStencilDesc = this->initDepthStencilBuffer11(TRUE);
			rasterizerDesc   = this->initRasterizer11();
			break;
		}

		switch (shaderID) {
		case SHADER_ID_WIREFRAME:
			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			break;
		case SHADER_ID_DEPTH:
			rasterizerDesc.DepthClipEnable = FALSE;
			rasterizerDesc.CullMode        = D3D11_CULL_FRONT;
			break;
		default:
			break;
		}

		if (FAILED(this->renderDevice11->CreateRasterizerState(&rasterizerDesc, &buffer->RasterizerStatesDX11[i])))
			return -5;

		if (FAILED(this->renderDevice11->CreateBlendState(&blendDesc, &buffer->BlendStatesDX11[i])))
			return -6;

		if (FAILED(this->renderDevice11->CreateDepthStencilState(&depthStencilDesc, &buffer->DepthStencilStatesDX11[i])))
			return -7;
	}

	return 0;
}

int DXContext::CreateVertexBuffer12(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, Buffer* buffer)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> attribsDescs = {};

	UINT offset = 0;

	// NORMALS
	if (!normals.empty())
	{
		D3D12_INPUT_ELEMENT_DESC attribsDesc = {};

		attribsDesc.SemanticName      = "NORMAL";
		attribsDesc.InputSlotClass    = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		attribsDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
		attribsDesc.AlignedByteOffset = offset;
		//attribsDesc.SemanticIndex   = 0;
		//attribsDesc.InputSlot       = 0;
		//attribsDesc.InstanceDataStepRate = 0;

		attribsDescs.push_back(attribsDesc);

		offset += (3 * sizeof(float));
	}

	// POSITIONS
	if (!vertices.empty())
	{
		D3D12_INPUT_ELEMENT_DESC attribsDesc = {};

		attribsDesc.SemanticName      = "POSITION";
		attribsDesc.InputSlotClass    = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		attribsDesc.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
		attribsDesc.AlignedByteOffset = offset;

		attribsDescs.push_back(attribsDesc);

		offset += (3 * sizeof(float));
	}

	// TEXTURE COORDINATES
	if (!texCoords.empty())
	{
		D3D12_INPUT_ELEMENT_DESC attribsDesc = {};

		attribsDesc.SemanticName      = "TEXCOORD";
		attribsDesc.InputSlotClass    = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		attribsDesc.Format            = DXGI_FORMAT_R32G32_FLOAT;
		attribsDesc.AlignedByteOffset = offset;

		attribsDescs.push_back(attribsDesc);

		offset += (2 * sizeof(float));
	}

	buffer->BufferStride = offset;

	std::vector<float> vertexBufferData = Utils::ToVertexBufferData(vertices, normals, texCoords);
	UINT               bufferSize       = (vertexBufferData.size() * sizeof(float));

	HRESULT result = this->renderDevice12->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&buffer->VertexBufferDX12)
	);

	if (FAILED(result))
		return -1;

	ID3D12Resource* bufferResource = nullptr;

	result = this->renderDevice12->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&bufferResource)
	);
	
	if (FAILED(result))
		return -2;

	D3D12_SUBRESOURCE_DATA bufferData = {};

	bufferData.pData      = &vertexBufferData[0];
	bufferData.RowPitch   = bufferSize;
	bufferData.SlicePitch = bufferSize;

	buffer->VertexBufferViewDX12.BufferLocation = (buffer->VertexBufferDX12)->GetGPUVirtualAddress();
	buffer->VertexBufferViewDX12.StrideInBytes  = buffer->BufferStride;
	buffer->VertexBufferViewDX12.SizeInBytes    = bufferSize;

	this->commandsInit();

	UpdateSubresources(this->commandList, buffer->VertexBufferDX12, bufferResource, 0, 0, 1, &bufferData);

	this->commandList->ResourceBarrier(
		1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer->VertexBufferDX12, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
	);

	this->commandsExecute();
	this->wait();

	_RELEASEP(bufferResource);

	// RENDER PIPELINES
	for (int i = 0; i < NR_OF_SHADERS; i++)
	{
		ShaderProgram* program = ShaderManager::Programs[i];

		if (this->createPipeline(program, &buffer->PipelineStatesDX12[i], &buffer->RootSignaturesDX12[i], FBO_UNKNOWN, attribsDescs) < 0)
			return -3;

		if (this->createPipeline(program, &buffer->PipelineStatesColorDX12[i], &buffer->RootSignaturesColorDX12[i], FBO_COLOR, attribsDescs) < 0)
			return -4;

		if (this->createPipeline(program, &buffer->PipelineStatesDepthDX12[i], &buffer->RootSignaturesDepthDX12[i], FBO_DEPTH, attribsDescs) < 0)
			return -5;
	}

	return 0;
}

int DXContext::Draw11(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties)
{
	if ((RenderEngine::CameraMain == nullptr) || (mesh == nullptr) || (shaderProgram == nullptr))
		return -1;

	Buffer*             indexBuffer    = dynamic_cast<Mesh*>(mesh)->IndexBuffer();
	Buffer*             vertexBuffer   = dynamic_cast<Mesh*>(mesh)->VertexBuffer();
	ID3D11PixelShader*  fragmentShader = shaderProgram->FragmentShader();
	ID3D11VertexShader* vertexShader   = shaderProgram->VertexShader();
	ShaderID            shaderID       = shaderProgram->ID();

	if ((vertexBuffer == nullptr) || (fragmentShader == nullptr) || (vertexShader == nullptr))
		return -2;

	// CONSTANT BUFFER
	ID3D11Buffer* constantBuffer       = nullptr;
	const void*   constantBufferValues = nullptr;

	int result = shaderProgram->UpdateUniformsDX11(&constantBuffer, &constantBufferValues, mesh, properties);
	
	if ((result < 0) || (constantBuffer == nullptr) || (constantBufferValues == nullptr))
		return -3;

	this->deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, constantBufferValues, 0, 0);

	// INDEX BUFFER
	if (indexBuffer != nullptr)
		this->deviceContext->IASetIndexBuffer(indexBuffer->VertexBufferDX11, DXGI_FORMAT_R32_UINT, 0);

	// VERTEX BUFFER
	ID3D11Buffer* vertexBufferData   = vertexBuffer->VertexBufferDX11;
	UINT          vertexBufferStride = vertexBuffer->BufferStride;
	UINT          vertexBufferOffset = 0;

	this->deviceContext->IASetInputLayout(vertexBuffer->InputLayoutsDX11[shaderID]);
	this->deviceContext->IASetVertexBuffers(0, 1, &vertexBufferData, &vertexBufferStride, &vertexBufferOffset);
	
	// DRAW MODE
	this->deviceContext->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)RenderEngine::GetDrawMode());

	// VERTEX SHADER
	this->deviceContext->VSSetShader(vertexShader, nullptr, 0);
	this->deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	// PIXEL (FRAGMENT) SHADER
	this->deviceContext->PSSetShader(fragmentShader, nullptr, 0);
	this->deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);

	// TEXTURES
	ID3D11ShaderResourceView* meshTextureSRVs[MAX_TEXTURES]     = {};
	ID3D11SamplerState*       meshTextureSamplers[MAX_TEXTURES] = {};

	// MESH TEXTURES
	switch (shaderID) {
	case SHADER_ID_DEFAULT:
	case SHADER_ID_HUD:
	case SHADER_ID_SKYBOX:
	case SHADER_ID_TERRAIN:
	case SHADER_ID_WATER:
		for (int i = 0; i < MAX_TEXTURES; i++) {
			meshTextureSRVs[i]     = mesh->Textures[i]->SRV11;
			meshTextureSamplers[i] = mesh->Textures[i]->SamplerState11;
		}

		this->deviceContext->PSSetShaderResources(0, MAX_TEXTURES, meshTextureSRVs);
		this->deviceContext->PSSetSamplers(0,        MAX_TEXTURES, meshTextureSamplers);

		break;
	default:
		break;
	}

	// BIND DEPTH MAP TEXTURES
	if (shaderID == SHADER_ID_DEFAULT)
	{
		const int NR_OF_SAMPLERS = 2;

		ID3D11SamplerState* depthTextureSamplers[NR_OF_SAMPLERS] = {
			SceneManager::EmptyTexture->SamplerState11,
			SceneManager::EmptyCubemap->SamplerState11
		};

		// n0: 2D textures (dir lights), n1: Cube Map textures (point lights)
		for (int n = 0; n < NR_OF_SAMPLERS; n++)
		{
			ID3D11ShaderResourceView* depthTextureSRVs[MAX_LIGHT_SOURCES] = {};

			// LIGHT SOURCES
			for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			{
				Texture*     texture;
				LightSource* lightSource = SceneManager::LightSources[i];

				if ((lightSource != nullptr) && (lightSource->DepthMapFBO() != nullptr)) {
					texture                 = lightSource->DepthMapFBO()->GetTexture();
					depthTextureSamplers[n] = texture->SamplerState11;
				} else {
					texture = (n == 0 ? SceneManager::EmptyTexture : SceneManager::EmptyCubemap);
				}
				
				if ((n == 0) && (texture->GetTextureType() != TEXTURE_2D)) {
					texture                 = SceneManager::EmptyTexture;
					depthTextureSamplers[n] = texture->SamplerState11;
				}

				if ((n == 1) && (texture->GetTextureType() != TEXTURE_CUBEMAP)) {
					texture                 = SceneManager::EmptyCubemap;
					depthTextureSamplers[n] = texture->SamplerState11;
				}

				depthTextureSRVs[i] = texture->SRV11;
			}

			const UINT START_SLOT_SRV     = (MAX_TEXTURES + (n * MAX_LIGHT_SOURCES));
			const UINT START_SLOT_SAMPLER = (MAX_TEXTURES + n);

			this->deviceContext->PSSetShaderResources(START_SLOT_SRV, MAX_LIGHT_SOURCES, depthTextureSRVs);
			this->deviceContext->PSSetSamplers(START_SLOT_SAMPLER, 1, &depthTextureSamplers[n]);
		}
	}

	// DRAW
	this->deviceContext->RSSetState(vertexBuffer->RasterizerStatesDX11[shaderID]);

	float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	UINT  sampleMask    = 0xffffffff;
	
	this->deviceContext->OMSetBlendState(vertexBuffer->BlendStatesDX11[shaderID], blendFactor, sampleMask);
	this->deviceContext->OMSetDepthStencilState(vertexBuffer->DepthStencilStatesDX11[shaderID], 1);

	if (indexBuffer != nullptr)
		this->deviceContext->DrawIndexed(dynamic_cast<Mesh*>(mesh)->NrOfIndices(), 0, 0);
	else
		this->deviceContext->Draw(dynamic_cast<Mesh*>(mesh)->NrOfVertices(), 0);

	// RESET
	ID3D11ShaderResourceView* nullSRV[MAX_LIGHT_SOURCES]     = {};
	ID3D11SamplerState*       nullSampler[MAX_LIGHT_SOURCES] = {};

	this->deviceContext->PSSetShaderResources(0, MAX_TEXTURES, nullSRV);
	this->deviceContext->PSSetSamplers(0,        MAX_TEXTURES, nullSampler);

	for (int n = 0; n < 2; n++)
		this->deviceContext->PSSetShaderResources(MAX_TEXTURES + (n * MAX_LIGHT_SOURCES), MAX_LIGHT_SOURCES, nullSRV);

	this->deviceContext->PSSetSamplers(MAX_TEXTURES, 2, nullSampler);

	return 0;
}

int DXContext::Draw12(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties)
{
	if ((RenderEngine::CameraMain == nullptr) || (mesh == nullptr) || (shaderProgram == nullptr))
		return -1;

	Buffer*   indexBuffer    = dynamic_cast<Mesh*>(mesh)->IndexBuffer();
	Buffer*   vertexBuffer   = dynamic_cast<Mesh*>(mesh)->VertexBuffer();
	ID3DBlob* fragmentShader = shaderProgram->FS();
	ID3DBlob* vertexShader   = shaderProgram->VS();
	ShaderID  shaderID       = shaderProgram->ID();

	if ((vertexBuffer == nullptr) || (fragmentShader == nullptr) || (vertexShader == nullptr))
		return -2;

	// SHADER UNIFORM BUFFERS
	if (shaderProgram->UpdateUniformsDX12(mesh, properties) < 0)
		return -3;

	// INDEX AND VERTEX BUFFERS
	D3D12_INDEX_BUFFER_VIEW  indexBufferView  = {};
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = vertexBuffer->VertexBufferViewDX12;

	if (indexBuffer != nullptr)
		indexBufferView = indexBuffer->IndexBufferViewDX12;
	
	ID3D12DescriptorHeap* descHeaps[2] = {
		vertexBuffer->ConstantBufferHeapsDX12[shaderProgram->ID()],
		vertexBuffer->SamplerHeapsDX12[shaderProgram->ID()]
	};

	// CONSTANT BUFFER AND TEXTURES
	UINT cbvSrvDescSize = this->renderDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandleCPU(descHeaps[0]->GetCPUDescriptorHandleForHeapStart(), 1, cbvSrvDescSize);
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandleGPU(descHeaps[0]->GetGPUDescriptorHandleForHeapStart(), 1, cbvSrvDescSize);

	UINT samplerDescSize = this->renderDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	CD3DX12_CPU_DESCRIPTOR_HANDLE samplerHandleCPU(descHeaps[1]->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_GPU_DESCRIPTOR_HANDLE samplerHandleGPU(descHeaps[1]->GetGPUDescriptorHandleForHeapStart());

	bool setRootTextures = false;

	// MESH TEXTURES
	switch (shaderID) {
	case SHADER_ID_DEFAULT:
	case SHADER_ID_HUD:
	case SHADER_ID_SKYBOX:
	case SHADER_ID_TERRAIN:
	case SHADER_ID_WATER:
		for (int i = 0; i < MAX_TEXTURES; i++)
		{
			this->renderDevice12->CreateShaderResourceView(
				mesh->Textures[i]->Resource12, &mesh->Textures[i]->SRVDesc12, srvHandleCPU
			);
			srvHandleCPU.Offset(1, cbvSrvDescSize);

			this->renderDevice12->CreateSampler(&mesh->Textures[i]->SamplerDesc12, samplerHandleCPU);
			samplerHandleCPU.Offset(1, samplerDescSize);
		}

		setRootTextures = true;

		break;
	default:
		break;
	}

	// DEPTH MAP TEXTURES
	if (shaderID == SHADER_ID_DEFAULT)
	{
		const int NR_OF_SAMPLERS = 2;

		// n0: 2D textures (dir lights), n1: Cube Map textures (point lights)
		for (int n = 0; n < NR_OF_SAMPLERS; n++)
		{
			D3D12_SAMPLER_DESC samplerDesc;

			// LIGHT SOURCES
			for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			{
				Texture*     texture;
				LightSource* lightSource = SceneManager::LightSources[i];

				if ((lightSource != nullptr) && (lightSource->DepthMapFBO() != nullptr)) {
					texture     = lightSource->DepthMapFBO()->GetTexture();
					samplerDesc = texture->SamplerDesc12;
				} else {
					texture = (n == 0 ? SceneManager::EmptyTexture : SceneManager::EmptyCubemap);
				}
				
				if ((n == 0) && (texture->GetTextureType() != TEXTURE_2D)) {
					texture     = SceneManager::EmptyTexture;
					samplerDesc = texture->SamplerDesc12;
				}

				if ((n == 1) && (texture->GetTextureType() != TEXTURE_CUBEMAP)) {
					texture     = SceneManager::EmptyCubemap;
					samplerDesc = texture->SamplerDesc12;
				}

				this->renderDevice12->CreateShaderResourceView(
					texture->Resource12, &texture->SRVDesc12, srvHandleCPU
				);

				srvHandleCPU.Offset(1, cbvSrvDescSize);
			}

			this->renderDevice12->CreateSampler(&samplerDesc, samplerHandleCPU);
			samplerHandleCPU.Offset(1, samplerDescSize);
		}

		setRootTextures = true;
	}

	// SHADER ROOT SIGNATURES
	switch (properties.FboType) {
		case FBO_COLOR: this->commandList->SetGraphicsRootSignature(vertexBuffer->RootSignaturesColorDX12[shaderID]); break;
		case FBO_DEPTH: this->commandList->SetGraphicsRootSignature(vertexBuffer->RootSignaturesDepthDX12[shaderID]); break;
		default:        this->commandList->SetGraphicsRootSignature(vertexBuffer->RootSignaturesDX12[shaderID]);      break;
	}

	this->commandList->SetDescriptorHeaps(2, descHeaps);
	this->commandList->SetGraphicsRootDescriptorTable(ROOT_CBV, descHeaps[0]->GetGPUDescriptorHandleForHeapStart());

	if (setRootTextures) {
		this->commandList->SetGraphicsRootDescriptorTable(ROOT_TEXTURE_SRV,     srvHandleGPU);
		this->commandList->SetGraphicsRootDescriptorTable(ROOT_TEXTURE_SAMPLER, samplerHandleGPU);
	}

	// DRAW
	if (indexBuffer != nullptr)
		this->commandList->IASetIndexBuffer(&indexBufferView);

	this->commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	this->commandList->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)RenderEngine::GetDrawMode());

	switch (properties.FboType) {
		case FBO_COLOR: this->commandList->SetPipelineState(vertexBuffer->PipelineStatesColorDX12[shaderID]); break;
		case FBO_DEPTH: this->commandList->SetPipelineState(vertexBuffer->PipelineStatesDepthDX12[shaderID]); break;
		default:        this->commandList->SetPipelineState(vertexBuffer->PipelineStatesDX12[shaderID]);      break;
	}

	if (indexBuffer != nullptr)
		this->commandList->DrawIndexedInstanced((UINT)dynamic_cast<Mesh*>(mesh)->NrOfIndices(), 1, 0, 0, 0);
	else
		this->commandList->DrawInstanced((UINT)dynamic_cast<Mesh*>(mesh)->NrOfVertices(), 1, 0, 0);

	return 0;
}

IDXGIAdapter* DXContext::getAdapter11(IDXGIFactory* factory)
{
	if (factory == nullptr)
		return nullptr;

	IDXGIAdapter*     adapter         = nullptr;
	D3D_FEATURE_LEVEL featureLevel    = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	UINT              i               = 0;

	while (factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		HRESULT result = D3D11CreateDevice(
			adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, featureLevels, 1, D3D11_SDK_VERSION, nullptr, &featureLevel, nullptr
		);

		if (SUCCEEDED(result))
			return adapter;

		_RELEASEP(adapter);
		i++;
	}

	return nullptr;
}

IDXGIAdapter1* DXContext::getAdapter12(IDXGIFactory5* factory)
{
	if (factory == nullptr) { return nullptr; }

	IDXGIAdapter1* adapter = nullptr;
	UINT           i       = 0;

	while (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);

		if (SUCCEEDED(result))
			return adapter;

		_RELEASEP(adapter);
		i++;
	}

	return nullptr;
}

D3D12_CLEAR_VALUE DXContext::getClearValueDX12(FBOType fboType, DXGI_FORMAT format)
{
	D3D12_CLEAR_VALUE clearValue = {};

	for (int i = 0; i < 4; i++)
	{
		switch (fboType) {
			case FBO_COLOR: clearValue.Color[i] = CLEAR_VALUE_COLOR[i];   break;
			case FBO_DEPTH: clearValue.Color[i] = CLEAR_VALUE_DEPTH[i];   break;
			default:        clearValue.Color[i] = CLEAR_VALUE_DEFAULT[i]; break;
		}
	}

	clearValue.Format               = format;
	clearValue.DepthStencil.Depth   = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	return clearValue;
}

bool DXContext::init11(bool vsync)
{
	this->multiSampleCount = 16;
	this->vSync            = vsync;

	// SWAP CHAIN
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferCount                        = NR_OF_FRAMEBUFFERS;
	swapChainDesc.BufferDesc.Width                   = RenderEngine::Canvas.Size.GetWidth();
	swapChainDesc.BufferDesc.Height                  = RenderEngine::Canvas.Size.GetHeight();
	swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator   = (this->vSync ? 60 : 0);
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.OutputWindow                       = (HWND)RenderEngine::Canvas.Canvas->GetHWND();
	swapChainDesc.SampleDesc.Count                   = this->multiSampleCount;
	swapChainDesc.Windowed                           = TRUE;

	IDXGIFactory* factory = nullptr;
	HRESULT       result  = CreateDXGIFactory(IID_PPV_ARGS(&factory));

	if (FAILED(result))
		return false;

	// ADAPTER
	IDXGIAdapter* adapter = this->getAdapter11(factory);
	_RELEASEP(factory);

	if (adapter == nullptr)
		return false;

	UINT flags = 0;

	#if defined(_DEBUG)
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	// DEVICE
	D3D_FEATURE_LEVEL featureLevel    = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	do {
		result = D3D11CreateDeviceAndSwapChain(
			adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION,
			&swapChainDesc, &this->swapChain11, &this->renderDevice11, &featureLevel, &this->deviceContext
		);

		if (SUCCEEDED(result))
		{
			//return false;
			UINT sampleQuality = -1;

			result = this->renderDevice11->CheckMultisampleQualityLevels(
				swapChainDesc.BufferDesc.Format, swapChainDesc.SampleDesc.Count, &sampleQuality
			);
		}

		if (SUCCEEDED(result) || (this->multiSampleCount == 1))
			break;
			//return false;

		this->multiSampleCount        /= 2;
		swapChainDesc.SampleDesc.Count = this->multiSampleCount;
	} while (FAILED(result));

	if (FAILED(result))
		return false;

	// COLOR BUFFER (RENDER TARGET VIEW)
	ID3D11Resource* colorBuffer = nullptr;

	result = this->swapChain11->GetBuffer(0, IID_PPV_ARGS(&colorBuffer));
	
	if (FAILED(result))
		return false;

	result = this->renderDevice11->CreateRenderTargetView(colorBuffer, nullptr, &this->colorBuffer);
	_RELEASEP(colorBuffer);
	
	if (FAILED(result))
		return false;

	// DEPTH STENCIL BUFFER
	D3D11_TEXTURE2D_DESC depthStencilResourceDesc = {};

	depthStencilResourceDesc.ArraySize          = 1;
	depthStencilResourceDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
	depthStencilResourceDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilResourceDesc.MipLevels          = 1;
	depthStencilResourceDesc.Width              = swapChainDesc.BufferDesc.Width;
	depthStencilResourceDesc.Height             = swapChainDesc.BufferDesc.Height;
	depthStencilResourceDesc.SampleDesc.Count   = swapChainDesc.SampleDesc.Count;

	ID3D11Texture2D* depthStencilBuffer = nullptr;

	result = this->renderDevice11->CreateTexture2D(&depthStencilResourceDesc, nullptr, &depthStencilBuffer);
	
	if (FAILED(result))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};

	depthStencilViewDesc.Format             = depthStencilResourceDesc.Format;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.ViewDimension      = (depthStencilResourceDesc.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D);

	result = this->renderDevice11->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &this->depthStencilBuffer11);
	_RELEASEP(depthStencilBuffer);

	if (FAILED(result))
		return false;

	// VIEWPORT
	this->viewPort11.TopLeftX = 0.0f;
	this->viewPort11.TopLeftY = 0.0f;
	this->viewPort11.Width    = (FLOAT)swapChainDesc.BufferDesc.Width;
	this->viewPort11.Height   = (FLOAT)swapChainDesc.BufferDesc.Height;
	this->viewPort11.MinDepth = 0.0f;
	this->viewPort11.MaxDepth = 1.0f;

	DXGI_ADAPTER_DESC adapterDesc = {};

	result = adapter->GetDesc(&adapterDesc);

	if (FAILED(result))
		return false;

	RenderEngine::GPU.Vendor   = "";
	RenderEngine::GPU.Renderer = adapterDesc.Description;
	RenderEngine::GPU.Version  = "DirectX 11";

	return SUCCEEDED(result);
}

bool DXContext::init12(bool vsync)
{
	this->multiSampleCount = 16;
	this->vSync            = vsync;

	UINT factoryFlags = 0;

	#if defined _DEBUG
		ID3D12Debug* debug;

		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
			debug->EnableDebugLayer();
			factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	#endif

	// FACTORY
	IDXGIFactory5* factory = nullptr;
	HRESULT        result  = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory));
	
	if (FAILED(result))
		return false;

	// ADAPTER
	IDXGIAdapter1* adapter = this->getAdapter12(factory);
	
	if (adapter == nullptr) {
		_RELEASEP(factory);
		return false;
	}

	// DEVICE
	result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->renderDevice12));
	
	if (FAILED(result)) {
		_RELEASEP(factory);
		return false;
	}

	#if defined _DEBUG
		ID3D12InfoQueue*        infoQueue;
		D3D12_MESSAGE_ID        messageIDs[]        = { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE };
		D3D12_MESSAGE_SEVERITY  messageSeverities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER messageFilter       = {};

		messageFilter.DenyList.NumSeverities = _countof(messageSeverities);
		messageFilter.DenyList.pSeverityList = messageSeverities;
		messageFilter.DenyList.NumIDs        = _countof(messageIDs);
		messageFilter.DenyList.pIDList       = messageIDs;

		result = this->renderDevice12->QueryInterface(IID_ID3D12InfoQueue, (void**)&infoQueue);

		if (SUCCEEDED(result) && (infoQueue != nullptr))
			infoQueue->PushStorageFilter(&messageFilter);
	#endif
	
	// COMMAND QUEUE
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};

	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = this->renderDevice12->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&this->commandQueue));
	
	if (FAILED(result)) {
		_RELEASEP(factory);
		return false;
	}

	// Check if tearing is supported for variable refresh rate
	BOOL tearing      = FALSE;
	result            = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearing, sizeof(tearing));
	UINT tearingFlags = (SUCCEEDED(result) && tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

	// SWAP CHAIN
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferCount                        = NR_OF_FRAMEBUFFERS;
	swapChainDesc.BufferDesc.Width                   = RenderEngine::Canvas.Size.GetWidth();
	swapChainDesc.BufferDesc.Height                  = RenderEngine::Canvas.Size.GetHeight();
	swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator   = (this->vSync ? 60 : 0);
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags                              = (this->vSync ? 0 : tearingFlags);
	swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow                       = (HWND)RenderEngine::Canvas.Canvas->GetHWND();
	swapChainDesc.SampleDesc.Count                   = 1;
	//swapChainDesc.SampleDesc.Count                   = this->multiSampleCount;
	swapChainDesc.Windowed                           = TRUE;

	IDXGISwapChain* swapChain = nullptr;

	result = factory->CreateSwapChain(this->commandQueue, &swapChainDesc, &swapChain);
	_RELEASEP(factory);

	if (FAILED(result))
		return false;

	this->swapChain12      = reinterpret_cast<IDXGISwapChain3*>(swapChain);
	this->colorBufferIndex = this->swapChain12->GetCurrentBackBufferIndex();

	// COLOR BUFFER (RENDER TARGET VIEW) - HEAP DESCRIPTION
	D3D12_DESCRIPTOR_HEAP_DESC colorBufferHeapDesc = {};

	colorBufferHeapDesc.NumDescriptors = NR_OF_FRAMEBUFFERS;
	colorBufferHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	result = this->renderDevice12->CreateDescriptorHeap(&colorBufferHeapDesc, IID_PPV_ARGS(&this->colorBufferHeap));

	if (FAILED(result))
		return false;

    this->colorBufferSize = this->renderDevice12->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// COLOR BUFFER (RENDER TARGET VIEW) - HEAP DESCRIPTION HANDLE
	CD3DX12_CPU_DESCRIPTOR_HANDLE colorBufferHandle(this->colorBufferHeap->GetCPUDescriptorHandleForHeapStart());

	// COLOR BUFFER (RENDER TARGET VIEW) - BUFFERS/VIEWS
	for (UINT i = 0; i < NR_OF_FRAMEBUFFERS; i++)
	{
		result = this->swapChain12->GetBuffer(i, IID_PPV_ARGS(&this->colorBuffers[i]));

		if (FAILED(result))
			return false;

		this->renderDevice12->CreateRenderTargetView(this->colorBuffers[i], nullptr, colorBufferHandle);
		colorBufferHandle.Offset(1, this->colorBufferSize);
	}

	// DEPTH/STENCIL BUFFER - HEAP DESCRIPTION
	D3D12_DESCRIPTOR_HEAP_DESC depthStencilHeapDesc = {};

	depthStencilHeapDesc.NumDescriptors = 1;
	depthStencilHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	result = this->renderDevice12->CreateDescriptorHeap(&depthStencilHeapDesc, IID_PPV_ARGS(&this->depthStencilBufferHeap));

	if (FAILED(result))
		return false;

	// DEPTH/STENCIL BUFFER - HEAP DESCRIPTION HANDLE
	CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilHandle(this->depthStencilBufferHeap->GetCPUDescriptorHandleForHeapStart());

	// DEPTH/STENCIL BUFFER - RESOURCE DESCRIPTION
	D3D12_RESOURCE_DESC depthStencilResourceDesc = {};

	depthStencilResourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilResourceDesc.DepthOrArraySize = 1;
	depthStencilResourceDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthStencilResourceDesc.Format           = DXGI_FORMAT_D32_FLOAT;
	depthStencilResourceDesc.MipLevels        = 1;
	depthStencilResourceDesc.SampleDesc.Count = swapChainDesc.SampleDesc.Count;
	depthStencilResourceDesc.Width            = swapChainDesc.BufferDesc.Width;
	depthStencilResourceDesc.Height           = swapChainDesc.BufferDesc.Height;

	// DEPTH/STENCIL BUFFER - CLEAR VALUE
	D3D12_CLEAR_VALUE depthStencilClearValue = this->getClearValueDX12(FBO_UNKNOWN, depthStencilResourceDesc.Format);

    result = this->renderDevice12->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &depthStencilResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthStencilClearValue, IID_PPV_ARGS(&this->depthStencilBuffer12)
	);

	if (FAILED(result))
		return false;

	// DEPTH/STENCIL BUFFER - VIEW DESCRIPTION
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};

	depthStencilViewDesc.Format        = depthStencilResourceDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	this->renderDevice12->CreateDepthStencilView(this->depthStencilBuffer12, &depthStencilViewDesc, depthStencilHandle);

	// VIEWPORT
	this->viewPort12.TopLeftX = 0.0f;
	this->viewPort12.TopLeftY = 0.0f;
	this->viewPort12.Width    = (FLOAT)swapChainDesc.BufferDesc.Width;
	this->viewPort12.Height   = (FLOAT)swapChainDesc.BufferDesc.Height;
	this->viewPort12.MinDepth = 0.0f;
	this->viewPort12.MaxDepth = 1.0f;

	this->scissorRect = { 0, 0, (LONG)this->viewPort12.Width, (LONG)this->viewPort12.Height };

	// COMMAND ALLOCATOR
	result = this->renderDevice12->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->commandAllocator));
	
	if (FAILED(result))
		return false;

	// COMMAND LIST
	result = this->renderDevice12->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->commandAllocator, this->pipelineState, IID_PPV_ARGS(&this->commandList));
	
	if (FAILED(result))
		return false;

	result = this->commandList->Close();
	
	if (FAILED(result))
		return false;

	// FENCE (SYNCHRONIZATION)
	result = this->renderDevice12->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence));
	
	if (FAILED(result))
		return false;

	this->fenceValue = 1;
	this->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (fenceEvent == nullptr)
		return false;

	DXGI_ADAPTER_DESC adapterDesc = {};

	result = adapter->GetDesc(&adapterDesc);
	
	if (FAILED(result))
		return false;

	RenderEngine::GPU.Vendor   = "";
	RenderEngine::GPU.Renderer = adapterDesc.Description;
	RenderEngine::GPU.Version  = "DirectX 12";

	return SUCCEEDED(result);
}

D3D11_BLEND_DESC DXContext::initColorBlending11(BOOL enableBlend)
{
	D3D11_BLEND_DESC colorBlendInfo = {};

	colorBlendInfo.AlphaToCoverageEnable  = FALSE;
	colorBlendInfo.IndependentBlendEnable = FALSE;

	colorBlendInfo.RenderTarget[0].BlendEnable = enableBlend;

	colorBlendInfo.RenderTarget[0].BlendOp   = D3D11_BLEND_OP_ADD;
	colorBlendInfo.RenderTarget[0].SrcBlend  = D3D11_BLEND_SRC_ALPHA;
	colorBlendInfo.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	colorBlendInfo.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	colorBlendInfo.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	colorBlendInfo.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	colorBlendInfo.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	return colorBlendInfo;
}

D3D12_BLEND_DESC DXContext::initColorBlending12(BOOL enableBlend)
{
	D3D12_BLEND_DESC colorBlendInfo = {};

	colorBlendInfo.AlphaToCoverageEnable  = FALSE;
	colorBlendInfo.IndependentBlendEnable = FALSE;

	colorBlendInfo.RenderTarget[0].BlendEnable = enableBlend;

	colorBlendInfo.RenderTarget[0].BlendOp   = D3D12_BLEND_OP_ADD;
	colorBlendInfo.RenderTarget[0].SrcBlend  = D3D12_BLEND_SRC_ALPHA;
	colorBlendInfo.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

	colorBlendInfo.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;
	colorBlendInfo.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_ONE;
	colorBlendInfo.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;

	colorBlendInfo.RenderTarget[0].LogicOpEnable = FALSE;
	colorBlendInfo.RenderTarget[0].LogicOp       = D3D12_LOGIC_OP_NOOP;

	colorBlendInfo.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return colorBlendInfo;
}

D3D11_DEPTH_STENCIL_DESC DXContext::initDepthStencilBuffer11(BOOL enableDepth, D3D11_COMPARISON_FUNC compareOperation)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilInfo = {};

	depthStencilInfo.DepthEnable    = enableDepth;
	depthStencilInfo.DepthFunc      = compareOperation;
	depthStencilInfo.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	depthStencilInfo.StencilEnable    = FALSE;
	depthStencilInfo.StencilReadMask  = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilInfo.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	depthStencilInfo.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
	depthStencilInfo.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	depthStencilInfo.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
	depthStencilInfo.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;

	depthStencilInfo.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	depthStencilInfo.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilInfo.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;

	return depthStencilInfo;
}

D3D12_DEPTH_STENCIL_DESC DXContext::initDepthStencilBuffer12(BOOL enableDepth, D3D12_COMPARISON_FUNC compareOperation)
{
	D3D12_DEPTH_STENCIL_DESC depthStencilInfo = {};

	depthStencilInfo.DepthEnable    = enableDepth;
	depthStencilInfo.DepthFunc      = compareOperation;
	depthStencilInfo.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	depthStencilInfo.StencilEnable    = FALSE;
	depthStencilInfo.StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilInfo.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	depthStencilInfo.BackFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilInfo.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilInfo.BackFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;
	depthStencilInfo.BackFace.StencilFailOp      = D3D12_STENCIL_OP_KEEP;

	depthStencilInfo.FrontFace.StencilFunc        = D3D12_COMPARISON_FUNC_ALWAYS;
	depthStencilInfo.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilInfo.FrontFace.StencilPassOp      = D3D12_STENCIL_OP_KEEP;

	return depthStencilInfo;
}

D3D11_RASTERIZER_DESC DXContext::initRasterizer11(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode)
{
	D3D11_RASTERIZER_DESC rasterizationInfo = {};

	rasterizationInfo.CullMode = cullMode;
	rasterizationInfo.FillMode = fillMode;

	rasterizationInfo.AntialiasedLineEnable = TRUE;
	rasterizationInfo.FrontCounterClockwise = TRUE;
	rasterizationInfo.MultisampleEnable     = TRUE;
	rasterizationInfo.ScissorEnable         = FALSE;

	rasterizationInfo.DepthClipEnable      = TRUE;
	//rasterizationInfo.DepthBias            = 0;
	//rasterizationInfo.DepthBiasClamp       = 0.0f;
	//rasterizationInfo.SlopeScaledDepthBias = 0.0f;

	return rasterizationInfo;
}

D3D12_RASTERIZER_DESC DXContext::initRasterizer12(D3D12_CULL_MODE cullMode, D3D12_FILL_MODE fillMode)
{
	D3D12_RASTERIZER_DESC rasterizationInfo = {};

	rasterizationInfo.CullMode = cullMode;
	rasterizationInfo.FillMode = fillMode;

	rasterizationInfo.AntialiasedLineEnable = TRUE;
	rasterizationInfo.FrontCounterClockwise = TRUE;
	rasterizationInfo.MultisampleEnable     = TRUE;
	rasterizationInfo.ForcedSampleCount     = 0;
	rasterizationInfo.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	rasterizationInfo.DepthClipEnable      = TRUE;
	//rasterizationInfo.DepthBias            = 0;
	//rasterizationInfo.DepthBiasClamp       = 0.0f;
	//rasterizationInfo.SlopeScaledDepthBias = 0.0f;

	return rasterizationInfo;
}

bool DXContext::IsOK()
{
	return this->isOK;
}

void DXContext::Present11()
{
	this->swapChain11->Present((this->vSync ? 1 : 0), 0);
}

void DXContext::Present12()
{
	this->transitionResource(
		this->colorBuffers[this->colorBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	this->commandsExecute();

	if (this->vSync)
		this->swapChain12->Present(1, 0);
	else
		this->swapChain12->Present(0, DXGI_PRESENT_ALLOW_TEARING);

	this->wait();
}

void DXContext::release()
{
	RenderEngine::Ready = false;

	#if defined(_DEBUG)
		ID3D11Debug*       debugDevice11 = nullptr;
		ID3D12DebugDevice* debugDevice12 = nullptr;

		if (this->renderDevice11 != nullptr)
			this->renderDevice11->QueryInterface(IID_ID3D11Debug, (void**)&debugDevice11);

		if (this->renderDevice12 != nullptr)
			this->renderDevice12->QueryInterface(IID_ID3D12DebugDevice, (void**)&debugDevice12);
	#endif
		
	if (this->fenceEvent != nullptr) {
		this->wait();
		CloseHandle(this->fenceEvent);
	}

	_RELEASEP(this->fence);
	_RELEASEP(this->pipelineState);
	_RELEASEP(this->commandList);
	_RELEASEP(this->commandAllocator);
	_RELEASEP(this->depthStencilBuffer11);
	_RELEASEP(this->depthStencilBuffer12);
	_RELEASEP(this->depthStencilBufferHeap);

	for (UINT i = 0; i < NR_OF_FRAMEBUFFERS; i++)
		_RELEASEP(this->colorBuffers[i]);

	_RELEASEP(this->colorBufferHeap);
	_RELEASEP(this->colorBuffer);
	_RELEASEP(this->swapChain11);
	_RELEASEP(this->swapChain12);
	_RELEASEP(this->commandQueue);
	_RELEASEP(this->renderDevice11);
	_RELEASEP(this->renderDevice12);
	_RELEASEP(this->deviceContext);

	this->colorBufferIndex = 0;
	this->colorBufferSize  = 0;
	this->scissorRect      = {};
	this->viewPort11       = {};
	this->viewPort12       = {};

	#if defined(_DEBUG)
	if (debugDevice11 != nullptr) {
		debugDevice11->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
		_RELEASEP(debugDevice11);
	}

	if (debugDevice12 != nullptr) {
		debugDevice12->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		_RELEASEP(debugDevice12);
	}
	#endif

	RenderEngine::Ready = true;
}

void DXContext::SetVSync(bool enable)
{
	this->vSync = enable;
}

void DXContext::transitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES oldState, D3D12_RESOURCE_STATES newState)
{
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, oldState, newState);
	this->commandList->ResourceBarrier(1, &resourceBarrier);
}

void DXContext::wait()
{
    const UINT64 fence = this->fenceValue;

	this->commandQueue->Signal(this->fence, fence);
    this->fenceValue++;

    if (this->fence->GetCompletedValue() < fence) {
		this->fence->SetEventOnCompletion(fence, this->fenceEvent);
        WaitForSingleObject(this->fenceEvent, INFINITE);
    }

    this->colorBufferIndex = this->swapChain12->GetCurrentBackBufferIndex();
}

#endif
