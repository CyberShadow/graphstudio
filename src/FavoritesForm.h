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
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------
class CFavoritesForm : public CDialog
{
protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CFavoritesForm)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:

	GraphStudio::TitleBar		title;
	CTreeCtrl					tree;
	CImageList					image_list;

public:
	CFavoritesForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFavoritesForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_FAVORITES };

	BOOL DoCreateDialog();
	void OnInitialize();
	void OnSize(UINT nType, int cx, int cy);

};

