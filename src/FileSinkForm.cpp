//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FileSinkForm.h"


//-----------------------------------------------------------------------------
//
//	CFileSinkForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFileSinkForm, CDialog)
BEGIN_MESSAGE_MAP(CFileSinkForm, CDialog)
	ON_BN_CLICKED(IDC_RADIO_FILE, &CFileSinkForm::OnBnClickedRadioFile)
	ON_BN_CLICKED(IDC_RADIO_URL, &CFileSinkForm::OnBnClickedRadioUrl)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CFileSinkForm::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CFileSinkForm::OnBnClickedButtonClear)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFileSinkForm class
//
//-----------------------------------------------------------------------------

CFileSinkForm::CFileSinkForm(CWnd* pParent)	: 
	CDialog(CFileSinkForm::IDD, pParent)
{

}

CFileSinkForm::~CFileSinkForm()
{
}

void CFileSinkForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, button_browse);
	DDX_Control(pDX, IDC_RADIO_FILE, radio_file);
	DDX_Control(pDX, IDC_RADIO_URL, radio_url);
	DDX_Control(pDX, IDC_COMBO_FILE, combo_file);
	DDX_Control(pDX, IDC_COMBO_URL, combo_url);
}

BOOL CFileSinkForm::OnInitDialog()
{
	BOOL ret = __super::OnInitDialog();
	if (!ret) return FALSE;

	// load saved lists
	LoadRecentlyUsedList(_T("Sink-FileCache"), file_list);
	LoadRecentlyUsedList(_T("Sink-URLCache"), url_list);

	int i;
	for (i=0; i<file_list.GetCount(); i++) combo_file.AddString(file_list[i]);
	for (i=0; i<url_list.GetCount(); i++) combo_url.AddString(url_list[i]);

	OnBnClickedRadioFile();
	return TRUE;
}

void CFileSinkForm::SaveRecentlyUsedList(CString name, CArray<CString> &list)
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

void CFileSinkForm::UpdateList(CString item, CArray<CString> &list)
{
	for (int i=0; i<list.GetCount(); i++) {
		if (list[i] == item) {
			list.RemoveAt(i);
			break;
		}
	}
	list.InsertAt(0, item);
}


void CFileSinkForm::LoadRecentlyUsedList(CString name, CArray<CString> &list)
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

void CFileSinkForm::OnBnClickedRadioFile()
{
	radio_url.SetCheck(FALSE);
	radio_file.SetCheck(TRUE);
	combo_url.EnableWindow(FALSE);
	combo_file.EnableWindow(TRUE);
	button_browse.EnableWindow(TRUE);
}

void CFileSinkForm::OnBnClickedRadioUrl()
{
	radio_url.SetCheck(TRUE);
	radio_file.SetCheck(FALSE);
	combo_url.EnableWindow(TRUE);
	combo_file.EnableWindow(FALSE);
	button_browse.EnableWindow(FALSE);
}

void CFileSinkForm::OnBnClickedButtonBrowse()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*|");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		combo_file.SetWindowText(filename);
	}
}

void CFileSinkForm::OnOK()
{
	if (radio_url.GetCheck()) {
		combo_url.GetWindowText(result_file);

		if (result_file != _T("")) {
			UpdateList(result_file, url_list);
			SaveRecentlyUsedList(_T("URLCache"), url_list);
		}
	} else {
		combo_file.GetWindowText(result_file);
		if (result_file != _T("")) {
			UpdateList(result_file, file_list);
			SaveRecentlyUsedList(_T("Sink-FileCache"), file_list);
		}
	}

	EndDialog(IDOK);
}

void CFileSinkForm::OnBnClickedButtonClear()
{
	url_list.RemoveAll();
	file_list.RemoveAll();
	SaveRecentlyUsedList(_T("Sink-URLCache"), url_list);
	SaveRecentlyUsedList(_T("Sink-FileCache"), file_list);
	combo_file.ResetContent();
	combo_url.ResetContent();
}
