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
//	CFileSinkForm class
//
//-----------------------------------------------------------------------------
class CFileSinkForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CFileSinkForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:
	GraphStudio::TitleBar	title;
	CButton					radio_file;
	CButton					radio_url;
	CButton					button_browse;
	CComboBox				combo_file;
	CComboBox				combo_url;

	CString					result_file;

	// list of recent URLs/Files
	CArray<CString>	file_list;
	CArray<CString>	url_list;

	void LoadRecentlyUsedList(CString name, CArray<CString> &list);
	void SaveRecentlyUsedList(CString name, CArray<CString> &list);
	void UpdateList(CString item, CArray<CString> &list);
public:
	CFileSinkForm(CWnd* pParent = NULL);   
	virtual ~CFileSinkForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_FILESINK };

	afx_msg void OnBnClickedRadioFile();
	afx_msg void OnBnClickedRadioUrl();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonClear();
};
