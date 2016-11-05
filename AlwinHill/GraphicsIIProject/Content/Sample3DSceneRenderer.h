#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <atlbase.h>
#include "Diffuse_Knight_Cleansed.h"
#include "Common\DDSTextureLoader.h"
#include "..\Common\DirectXHelper.h"
#include <random>

using namespace DirectX;

namespace DX11UWA
{
	struct ObjForLoading
	{
		std::vector<XMFLOAT3> vertices;
		std::vector<unsigned short> indices;
		std::vector<XMFLOAT3> normals;
	};

	//stores the info for instamced objects
	struct InstanceType
	{
		XMFLOAT3 position;
	};

	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources(void);
		void CreateWindowSizeDependentResources(void);
		void ReleaseDeviceDependentResources(void);
		void Update(DX::StepTimer const& timer);
		void Render(void);
		void StartTracking(void);
		void TrackingUpdate(float positionX);
		void StopTracking(void);
		inline bool IsTracking(void) { return m_tracking; }

		//Loading Model Function
		ObjForLoading LoadModel(char *_path);

		// Helper functions for keyboard and mouse input
		void SetKeyboardButtons(const char* list);
		void SetMousePosition(const Windows::UI::Input::PointerPoint^ pos);
		void SetInputDeviceData(const char* kb, const Windows::UI::Input::PointerPoint^ pos);

		//CComPtr<ID3D11Texture2D> diffuseTexture;
		CComPtr<ID3D11Texture2D> modelTexture;

	private:
		void Rotate(/*ModelViewProjectionConstantBuffer &modelView,*/ float radians);
		void UpdateCamera(DX::StepTimer const& timer, float const moveSpd, float const rotSpd);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;

		//Skybox variables
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_skyboxVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_skyboxIndexBuffer;
		uint32										m_skyboxIndexCount;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	skyboxVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	skyboxPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	skyboxInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		skyboxConstantBuffer;
		CComPtr<ID3D11Texture2D>					skyboxTexture;
		CComPtr<ID3D11ShaderResourceView>			skyboxModelView;
		CComPtr<ID3D11Buffer>						m_gradientBuffer;
		XMFLOAT4 m_apexColor;
		XMFLOAT4 m_centerColor;


		struct GradientBufferType
		{
			XMFLOAT4 apexColor;
			XMFLOAT4 centerColor;
		};
		struct MatrixBufferType
		{
			XMMATRIX world;
			XMMATRIX view;
			XMMATRIX projection;
		};

		CComPtr<ID3D11ShaderResourceView>			modelView;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer			m_constantBufferData;
		uint32	m_indexCount;

		struct MODELDATA
		{
			Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
			CComPtr<ID3D11ShaderResourceView>			modelView;
			// System resources for cube geometry.
			ModelViewProjectionConstantBuffer			m_constantBufferData;
			uint32										m_indexCount;
			CComPtr<ID3D11Texture2D>					modelTexture;
		};

		std::vector<MODELDATA> modelData;

		//Shaders for models excluding the CUBE
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	modelVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	modelPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	modelInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		modelConstantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_instanceBuffer;
		int											m_instanceCount;


		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;

		// Data members for keyboard and mouse input
		char	m_kbuttons[256];
		Windows::UI::Input::PointerPoint^ m_currMousePos;
		Windows::UI::Input::PointerPoint^ m_prevMousePos;

		// Matrix data member for the camera
		DirectX::XMFLOAT4X4 m_camera;
	};


}

