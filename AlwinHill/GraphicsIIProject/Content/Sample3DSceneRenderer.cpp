#include "pch.h"
#include "Sample3DSceneRenderer.h"

using namespace DX11UWA;
using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	memset(m_kbuttons, 0, sizeof(m_kbuttons));
	m_currMousePos = nullptr;
	m_prevMousePos = nullptr;
	memset(&m_camera, 0, sizeof(XMFLOAT4X4));
	memset(&m_camera2, 0, sizeof(XMFLOAT4X4));


	D3D11_SAMPLER_DESC samplerDesc2;
	samplerDesc2.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT; // bilinear
	samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.MaxAnisotropy = 1.0f;
	samplerDesc2.MipLODBias = 1.0f;
	ID3D11SamplerState *samplerS2;

	m_deviceResources->GetD3DDevice()->CreateSamplerState(&samplerDesc2, &samplerS2);

	m_deviceResources->GetD3DDeviceContext()->PSSetSamplers(1, 1, &samplerS2);
	m_deviceResources->GetD3DDeviceContext()->PSSetSamplers(0, 1, &samplerS2);


	//ModelData for each model loaded... increase everytime new model is added
	int resize = 3;
	modelData.resize(resize);
	//++resize;
	m_instanceBuffer = nullptr;
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();


	//LIGHTING STUFF 
	spot = false; point = false; dir = false;
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	m_deviceResources->GetD3DDevice()->CreateBuffer(&lightBufferDesc, nullptr, &m_lightBuffer);

	//For changing Point light dir
	xDir = 5.0f;

	// Alpha Blending
	D3D11_RENDER_TARGET_BLEND_DESC rtbDesc = { true, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_COLOR_WRITE_ENABLE_ALL };
	D3D11_BLEND_DESC blendDesc = { true, true, rtbDesc };

	m_deviceResources->GetD3DDevice()->CreateBlendState(&blendDesc, &blendState);

	//Setting Up viewports
	viewports[0].Width = m_deviceResources->GetScreenViewport().Width * 0.5;
	viewports[0].Height = m_deviceResources->GetScreenViewport().Height;
	viewports[0].TopLeftX = 0.0f;
	viewports[0].MinDepth = 0.0f;
	viewports[0].MaxDepth = 1.0f;
	viewports[0].TopLeftY = 0.0f;

	viewports[1].Width = m_deviceResources->GetScreenViewport().Width * 0.5;
	viewports[1].Height = m_deviceResources->GetScreenViewport().Height;
	viewports[1].MinDepth = 0.0f;
	viewports[1].MaxDepth = 1.0f;
	viewports[1].TopLeftX = (m_deviceResources->GetScreenViewport().Width * 0.5);
	viewports[1].TopLeftY = 0.0f;

}

//Function to load models from .obj files
ObjForLoading Sample3DSceneRenderer::LoadModel(char *_path)
{
	//Loading .OBJ model
	const char *path;
	std::vector<unsigned short> outIndices;
	std::vector<unsigned short> outUV;
	std::vector<unsigned short> outNormals;

	std::vector<unsigned int> vertIndices, uvIndices, normalIndices;
	std::vector<XMFLOAT3> temp_verts;
	std::vector<XMFLOAT3> temp_uv;
	std::vector<XMFLOAT3> temp_normal;
	char readStream[128];
	std::string str;

	//path = "testpyramid.obj";
	//path = "dragonknight_hr-wind.obj";
	//path = "swords.obj";
	//path = "succubus-fire.obj";
	path = _path;

	std::ifstream file;
	file.open(path);
	while (!file.eof())
	{
		file >> readStream;

		if (readStream[0] == 'v' && readStream[1] == '\0' /*&& readStream[1] != 'n'*/)
		{
			XMFLOAT3 vertex;
			file >> vertex.x >> vertex.y >> vertex.z;
			vertex.x /= 100;
			vertex.y /= 100;
			vertex.z /= 100;

			temp_verts.push_back(vertex);
		}

		else if (readStream[0] == 'v' && readStream[1] == 'n')
		{
			XMFLOAT3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normal.push_back(normal);
		}

		else if (readStream[0] == 'v' && readStream[1] == 't')
		{
			XMFLOAT3 uv;
			file >> uv.x >> uv.y;
			uv.y = 1 - uv.y;
			temp_uv.push_back(uv);
		}

		else if (readStream[0] == 'f')
		{
			int ndx = 0;
			std::string entireLine;
			char delimiter = '/';
			char del = '\n';
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			std::getline(file, entireLine, del);
			std::stringstream ss(entireLine);

			while (!ss.eof())
			{
				std::getline(ss, str, delimiter);
				vertexIndex[ndx] = stoi(str);
				vertIndices.push_back(vertexIndex[ndx]);

				std::getline(ss, str, delimiter);
				uvIndex[ndx] = stoi(str);
				uvIndices.push_back(vertexIndex[ndx]);

				std::getline(ss, str, ' ');
				normalIndex[ndx] = stoi(str);
				normalIndices.push_back(vertexIndex[ndx]);

				++ndx;
			}
		}
	}
	file.close();

	//outVertices.resize(vertIndices.size());
	for (unsigned int i = 0; i < vertIndices.size(); i++)
	{
		unsigned short vIndex = vertIndices[i];
		unsigned short vertex = vIndex - 1;
		outIndices.push_back(vertex);

		unsigned short uvIndex = uvIndices[i];
		unsigned short uv = uvIndex - 1;
		outUV.push_back(uv);

		unsigned short normalIndex = normalIndices[i];
		unsigned short normal = normalIndex - 1;
		outNormals.push_back(normal);
	}

	std::vector<XMFLOAT3> objectVertices;
	for (size_t i = 0; i < (temp_verts.size()); i++)
	{
		objectVertices.push_back(temp_verts[i]);
		objectVertices.push_back(temp_uv[i]);
		objectVertices.push_back(temp_normal[i]);
	}

	ObjForLoading data;
	data.vertices = objectVertices;
	data.indices = outIndices;

	return data;
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources(void)
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	//Projection for Cube
	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	//For skybox
	XMStoreFloat4x4(&skyboxConstantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));

	//Projection for Model1
	XMStoreFloat4x4(&modelData[0].m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&modelData[1].m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
	XMStoreFloat4x4(&modelData[2].m_constantBufferData.projection, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));


	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, -2.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	//Viewport 2
	static const XMVECTORF32 eye2 = { 0.0f, 4.7f, 7.5f, 0.0f };
	static const XMVECTORF32 at2 = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up2 = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&m_camera2, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye2, at2, up2)));

	//CUBE
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));

	//Skybox
	XMStoreFloat4x4(&skyboxConstantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));


	//Models
	XMStoreFloat4x4(&modelData[0].m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&modelData[1].m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
	XMStoreFloat4x4(&modelData[2].m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));

}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (!m_tracking)
	{
		// Convert degrees to radians, then convert seconds to rotation angle
		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
		float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

		Rotate(radians);

		SkyboxRenderer(XMFLOAT3(m_camera._41, m_camera._42, m_camera._43));

		//Rotate Light
		float radius = 7.0f;
		float offset = 0/*2.0f * XM_PI / 3*/;
		XMFLOAT4 lightPos = XMFLOAT4(std::sin(totalRotation + offset) * radius, 7.0f, std::cos(totalRotation + offset) * radius, 1.0f);
		XMVECTOR lightDir = XMVectorSet(-lightPos.x, -lightPos.y, -lightPos.z, 1.0f);
		lightDir = XMVector3Normalize(lightDir);
		XMStoreFloat3(&DirLightDir, lightDir);

		/*xDir -= 0.1f;
		if (xDir < -5.0f)
		{
			xDir = 5.0f;
		}*/
	}

	// Update or move camera here
	UpdateCamera(timer, 1.0f, 0.75f);
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 3, 0) * XMMatrixRotationY(radians)));

	//Translate MODEL 1 
	XMStoreFloat4x4(&modelData[0].m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 0, -2.5) * XMMatrixRotationY(3.142)));
	XMStoreFloat4x4(&modelData[1].m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(-2.5, 0, 0.5) * XMMatrixRotationY(3.142)));
	XMStoreFloat4x4(&modelData[2].m_constantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(2.5, 0, 0.5) * XMMatrixRotationY(3.142)));

	//Skybox
	//XMStoreFloat4x4(&skyboxConstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(0, 3, 0) * XMMatrixRotationY(radians)));

}

void Sample3DSceneRenderer::SkyboxRenderer(XMFLOAT3 pos)
{
	XMStoreFloat4x4(&skyboxConstantBufferData.model, XMMatrixTranspose(XMMatrixTranslation(pos.x, pos.y, pos.z)) * XMMatrixScaling(100, 100, 100));
}

void Sample3DSceneRenderer::UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd)
{
	const float delta_time = (float)timer.GetElapsedSeconds();

	if (m_kbuttons['W'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['S'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpd * delta_time);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['A'])
	{
		XMMATRIX translation = XMMatrixTranslation(-moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['D'])
	{
		XMMATRIX translation = XMMatrixTranslation(moveSpd * delta_time, 0.0f, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons['X'])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, -moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}
	if (m_kbuttons[VK_SPACE])
	{
		XMMATRIX translation = XMMatrixTranslation(0.0f, moveSpd * delta_time, 0.0f);
		XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
		XMMATRIX result = XMMatrixMultiply(translation, temp_camera);
		XMStoreFloat4x4(&m_camera, result);
	}

	if (m_currMousePos)
	{
		if (m_currMousePos->Properties->IsRightButtonPressed && m_prevMousePos)
		{
			//597 x = half screen
			if (m_currMousePos->Position.X < (m_deviceResources->GetScreenViewport().Width * 0.5))
			{
				float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
				float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;

				XMFLOAT4 pos = XMFLOAT4(m_camera._41, m_camera._42, m_camera._43, m_camera._44);

				m_camera._41 = 0;
				m_camera._42 = 0;
				m_camera._43 = 0;

				XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
				XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

				XMMATRIX temp_camera = XMLoadFloat4x4(&m_camera);
				temp_camera = XMMatrixMultiply(rotX, temp_camera);
				temp_camera = XMMatrixMultiply(temp_camera, rotY);

				XMStoreFloat4x4(&m_camera, temp_camera);

				m_camera._41 = pos.x;
				m_camera._42 = pos.y;
				m_camera._43 = pos.z;
			}

			if (m_currMousePos->Position.X > (m_deviceResources->GetScreenViewport().Width * 0.5))
			{
				float dx = m_currMousePos->Position.X - m_prevMousePos->Position.X;
				float dy = m_currMousePos->Position.Y - m_prevMousePos->Position.Y;
				XMFLOAT4 pos2 = XMFLOAT4(m_camera2._41, m_camera2._42, m_camera2._43, m_camera2._44);

				m_camera2._41 = 0;
				m_camera2._42 = 0;
				m_camera2._43 = 0;

				XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
				XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

				XMMATRIX temp_camera2 = XMLoadFloat4x4(&m_camera2);
				temp_camera2 = XMMatrixMultiply(rotX, temp_camera2);
				temp_camera2 = XMMatrixMultiply(temp_camera2, rotY);

				XMStoreFloat4x4(&m_camera2, temp_camera2);

				m_camera2._41 = pos2.x;
				m_camera2._42 = pos2.y;
				m_camera2._43 = pos2.z;
			}

		}
		m_prevMousePos = m_currMousePos;
	}
}

void Sample3DSceneRenderer::SetKeyboardButtons(const char* list)
{
	memcpy_s(m_kbuttons, sizeof(m_kbuttons), list, sizeof(m_kbuttons));
}

void Sample3DSceneRenderer::SetMousePosition(const Windows::UI::Input::PointerPoint^ pos)
{
	m_currMousePos = const_cast<Windows::UI::Input::PointerPoint^>(pos);
}

void Sample3DSceneRenderer::SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos)
{
	SetKeyboardButtons(kb);
	SetMousePosition(pos);
}

void DX11UWA::Sample3DSceneRenderer::StartTracking(void)
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX)
{
	if (m_tracking)
	{
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
	}
}

void Sample3DSceneRenderer::StopTracking(void)
{
	m_tracking = false;
}

void Sample3DSceneRenderer::Draw(/*int lightType*/)
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	//For alpha blending
	float blendFactor[] = { 1, 1, 1, 1 };
	context->OMSetBlendState(blendState.Get(), blendFactor, 0xffffffff);

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, nullptr, &m_constantBufferData, 0, 0, 0);
	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	// Each index is one 16-bit unsigned integer (short).
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);
	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	//Draw the objects.
	context->DrawIndexed(m_indexCount, 0, 0);

	//Loading textures for models
	for (size_t i = 0; i < modelData.size(); i++)
	{
		ID3D11ShaderResourceView *texViews2[] = { { modelData[i].modelView } };
		context->PSSetShaderResources(1, 1, texViews2);

		context->UpdateSubresource1(modelConstantBuffer.Get(), 0, NULL, &modelData[i].m_constantBufferData, 0, 0, 0);
		UINT stride3 = sizeof(XMFLOAT3) * 3;
		UINT offset3 = 0;
		context->IASetVertexBuffers(0, 1, modelData[i].m_vertexBuffer.GetAddressOf(), &stride3, &offset3);
		// Each index is one 16-bit unsigned integer (short).
		context->IASetIndexBuffer(modelData[i].m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(modelInputLayout.Get());
		// Attach our vertex shader.
		context->VSSetShader(modelVertexShader.Get(), nullptr, 0);
		// Send the constant buffer to the graphics device.
		context->VSSetConstantBuffers1(0, 1, modelConstantBuffer.GetAddressOf(), nullptr, nullptr);
		// Attach our pixel shader.
		context->PSSetShader(modelPixelShader.Get(), nullptr, 0);
		// Draw the objects.
		context->DrawIndexed(modelData[i].m_indexCount, 0, 0);

	}

	//Instancing Stuff
	ID3D11ShaderResourceView *texViews2[] = { { modelData[0].modelView } };
	context->PSSetShaderResources(1, 1, texViews2);

	unsigned int strides[2];
	unsigned int offsets[2];
	Microsoft::WRL::ComPtr<ID3D11Buffer> bufferPointers[2];

	strides[0] = sizeof(XMFLOAT3) * 3;
	strides[1] = sizeof(InstanceType);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = modelData[0].m_vertexBuffer;
	bufferPointers[1] = m_instanceBuffer;


	context->UpdateSubresource1(modelConstantBuffer.Get(), 0, NULL, &modelData[0].m_constantBufferData, 0, 0, 0);

	context->IASetVertexBuffers(0, 2, bufferPointers->GetAddressOf(), strides, offsets);
	context->IASetIndexBuffer(modelData[0].m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(modelInputLayout.Get());
	context->DrawIndexedInstanced(modelData[0].m_indexCount, m_instanceCount, 0, 0, 0);


	//Skybox Render
	ID3D11ShaderResourceView *skyboxTexViews[] = { { skyboxModelView } };
	context->PSSetShaderResources(0, 1, skyboxTexViews);
	skyboxConstantBufferData.view = m_constantBufferData.view;
	skyboxConstantBufferData.projection = m_constantBufferData.projection;

	context->UpdateSubresource1(modelConstantBuffer.Get(), 0, NULL, &skyboxConstantBufferData, 0, 0, 0);
	UINT skyboxStride = sizeof(VertexPositionColor);
	UINT skyboxOffset = 0;

	context->IASetVertexBuffers(0, 1, m_skyboxVertexBuffer.GetAddressOf(), &skyboxStride, &skyboxOffset);
	context->IASetIndexBuffer(m_skyboxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_inputLayout.Get());
	context->VSSetShader(skyboxVertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers1(0, 1, modelConstantBuffer.GetAddressOf(), nullptr, nullptr);
	context->PSSetShader(skyboxPixelShader.Get(), nullptr, 0);
	context->DrawIndexed(m_skyboxIndexCount, 0, 0);


	if (dir)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		LightBufferType* dataPtr2;
		m_deviceResources->GetD3DDeviceContext()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr2 = (LightBufferType*)mappedResource.pData;
		dataPtr2->diffuseColor = XMFLOAT4(DirLightDir.x, DirLightDir.y, DirLightDir.z, 1.0f)/*XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f)*/;
		dataPtr2->lightDirection = DirLightDir;
		dataPtr2->lightType = 1.0f;
		m_deviceResources->GetD3DDeviceContext()->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		m_deviceResources->GetD3DDeviceContext()->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
	}
	if (point)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		LightBufferType* dataPtr2;
		m_deviceResources->GetD3DDeviceContext()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr2 = (LightBufferType*)mappedResource.pData;
		dataPtr2->diffuseColor = XMFLOAT4(0.8f, 1.0f, 0.8f, 1.0f);
		dataPtr2->lightDirection = XMFLOAT3(-15.0f, -15.0f, 2.0f);
		dataPtr2->lightType = 2.0f;
		m_deviceResources->GetD3DDeviceContext()->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		m_deviceResources->GetD3DDeviceContext()->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
	}
	if (spot)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		LightBufferType* dataPtr2;
		m_deviceResources->GetD3DDeviceContext()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr2 = (LightBufferType*)mappedResource.pData;
		dataPtr2->diffuseColor = XMFLOAT4(0.2f, 0.5f, 0.4f, 1.0f);
		dataPtr2->lightDirection = XMFLOAT3(0.0f, 5.0f, 1.0f);
		dataPtr2->lightType = 3.0f;
		m_deviceResources->GetD3DDeviceContext()->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		m_deviceResources->GetD3DDeviceContext()->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
	}

	if (!spot && !dir && !point)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		unsigned int bufferNumber;
		LightBufferType* dataPtr2;
		m_deviceResources->GetD3DDeviceContext()->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		dataPtr2 = (LightBufferType*)mappedResource.pData;
		dataPtr2->diffuseColor = XMFLOAT4(0.4f, 0.4f, 0.2f, 1.0f);
		dataPtr2->lightDirection = XMFLOAT3(0.0f, 5.0f, 1.0f);
		dataPtr2->lightType = 4.0f;
		m_deviceResources->GetD3DDeviceContext()->Unmap(m_lightBuffer, 0);
		bufferNumber = 0;
		m_deviceResources->GetD3DDeviceContext()->PSSetConstantBuffers(bufferNumber, 1, &m_lightBuffer);
	}
}

// Renders one frame using the vertex and pixel shaders.
void Sample3DSceneRenderer::Render(void)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	for (size_t i = 0; i < modelData.size(); i++)
	{
		XMStoreFloat4x4(&modelData[i].m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));
	}

	XMStoreFloat4x4(&skyboxConstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera))));

	//Directional Lighting
	if (m_kbuttons['1'])
	{
		dir = !dir;
		point = false;
		spot = false;
	}
	if (m_kbuttons['2'])
	{
		point = !point;
		dir = false;
		spot = false;
	}
	if (m_kbuttons['3'])
	{
		spot = !spot;
		dir = false;
		point = false;
	}

	m_deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &viewports[0]);
	Draw();

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera2))));

	for (size_t i = 0; i < modelData.size(); i++)
	{
		XMStoreFloat4x4(&modelData[i].m_constantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera2))));
	}


	XMStoreFloat4x4(&skyboxConstantBufferData.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_camera2))));

	m_deviceResources->GetD3DDeviceContext()->RSSetViewports(1, &viewports[1]);
	Draw();
}

void Sample3DSceneRenderer::CreateDeviceDependentResources(void)
{
	// Load shaders asynchronously.
	//For Cube
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	//For models
	auto loadModelVS = DX::ReadDataAsync(L"VS_ModelShader.cso");
	auto loadModelPS = DX::ReadDataAsync(L"PS_ModelShader.cso");

	//For Skybox
	auto loadSkyboxVS = DX::ReadDataAsync(L"VS_SkyboxShader.cso");
	auto loadSkyboxPS = DX::ReadDataAsync(L"PS_SkyboxShader.cso");

	//For Lighting
	auto loadDirShaderVS = DX::ReadDataAsync(L"VS_Directional.cso");
	auto loadDirShaderPS = DX::ReadDataAsync(L"PS_Directional.cso");


	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &m_vertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &m_inputLayout));
	});

	//Pixel Shader
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &m_pixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer));
	});


	//Shaders for models including Normals
	auto createModelVS = loadModelVS.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &modelVertexShader));

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &fileData[0], fileData.size(), &modelInputLayout));
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createModelPS = loadModelPS.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &modelPixelShader));

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, &modelConstantBuffer));
	});


	//Skybox Shaders
	auto createSkyboxVS = loadSkyboxVS.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateVertexShader(&fileData[0], fileData.size(), nullptr, &skyboxVertexShader));

	});
	auto createSkyboxPS = loadSkyboxPS.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &skyboxPixelShader));
	});

	//LIGHT Shaders
	auto createDirLightPS = loadDirShaderPS.then([this](const std::vector<byte>& fileData)
	{
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreatePixelShader(&fileData[0], fileData.size(), nullptr, &modelPixelShader));
	});


	// Once shaders are loaded, create the mesh.
	auto createModelTask = (createModelVS && createModelPS).then([this]()
	{
		srand(time(NULL));

		//DDS Loader
		//CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"Diffuse_Knight_Cleansed.dds", (ID3D11Resource**)&modelTexture, &modelView);
		//CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"BStrongoli_D_Sword.dds", (ID3D11Resource**)&modelTexture, &modelView);
		//CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"dragonknight_hr-wind.dds", (ID3D11Resource**)&modelData[0].modelTexture, &modelData[0].modelView);

		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"succubus-fire.dds", (ID3D11Resource**)&modelData[0].modelTexture, &modelData[0].modelView);

		//Load Model1
		ObjForLoading object = LoadModel("succubus-fire.obj");

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = object.vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(XMFLOAT3) * object.vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &modelData[0].m_vertexBuffer));

		modelData[0].m_indexCount = object.indices.size();
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = object.indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * object.indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &modelData[0].m_indexBuffer));

		//For instancing 
		InstanceType *instances;
		D3D11_BUFFER_DESC instanceBuffDesc;
		D3D11_SUBRESOURCE_DATA instanceData;

		m_instanceCount = 5;
		instances = new InstanceType[m_instanceCount];
		for (size_t i = 0; i < m_instanceCount; i++)
		{
			float x = (rand() % 6) - 3;
			float y = (rand() % 6) - 3;
			float z = (rand() % 6);

			instances[i].position = XMFLOAT3(x, 0, 3.5f);
		}

		instanceBuffDesc.Usage = D3D11_USAGE_DEFAULT;
		instanceBuffDesc.ByteWidth = sizeof(InstanceType) * m_instanceCount;
		instanceBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBuffDesc.CPUAccessFlags = 0;
		instanceBuffDesc.MiscFlags = 0;
		instanceBuffDesc.StructureByteStride = 0;

		instanceData.pSysMem = instances;
		instanceData.SysMemPitch = 0;
		instanceData.SysMemSlicePitch = 0;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&instanceBuffDesc, &instanceData, &m_instanceBuffer));

		delete[] instances;
		instances = nullptr;
	});

	auto createModel2 = (createModelVS && createModelPS).then([this]()
	{
		srand(time(NULL));

		//DDS Loader
		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"dragonknight_hr-wind.dds", (ID3D11Resource**)&modelData[1].modelTexture, &modelData[1].modelView);

		//Load Model1
		ObjForLoading object = LoadModel("dragonknight_hr-wind.obj");

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = object.vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(XMFLOAT3) * object.vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &modelData[1].m_vertexBuffer));

		modelData[1].m_indexCount = object.indices.size();
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = object.indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * object.indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &modelData[1].m_indexBuffer));
	});

	auto createModel3 = (createModelVS && createModelPS).then([this]()
	{
		srand(time(NULL));

		//DDS Loader
		HRESULT res = CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"archangel_dark.dds", (ID3D11Resource**)&modelData[2].modelTexture, &modelData[2].modelView);

		//Load Model1
		ObjForLoading object = LoadModel("archangel_dark.obj");

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = object.vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(XMFLOAT3) * object.vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &modelData[2].m_vertexBuffer));

		modelData[2].m_indexCount = object.indices.size();
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = object.indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned short) * object.indices.size(), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &modelData[2].m_indexBuffer));
	});


	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		//CUBE STUFF
		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer));

		static const unsigned short cubeIndices[] =
		{
			0,1,2, // -x
			1,3,2,

			4,6,5, // +x
			5,6,7,

			0,5,1, // -y
			0,4,5,

			2,7,6, // +y
			2,3,7,

			0,6,4, // -z
			0,2,6,

			1,7,3, // +z
			1,5,7,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer));
	});

	//Creating the skybox
	auto skyBox = (createSkyboxPS && createSkyboxVS).then([this]()
	{
		CreateDDSTextureFromFile(m_deviceResources->GetD3DDevice(), L"mercury.dds", (ID3D11Resource**)&skyboxTexture, &skyboxModelView);
		XMFLOAT3 skyOffset = XMFLOAT3(1, 1, 1);

		static const VertexPositionColor skyboxVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = skyboxVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(skyboxVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_skyboxVertexBuffer));

		static const unsigned short skyboxIndices[] =
		{
			2,1,0, // -x
			2,3,1,

			5,6,4, // +x
			7,6,5,

			1,5,0, // -y
			5,4,0,

			6,7,2, // +y
			7,3,2,

			4,6,0, // -z
			6,2,0,

			3,7,1, // +z
			7,5,1,
		};

		m_skyboxIndexCount = ARRAYSIZE(skyboxIndices);
		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = skyboxIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(skyboxIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_skyboxIndexBuffer));

	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]()
	{

	});

	createModelTask.then([this]()
	{
		m_loadingComplete = true;

	});

	createModel2.then([this]()
	{

	});

	createModel3.then([this]()
	{

	});

	skyBox.then([this]()
	{

	});
}

void Sample3DSceneRenderer::ReleaseDeviceDependentResources(void)
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();

	modelVertexShader.Reset();
	modelPixelShader.Reset();

	m_lightBuffer->Release();
	m_lightBuffer = 0;
}