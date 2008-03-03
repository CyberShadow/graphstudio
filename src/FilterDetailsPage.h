//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


//-----------------------------------------------------------------------------
//
//	CFilterDetailsPage class
//
//-----------------------------------------------------------------------------
class CFilterDetailsPage : public CMFCPropertyPage
{
protected:
	DECLARE_MESSAGE_MAP()

public:

	GraphStudio::PropertyTree		tree;
	GraphStudio::PropItem			info;

	CComPtr<IBaseFilter>			filter;

	enum { IDD = IDD_DIALOG_FILTERDETAILS };
public:
	CFilterDetailsPage(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~CFilterDetailsPage();
	
    virtual HRESULT OnConnect(IUnknown *pUnknown);
    virtual HRESULT OnDisconnect();

	// overriden
	virtual BOOL OnInitDialog();

	void OnSize(UINT nType, int cx, int cy);
};
