#pragma once

#include "Utils\Utils.h"

extern bool GetD3DPath;
extern char D3DImPath[MAX_PATH];
extern char D3DIm700Path[MAX_PATH];
extern HMODULE hD3DIm;
extern HMODULE hD3DIm700;

class m_IDirect3DX : public IUnknown
{
private:
	IDirect3D7 *ProxyInterface = nullptr;
	m_IDirect3D7 *WrapperInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;
	m_IDirectDrawX *ddrawParent = nullptr;

	// Store d3d version wrappers
	std::unique_ptr<m_IDirect3D> UniqueProxyInterface = nullptr;
	std::unique_ptr<m_IDirect3D2> UniqueProxyInterface2 = nullptr;
	std::unique_ptr<m_IDirect3D3> UniqueProxyInterface3 = nullptr;
	std::unique_ptr<m_IDirect3D7> UniqueProxyInterface7 = nullptr;

public:
	m_IDirect3DX(IDirect3D7 *aOriginal, DWORD DirectXVersion, m_IDirect3D7 *Interface) : ProxyInterface(aOriginal), WrapperInterface(Interface)
	{
		ProxyDirectXVersion = GetIIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			Logging::LogDebug() << "Convert Direct3D v" << DirectXVersion << " to v" << ProxyDirectXVersion;
		}
		else
		{
			Logging::LogDebug() << "Create " << __FUNCTION__ << " v" << DirectXVersion;
		}

		if (Config.DDrawResolutionHack)
		{
			if (GetD3DPath)
			{
				GetD3DPath = false;
				GetSystemDirectory(D3DImPath, MAX_PATH);
				strcpy_s(D3DIm700Path, MAX_PATH, D3DImPath);
				strcat_s(D3DImPath, MAX_PATH, "\\d3dim.dll");
				strcat_s(D3DIm700Path, MAX_PATH, "\\d3dim700.dll");
			}

			if (!hD3DIm)
			{
				hD3DIm = GetModuleHandle(D3DImPath);
				if (hD3DIm)
				{
					Logging::LogDebug() << __FUNCTION__ << " Found loaded dll: 'd3dim.dll'";
					Utils::DDrawResolutionHack(hD3DIm);
				}
			}

			if (!hD3DIm700)
			{
				hD3DIm700 = GetModuleHandle(D3DIm700Path);
				if (hD3DIm700)
				{
					Logging::LogDebug() << __FUNCTION__ << " Found loaded dll: 'd3dim700.dll'";
					Utils::DDrawResolutionHack(hD3DIm700);
				}
			}
		}
	}
	m_IDirect3DX(m_IDirectDrawX *aOriginal, DWORD DirectXVersion) : ddrawParent(aOriginal)
	{
		ProxyDirectXVersion = 9;

		Logging::LogDebug() << "Convert Direct3DDevice v" << DirectXVersion << " to v" << ProxyDirectXVersion;
	}
	~m_IDirect3DX()
	{
		if (Config.Exiting)
		{
			return;
		}

		if (Config.Dd7to9)
		{
			ReleaseD3DInterface();
		}
	}

	DWORD GetDirectXVersion() { return DDWRAPPER_TYPEX; }
	REFIID GetWrapperType() { return IID_IUnknown; }
	IDirect3D7 *GetProxyInterface() { return ProxyInterface; }
	m_IDirect3D7 *GetWrapperInterface() { return WrapperInterface; }

	/*** IUnknown methods ***/
	HRESULT QueryInterface(REFIID riid, LPVOID FAR * ppvObj, DWORD DirectXVersion);
	STDMETHOD(QueryInterface) (THIS_ REFIID, LPVOID FAR *) { return E_NOINTERFACE; }
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3D methods ***/
	STDMETHOD(Initialize)(THIS_ REFCLSID);
	HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK, LPVOID);
	HRESULT EnumDevices7(LPD3DENUMDEVICESCALLBACK7, LPVOID, bool = false);
	STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT*, LPUNKNOWN);
	STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL3*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT3*, LPUNKNOWN, DWORD);
	STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT);
	STDMETHOD(CreateDevice)(THIS_ REFCLSID, LPDIRECTDRAWSURFACE7, LPDIRECT3DDEVICE7*, LPUNKNOWN, DWORD);
	STDMETHOD(CreateVertexBuffer)(THIS_ LPD3DVERTEXBUFFERDESC, LPDIRECT3DVERTEXBUFFER7*, DWORD, LPUNKNOWN, DWORD);
	STDMETHOD(EnumZBufferFormats)(THIS_ REFCLSID, LPD3DENUMPIXELFORMATSCALLBACK, LPVOID);
	STDMETHOD(EvictManagedTextures)(THIS);

	// Helper functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3D :
			(DirectXVersion == 2) ? IID_IDirect3D2 :
			(DirectXVersion == 3) ? IID_IDirect3D3 :
			(DirectXVersion == 7) ? IID_IDirect3D7 : IID_IDirect3D7;
	}
	IDirect3D *GetProxyInterfaceV1() { return (IDirect3D *)ProxyInterface; }
	IDirect3D2 *GetProxyInterfaceV2() { return (IDirect3D2 *)ProxyInterface; }
	IDirect3D3 *GetProxyInterfaceV3() { return (IDirect3D3 *)ProxyInterface; }
	IDirect3D7 *GetProxyInterfaceV7() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
	void ClearDdraw() { ddrawParent = nullptr; }
	void ReleaseD3DInterface();
};
