//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

namespace GraphStudio
{

	//-------------------------------------------------------------------------
	//
	//	FavoriteFilter class
	//
	//-------------------------------------------------------------------------
	class FavoriteFilter
	{
	public:
		CString		type;				// sw, dmo
		CString		name;				// filter name
		GUID		clsid_category;
		GUID		clsid_filter;
	public:
		FavoriteFilter();
		FavoriteFilter(const FavoriteFilter &f);
		~FavoriteFilter();
		FavoriteFilter &operator =(const FavoriteFilter &f);

		// helpers
		void FromTemplate(DSUtil::FilterTemplate &ft);
	};

	//-------------------------------------------------------------------------
	//
	//	FavoriteGroup class
	//
	//-------------------------------------------------------------------------
	class FavoriteGroup
	{
	public:
		CString					name;
		CArray<FavoriteFilter>	filters;
	public:
		FavoriteGroup();
		FavoriteGroup(const FavoriteGroup &g);
		~FavoriteGroup();
		FavoriteGroup &operator =(const FavoriteGroup &g);
	};


};


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

