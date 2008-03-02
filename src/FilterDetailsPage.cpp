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
//	CFilterDetailsPage class
//
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFilterDetailsPage, CMFCPropertyPage)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFilterDetailsPage class
//
//-----------------------------------------------------------------------------
CFilterDetailsPage::CFilterDetailsPage(LPUNKNOWN pUnk, HRESULT *phr) :
	CMFCPropertyPage(_T("FilterDetails"), pUnk, IDD, _T("Filter")),
	info(_T("root"))
{
	// retval
	if (phr) *phr = NOERROR;

}

CFilterDetailsPage::~CFilterDetailsPage()
{
	// todo
}

// overriden
BOOL CFilterDetailsPage::OnInitDialog()
{
	BOOL ok = CMFCPropertyPage::OnInitDialog();
	if (!ok) return FALSE;

	// create the tree
	CRect	rc;
	GetClientRect(&rc);

	ok = tree.Create(NULL, WS_CHILD | WS_VISIBLE, rc, this, IDC_TREE);
	if (!ok) return FALSE;

	GraphStudio::PropItem	*group;
	
	group = info.AddItem(new GraphStudio::PropItem(_T("Test Group")));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota"), 10));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota2"), 10));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota3"), _T("Stringova")));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota4"), 10));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota"), 10));

	group = info.AddItem(new GraphStudio::PropItem(_T("Podskupina")));
		group->AddItem(new GraphStudio::PropItem(_T("GUID"), CLSID_FilterGraph));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota2"), 10));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota3"), _T("Stringova")));
		group->AddItem(new GraphStudio::PropItem(_T("Hodnota"), true));

	tree.Initialize();
	tree.BuildPropertyTree(&info);

	return TRUE;
}

void CFilterDetailsPage::OnSize(UINT nType, int cx, int cy)
{
	if (IsWindow(tree)) tree.MoveWindow(0, 0, cx, cy);
}
