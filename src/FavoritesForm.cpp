//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"


//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFavoritesForm, CDialog)

BEGIN_MESSAGE_MAP(CFavoritesForm, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFavoritesForm class
//
//-----------------------------------------------------------------------------

CFavoritesForm::CFavoritesForm(CWnd* pParent) : 
	CDialog(CFavoritesForm::IDD, pParent)
{

}

CFavoritesForm::~CFavoritesForm()
{
}

void CFavoritesForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_TREE_FAVORITES, tree);
}

BOOL CFavoritesForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CBitmap	bmp;
	bmp.LoadBitmap(IDB_BITMAP_TREE);
	image_list.Create(17, 20, ILC_COLOR32, 1, 1);
	
	image_list.Add(&bmp, RGB(255,0,255));

	tree.SetImageList(&image_list, TVSIL_NORMAL);
	tree.SetImageList(&image_list, TVSIL_STATE);
	tree.SetItemHeight(20);

	OnInitialize();

	SetWindowPos(NULL, 0, 0, 400, 300, SWP_NOMOVE);
	return TRUE;
};

void CFavoritesForm::OnInitialize()
{
	// skusime nahodit nejake itemy do stromu
	HTREEITEM item = tree.InsertItem(_T("MONOGRAM Filters"), 1, 1);
	tree.InsertItem(_T("MONOGRAM AAC Encoder"), 0, 0, item);
	tree.InsertItem(_T("MONOGRAM AAC Decoder"), 0, 0, item);
	tree.InsertItem(_T("MONOGRAM AMR Encoder"), 0, 0, item);
	tree.InsertItem(_T("MONOGRAM Multigraph Sink"), 0, 0, item);
	tree.InsertItem(_T("MONOGRAM Multigraph Source"), 0, 0, item);
	item = tree.InsertItem(_T("ffdshow Filters"), 1, 1);
	tree.InsertItem(_T("ffdshow Audio Decoder"), 0, 0, item);
	tree.InsertItem(_T("ffdshow Audio Processor"), 0, 0, item);
	tree.InsertItem(_T("ffdshow MPEG-4 Video Decoder"), 0, 0, item);
	tree.InsertItem(_T("ffdshow raw video filter"), 0, 0, item);
	tree.InsertItem(_T("Dump"), 0, 0);
}


void CFavoritesForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	// compute anchor lines
	int	right_x = rc.Width() - 320;
	int merit_combo_width = 180;

	if (IsWindow(title)) {

		title.GetClientRect(&rc2);
		title.SetWindowPos(NULL, 0, 0, cx, rc2.Height(), SWP_SHOWWINDOW);

		tree.SetWindowPos(NULL, 0, rc2.Height(), cx, rc.Height() - rc2.Height(), SWP_SHOWWINDOW);

		// invalidate all controls
		title.Invalidate();

	}
}




namespace GraphStudio
{

	//-------------------------------------------------------------------------
	//
	//	FavoriteFilter class
	//
	//-------------------------------------------------------------------------

	FavoriteFilter::FavoriteFilter() :
		type(_T("sw")),
		name(_T("")),
		clsid_category(GUID_NULL),
		clsid_filter(GUID_NULL)
	{
	}

	FavoriteFilter::FavoriteFilter(const FavoriteFilter &f) :
		type(f.type),
		name(f.name),
		clsid_category(f.clsid_category),
		clsid_filter(f.clsid_filter)
	{
	}

	FavoriteFilter::~FavoriteFilter()
	{
		// nic
	}

	FavoriteFilter &FavoriteFilter::operator =(const GraphStudio::FavoriteFilter &f)
	{
		type = f.type;
		name = f.name;
		clsid_category = f.clsid_category;
		clsid_filter = f.clsid_filter;
		return *this;
	}

	void FavoriteFilter::FromTemplate(DSUtil::FilterTemplate &ft)
	{
		type = _T("sw");		// for now... :-
		name = ft.name;
		clsid_category = ft.category;
		clsid_filter   = ft.clsid;
	}

	//-------------------------------------------------------------------------
	//
	//	FavoriteGroup class
	//
	//-------------------------------------------------------------------------

	FavoriteGroup::FavoriteGroup() :
		name(_T(""))
	{
	}

	FavoriteGroup::FavoriteGroup(const FavoriteGroup &g) :
		name(g.name)
	{
		filters.Append(g.filters);
	}

	FavoriteGroup::~FavoriteGroup()
	{
		filters.RemoveAll();
	}

	FavoriteGroup &FavoriteGroup::operator =(const FavoriteGroup &g)
	{
		filters.RemoveAll();
		filters.Append(g.filters);
		name = g.name;
		return *this;
	}


};






