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
//	CRenderUrlForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CRenderUrlForm, CDialog)
BEGIN_MESSAGE_MAP(CRenderUrlForm, CDialog)
	ON_BN_CLICKED(IDC_RADIO_URL, &CRenderUrlForm::OnBnClickedRadioUrl)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CRenderUrlForm::OnBnClickedButtonClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CRenderUrlForm class
//
//-----------------------------------------------------------------------------

CRenderUrlForm::CRenderUrlForm(CWnd* pParent)	: 
	CDialog(CRenderUrlForm::IDD, pParent)
{

}

CRenderUrlForm::~CRenderUrlForm()
{
}

void CRenderUrlForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_RADIO_URL, radio_url);
	DDX_Control(pDX, IDC_COMBO_URL, combo_url);
}

BOOL CRenderUrlForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// load saved lists
	LoadRecentlyUsedList(_T("MRU-URLCache"), url_list);

	int i;
	for (i=0; i<url_list.GetCount(); i++) combo_url.AddString(url_list[i]);

	OnBnClickedRadioUrl();
	return TRUE;
}

void CRenderUrlForm::SaveRecentlyUsedList(CString name, CArray<CString> &list)
{
	CString		count_name;
	count_name = name + _T("_count");
	int count = list.GetCount();
	// limit to 30
	if (count > 30) count = 30;

	AfxGetApp()->WriteProfileInt(_T("Settings"), count_name, count);

	for (int i=0; i<count; i++) {
		CString		key;
		key.Format(_T("%s_%d"), name, i);
		AfxGetApp()->WriteProfileString(_T("Settings"), key, list[i]);
	}
}

void CRenderUrlForm::UpdateList(CString item, CArray<CString> &list)
{
	for (int i=0; i<list.GetCount(); i++) {
		if (list[i] == item) {
			list.RemoveAt(i);
			break;
		}
	}
	list.InsertAt(0, item);
}


void CRenderUrlForm::LoadRecentlyUsedList(CString name, CArray<CString> &list)
{
	CString		count_name;
	count_name = name + _T("_count");
	int count = AfxGetApp()->GetProfileInt(_T("Settings"), count_name, 0);

	// limit to 30
	if (count > 30) count = 30;
	list.RemoveAll();

	for (int i=0; i<count; i++) {
		CString		item;
		CString		key;
		key.Format(_T("%s_%d"), name, i);
		item = AfxGetApp()->GetProfileString(_T("Settings"), key, _T(""));
		if (item != _T("")) {
			list.Add(item);
		}
	}
}

void CRenderUrlForm::OnBnClickedRadioUrl()
{
	radio_url.SetCheck(TRUE);
	combo_url.EnableWindow(TRUE);
}

void CRenderUrlForm::OnOK()
{
	combo_url.GetWindowText(result_file);

	if (result_file != _T("")) {
		UpdateList(result_file, url_list);
		SaveRecentlyUsedList(_T("MRU-URLCache"), url_list);
	}

	EndDialog(IDOK);
}

void CRenderUrlForm::OnBnClickedButtonClear()
{
	url_list.RemoveAll();
	SaveRecentlyUsedList(_T("MRU-URLCache"), url_list);
	combo_url.ResetContent();
}
