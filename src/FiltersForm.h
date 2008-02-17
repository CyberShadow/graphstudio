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
//	CFiltersForm class
//
//-----------------------------------------------------------------------------
class CFiltersForm : 
	public CDialog,
	public GraphStudio::FilterListCallback
{
protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(CFiltersForm)

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CComboBox					combo_categories;
	CComboBox					combo_merit;
	CButton						btn_registry;
	CButton						btn_insert;
	CButton						btn_propertypage;
	CButton						btn_unregister;
	CButton						btn_mediatypes;
	CButton						check_favorite;
	GraphStudio::FilterListCtrl	list_filters;
	GraphStudio::TitleBar		title;
	CListCtrl					list_details;

	// enumerated stuff
	DSUtil::FilterCategories	categories;
	DSUtil::FilterTemplates		filters;

	// view to work with
	CGraphView					*view;

	enum {
		MERIT_MODE_ALL = 0,
		MERIT_MODE_DONOTUSE = 1,
		MERIT_MODE_DONOTUSE_GE = 2,
		MERIT_MODE_DONOTUSE_G = 3,
		MERIT_MODE_UNLIKELY = 4,
		MERIT_MODE_UNLIKELY_GE = 5,
		MERIT_MODE_UNLIKELY_G = 6,
		MERIT_MODE_NORMAL = 7,
		MERIT_MODE_NORMAL_GE = 8,
		MERIT_MODE_NORMAL_G = 9,
		MERIT_MODE_PREFERRED = 10,
		MERIT_MODE_PREFERRED_GE = 11,
		MERIT_MODE_PREFERRED_G = 12,
		MERIT_MODE_NON_STANDARD = 13
	};
	int		merit_mode;

public:
	CFiltersForm(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFiltersForm();

	// Dialog Data
	enum { IDD = IDD_DIALOG_FILTERS };

	BOOL DoCreateDialog();
	void OnInitialize();
	void OnComboCategoriesChange();
	void OnSize(UINT nType, int cx, int cy);
	void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT item);
	afx_msg void OnBnClickedButtonInsert();

	// filtering
	DSUtil::FilterTemplate *GetSelected();
	bool CanBeDisplayed(DSUtil::FilterTemplate &filter);
	afx_msg void OnComboMeritChange();
	afx_msg void OnFilterItemClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPropertypage();

	// filterlist callback
	virtual void OnItemDblClk(int item);

	afx_msg void OnBnClickedCheckFavorite();
};

int ConfigureInsertedFilter(IBaseFilter *filter);


