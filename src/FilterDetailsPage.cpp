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
	info(_T("root")),
	filter(NULL)
{
	// retval
	if (phr) *phr = NOERROR;

}

CFilterDetailsPage::~CFilterDetailsPage()
{
	// todo
}


HRESULT CFilterDetailsPage::OnConnect(IUnknown *pUnknown)
{
	HRESULT hr = pUnknown->QueryInterface(IID_IBaseFilter, (void**)&filter);
	if (FAILED(hr)) return E_FAIL;
	return NOERROR;
}

HRESULT CFilterDetailsPage::OnDisconnect()
{
	filter = NULL;
	return NOERROR;
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
	info.Clear();

	GraphStudio::Filter		gfilter(NULL);
	int						ret;

	gfilter.LoadFromFilter(filter);

	group = info.AddItem(new GraphStudio::PropItem(_T("Filter Details")));
		group->AddItem(new GraphStudio::PropItem(_T("Name"), gfilter.display_name));
		group->AddItem(new GraphStudio::PropItem(_T("CLSID"), gfilter.clsid));

		CString	type;
		switch (gfilter.filter_type) {
		case GraphStudio::Filter::FILTER_DMO:		type = _T("DMO"); break;
		case GraphStudio::Filter::FILTER_WDM:		type = _T("WDM"); break;
		case GraphStudio::Filter::FILTER_STANDARD:	type = _T("Standard"); break;
		case GraphStudio::Filter::FILTER_UNKNOWN:	type = _T("Unknown"); break;
		}	
		group->AddItem(new GraphStudio::PropItem(_T("Type"), type));

	tree.Initialize();
	tree.BuildPropertyTree(&info);

	return TRUE;
}

void CFilterDetailsPage::OnSize(UINT nType, int cx, int cy)
{
	if (IsWindow(tree)) tree.MoveWindow(0, 0, cx, cy);
}
