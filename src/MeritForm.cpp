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
//	CMeritChangeDialog class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CMeritChangeDialog, CDialog)
BEGIN_MESSAGE_MAP(CMeritChangeDialog, CDialog)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CMeritChangeDialog class
//
//-----------------------------------------------------------------------------

CMeritChangeDialog::CMeritChangeDialog(CWnd* pParent)	: 
	CDialog(CMeritChangeDialog::IDD, pParent)
{

}

CMeritChangeDialog::~CMeritChangeDialog()
{
}

void CMeritChangeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FILTER, label_filter);
}

BOOL CMeritChangeDialog::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// create a nice big font
	GraphStudio::MakeFont(font_filter, _T("Arial"), 12, true, false);
	label_filter.SetFont(&font_filter);
	label_filter.SetWindowText(filter_name);

	return TRUE;
}


bool ChangeMeritDialog(CString name, DWORD original_merit, DWORD &new_merit)
{
	CMeritChangeDialog	dlg;
	dlg.filter_name = name;

	int ret = dlg.DoModal();

	return true;
}

