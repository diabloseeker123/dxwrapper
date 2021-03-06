#pragma once

class m_IDirect3DTextureX : public IUnknown, public AddressLookupTableDdrawObject
{
private:
	IDirect3DTexture2 *ProxyInterface = nullptr;
	DWORD ProxyDirectXVersion;
	ULONG RefCount = 1;

	// Convert Texture
	m_IDirect3DDeviceX **D3DDeviceInterface = nullptr;

	// Store d3d texture version wrappers
	m_IDirect3DTexture *WrapperInterface;
	m_IDirect3DTexture2 *WrapperInterface2;

public:
	m_IDirect3DTextureX(IDirect3DTexture2 *aOriginal, DWORD DirectXVersion) : ProxyInterface(aOriginal)
	{
		ProxyDirectXVersion = GetGUIDVersion(ConvertREFIID(GetWrapperType(DirectXVersion)));

		if (ProxyDirectXVersion != DirectXVersion)
		{
			LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);
		}
		else
		{
			LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ") v" << DirectXVersion);
		}

		WrapperInterface = new m_IDirect3DTexture((LPDIRECT3DTEXTURE)ProxyInterface, this);
		WrapperInterface2 = new m_IDirect3DTexture2((LPDIRECT3DTEXTURE2)ProxyInterface, this);

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	m_IDirect3DTextureX(m_IDirect3DDeviceX **D3DDInterface, DWORD DirectXVersion, IDirectDrawSurface7 *lpSurface) : D3DDeviceInterface(D3DDInterface), ProxyInterface((IDirect3DTexture2*)lpSurface)
	{
		ProxyDirectXVersion = (!Config.Dd7to9) ? 7 : 9;

		LOG_LIMIT(3, "Creating device " << __FUNCTION__ << "(" << this << ")" << " converting device from v" << DirectXVersion << " to v" << ProxyDirectXVersion);

		WrapperInterface = new m_IDirect3DTexture((LPDIRECT3DTEXTURE)ProxyInterface, this);
		WrapperInterface2 = new m_IDirect3DTexture2((LPDIRECT3DTEXTURE2)ProxyInterface, this);

		ProxyAddressLookupTable.SaveAddress(this, ProxyInterface);
	}
	~m_IDirect3DTextureX()
	{
		LOG_LIMIT(3, __FUNCTION__ << "(" << this << ")" << " deleting device!");

		WrapperInterface->DeleteMe();
		WrapperInterface2->DeleteMe();

		ProxyAddressLookupTable.DeleteAddress(this);
	}

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DTexture methods ***/
	STDMETHOD(Initialize)(THIS_ LPDIRECT3DDEVICE, LPDIRECTDRAWSURFACE);
	STDMETHOD(GetHandle)(THIS_ LPDIRECT3DDEVICE2, LPD3DTEXTUREHANDLE);
	STDMETHOD(PaletteChanged)(THIS_ DWORD, DWORD);
	STDMETHOD(Load)(THIS_ LPDIRECT3DTEXTURE2);
	STDMETHOD(Unload)(THIS);

	// Wrapper interface functions
	REFIID GetWrapperType(DWORD DirectXVersion)
	{
		return (DirectXVersion == 1) ? IID_IDirect3DTexture :
			(DirectXVersion == 2) ? IID_IDirect3DTexture2 : IID_IUnknown;
	}
	IDirect3DTexture *GetProxyInterfaceV1() { return (IDirect3DTexture *)ProxyInterface; }
	IDirect3DTexture2 *GetProxyInterfaceV2() { return ProxyInterface; }
	void *GetWrapperInterfaceX(DWORD DirectXVersion);
};
