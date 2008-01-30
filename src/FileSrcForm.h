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
//	CFileSrcForm class
//
//-----------------------------------------------------------------------------
class CFileSrcForm : public CDialog
{
protected:
	DECLARE_DYNAMIC(CFileSrcForm)
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);
public:
	CButton			radio_file;
	CButton			radio_url;
	CButton			button_browse;
	CComboBox		combo_file;
	CComboBox		combo_url;

	CString			result_file;

	// list of recent URLs/Files
	CArray<CString>	file_list;
	CArray<CString>	url_list;

	void LoadRecentlyUsedList(CString name, CArray<CString> &list);
	void SaveRecentlyUsedList(CString name, CArray<CString> &list);
	void UpdateList(CString item, CArray<CString> &list);
public:
	CFileSrcForm(CWnd* pParent = NULL);   
	virtual ~CFileSrcForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_ASYNCOPEN };

	afx_msg void OnBnClickedRadioFile();
	afx_msg void OnBnClickedRadioUrl();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonClear();
};
