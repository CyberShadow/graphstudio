//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "FiltersForm.h"


//-----------------------------------------------------------------------------
//
//	CFiltersForm class
//
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC(CFiltersForm, CDialog)

BEGIN_MESSAGE_MAP(CFiltersForm, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_CATEGORIES, &CFiltersForm::OnComboCategoriesChange)
	ON_WM_SIZE()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_BUTTON_INSERT, &CFiltersForm::OnBnClickedButtonInsert)
	ON_CBN_SELCHANGE(IDC_COMBO_MERIT, &CFiltersForm::OnComboMeritChange)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILTERS, &CFiltersForm::OnFilterItemClick)
	ON_BN_CLICKED(IDC_BUTTON_PROPERTYPAGE, &CFiltersForm::OnBnClickedButtonPropertypage)
	ON_BN_CLICKED(IDC_CHECK_FAVORITE, &CFiltersForm::OnBnClickedCheckFavorite)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CFiltersForm class
//
//-----------------------------------------------------------------------------

CFiltersForm::CFiltersForm(CWnd* pParent) : 
	CDialog(CFiltersForm::IDD, pParent),
	info(CString(_T("root")))
{

}

CFiltersForm::~CFiltersForm()
{
}

void CFiltersForm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TITLEBAR, title);
	DDX_Control(pDX, IDC_LIST_FILTERS, list_filters);
	DDX_Control(pDX, IDC_BUTTON_INSERT, btn_insert);
	DDX_Control(pDX, IDC_BUTTON_MEDIATYPES, btn_mediatypes);
	DDX_Control(pDX, IDC_BUTTON_PROPERTYPAGE, btn_propertypage);
	DDX_Control(pDX, IDC_BUTTON_UNREGISTER, btn_unregister);
	DDX_Control(pDX, IDC_CHECK_FAVORITE, check_favorite);
}

BOOL CFiltersForm::DoCreateDialog()
{
	BOOL ret = Create(IDD);
	if (!ret) return FALSE;

	// prepare titlebar
	title.ModifyStyle(0, WS_CLIPCHILDREN);
	title.ModifyStyleEx(0, WS_EX_CONTROLPARENT);


	// create buttons
	CRect	rc;
	rc.SetRect(0, 0, 125, 23);
	combo_categories.Create(WS_CHILD | WS_VISIBLE | CBS_SORT | CBS_DROPDOWNLIST, rc, &title, IDC_COMBO_CATEGORIES);
	combo_merit.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, rc, &title, IDC_COMBO_MERIT);
	btn_registry.Create(_T("Registry Check"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rc, &title, IDC_BUTTON_REGISTRY);

	tree_details.Create(NULL, WS_CHILD | WS_VISIBLE, rc, this, IDC_TREE);

	combo_categories.SetFont(GetFont());
	combo_merit.SetFont(GetFont());
	btn_registry.SetFont(GetFont());

	btn_registry.EnableWindow(FALSE);

	tree_details.Initialize();

	SetWindowPos(NULL, 0, 0, 700, 450, SWP_NOMOVE);

	OnInitialize();
	return TRUE;
};

// CFiltersForm message handlers
void CFiltersForm::OnInitialize()
{
	// Fill the categories combo
	int i;
	DragAcceptFiles(TRUE);

	list_filters.callback = this;

	// add some columns
	CRect	rc;
	list_filters.GetClientRect(&rc);

	DWORD	style = list_filters.GetExtendedStyle() | LVS_EX_DOUBLEBUFFER;
	list_filters.SetExtendedStyle(style);
	list_filters.InsertColumn(0, _T("Filter Name"), 0, rc.Width() - 20);

	// merits
	merit_mode = CFiltersForm::MERIT_MODE_ALL;
	combo_merit.ResetContent();
	combo_merit.AddString(_T("All Filters"));
	combo_merit.AddString(_T("= MERIT_DO_NOT_USE"));
	combo_merit.AddString(_T(">= MERIT_DO_NOT_USE"));
	combo_merit.AddString(_T("> MERIT_DO_NOT_USE"));
	combo_merit.AddString(_T("= MERIT_UNLIKELY "));
	combo_merit.AddString(_T(">= MERIT_UNLIKELY "));
	combo_merit.AddString(_T("> MERIT_UNLIKELY "));
	combo_merit.AddString(_T("= MERIT_NORMAL"));
	combo_merit.AddString(_T(">= MERIT_NORMAL"));
	combo_merit.AddString(_T("> MERIT_NORMAL"));
	combo_merit.AddString(_T("= MERIT_PREFERRED"));
	combo_merit.AddString(_T(">= MERIT_PREFERRED"));
	combo_merit.AddString(_T("> MERIT_PREFERRED"));
	combo_merit.AddString(_T("Non-Standard Merits"));
	combo_merit.SetCurSel(0);

	for (i=0; i<categories.categories.GetCount(); i++) {
		DSUtil::FilterCategory	&cat = categories.categories[i];

		// ignore empty categories
		DSUtil::FilterTemplates		filters;
		filters.Enumerate(cat);
		if (filters.filters.GetCount() > 0) {
			CString	n;
			n.Format(_T("%s (%d filters)"), cat.name, filters.filters.GetCount());
			int item = combo_categories.AddString(n);
			combo_categories.SetItemDataPtr(item, (void*)&cat);

			if (cat.clsid == CLSID_LegacyAmFilterCategory) {
				combo_categories.SetCurSel(item);
				OnComboCategoriesChange();
			}
		}
	}
}

void CFiltersForm::OnComboCategoriesChange()
{
	int item = combo_categories.GetCurSel();
	DSUtil::FilterCategory	*cat = (DSUtil::FilterCategory*)combo_categories.GetItemDataPtr(item);
	if (!cat) return ;

	list_filters.DeleteAllItems();
	filters.Enumerate(*cat);

	int i;
	for (i=0; i<filters.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = filters.filters[i];

		// pridame itemu
		if (CanBeDisplayed(filter)) {
			int item = list_filters.InsertItem(LVIF_PARAM | LVIF_TEXT, 0, filter.name, 0, 0, 0, (LPARAM)&filter);
			list_filters.SetItemData(item, (DWORD_PTR)&filter);
		}
	}
}

bool CFiltersForm::CanBeDisplayed(DSUtil::FilterTemplate &filter)
{
	switch (merit_mode) {
	case CFiltersForm::MERIT_MODE_ALL:				return true;
	case CFiltersForm::MERIT_MODE_DONOTUSE:			return (filter.merit == MERIT_DO_NOT_USE);
	case CFiltersForm::MERIT_MODE_DONOTUSE_GE:		return (filter.merit >= MERIT_DO_NOT_USE);
	case CFiltersForm::MERIT_MODE_DONOTUSE_G:		return (filter.merit > MERIT_DO_NOT_USE);
	case CFiltersForm::MERIT_MODE_UNLIKELY:			return (filter.merit == MERIT_UNLIKELY);
	case CFiltersForm::MERIT_MODE_UNLIKELY_GE:		return (filter.merit >= MERIT_UNLIKELY);
	case CFiltersForm::MERIT_MODE_UNLIKELY_G:		return (filter.merit > MERIT_UNLIKELY);
	case CFiltersForm::MERIT_MODE_NORMAL:			return (filter.merit == MERIT_NORMAL);
	case CFiltersForm::MERIT_MODE_NORMAL_GE:		return (filter.merit >= MERIT_NORMAL);
	case CFiltersForm::MERIT_MODE_NORMAL_G:			return (filter.merit > MERIT_NORMAL);
	case CFiltersForm::MERIT_MODE_PREFERRED:		return (filter.merit == MERIT_PREFERRED);
	case CFiltersForm::MERIT_MODE_PREFERRED_GE:		return (filter.merit >= MERIT_PREFERRED);
	case CFiltersForm::MERIT_MODE_PREFERRED_G:		return (filter.merit > MERIT_PREFERRED);
	case CFiltersForm::MERIT_MODE_NON_STANDARD:
		{
			if (filter.merit == MERIT_DO_NOT_USE ||
				filter.merit == MERIT_UNLIKELY ||
				filter.merit == MERIT_NORMAL ||
				filter.merit == MERIT_PREFERRED) return false;
			return true;
		}
		break;

	default:
		return false;
	}
}


void CFiltersForm::OnSize(UINT nType, int cx, int cy)
{
	// resize our controls along...
	CRect		rc, rc2;
	GetClientRect(&rc);

	// compute anchor lines
	int	right_x = rc.Width() - 320;
	int merit_combo_width = 180;

	title.GetClientRect(&rc2);
	title.SetWindowPos(NULL, 0, 0, cx, rc2.Height(), SWP_SHOWWINDOW);
	
	int details_top = rc2.Height();
	list_filters.SetWindowPos(NULL, 0, rc2.Height(), right_x, rc.Height() - rc2.Height(), SWP_SHOWWINDOW);
	list_filters.GetClientRect(&rc2);
	list_filters.SetColumnWidth(0, rc2.Width()-10);

	// details
	tree_details.SetWindowPos(NULL, right_x, details_top, rc.Width()-right_x, rc.Height() - 100-details_top, SWP_SHOWWINDOW);

	check_favorite.GetWindowRect(&rc2);
	check_favorite.SetWindowPos(NULL, right_x+8, rc.Height()-100+8, rc.Width()-16-right_x, rc2.Height(), SWP_SHOWWINDOW);

	// combo boxes
	combo_categories.GetWindowRect(&rc2);
	combo_categories.SetWindowPos(NULL, 4, 6, right_x-2*8 - 8-merit_combo_width, rc2.Height(), SWP_SHOWWINDOW);
	combo_merit.GetWindowRect(&rc2);
	combo_merit.SetWindowPos(NULL, right_x-4 - merit_combo_width, 6, merit_combo_width, rc2.Height(), SWP_SHOWWINDOW);

	// buttons
	btn_registry.GetWindowRect(&rc2);
	int	btn_height = rc2.Height();

	btn_registry.SetWindowPos(NULL, rc.Width() - 4 - rc2.Width(), 5, rc2.Width(), btn_height, SWP_SHOWWINDOW);
	btn_insert.GetWindowRect(&rc2);
	btn_insert.SetWindowPos(NULL, right_x+8, rc.Height() - 2*(8+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW);
	btn_propertypage.SetWindowPos(NULL, right_x+8, rc.Height() - 1*(8+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW);
	btn_mediatypes.SetWindowPos(NULL, rc.Width() - 8 - rc2.Width(), rc.Height() - 2*(8+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW);
	btn_unregister.SetWindowPos(NULL, rc.Width() - 8 - rc2.Width(), rc.Height() - 1*(8+btn_height), rc2.Width(), btn_height, SWP_SHOWWINDOW);


	// invalidate all controls
	title.Invalidate();
	//combo_categories.Invalidate();
	//combo_merit.Invalidate();
	btn_registry.Invalidate();
	btn_insert.Invalidate();
	btn_propertypage.Invalidate();
	btn_mediatypes.Invalidate();
	btn_unregister.Invalidate();

	list_filters.Invalidate();
	tree_details.Invalidate();

}

void CFiltersForm::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT item)
{
	if (item->CtlType == ODT_LISTVIEW) {
		if (item->CtlID == IDC_LIST_FILTERS) {
		
			// fixed height
			item->itemHeight = 18;
			return ;

		}
	}

	// base clasu
	__super::OnMeasureItem(nIDCtl, item);
}

void CFiltersForm::OnItemDblClk(int item)
{
	OnBnClickedButtonInsert();
}

void CFiltersForm::OnBnClickedButtonInsert()
{
	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = filter->CreateInstance(&instance);
		if (FAILED(hr)) {
			// display error message
		} else {
			
			// now check for a few interfaces
			int ret = ConfigureInsertedFilter(instance);
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {
				// add the filter to graph
				hr = view->graph.AddFilter(instance, filter->name);
				if (FAILED(hr)) {
					// display error message
				} else {
					view->graph.SmartPlacement();
					view->Invalidate();
				}
			}
		}
		instance = NULL;
	}
}

void CFiltersForm::OnComboMeritChange()
{
	merit_mode = combo_merit.GetCurSel();
	OnComboCategoriesChange();
}

void CFiltersForm::OnFilterItemClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if (!(pNMLV->uOldState & ODS_SELECTED) &&
		(pNMLV->uNewState & ODS_SELECTED)) {
		DSUtil::FilterTemplate	*filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(pNMLV->iItem);

		if (filter) {

			// display information
			info.Clear();
			GraphStudio::PropItem	*group;

			group = info.AddItem(new GraphStudio::PropItem(_T("Filter Details")));
				CString	type;
				switch (filter->type) {
				case DSUtil::FilterTemplate::FT_DMO:		type = _T("DMO"); break;
				case DSUtil::FilterTemplate::FT_KSPROXY:	type = _T("WDM"); break;
				case DSUtil::FilterTemplate::FT_FILTER:		type = _T("Standard"); break;
				case DSUtil::FilterTemplate::FT_ACM_ICM:	type = _T("ACM/ICM"); break;
				case DSUtil::FilterTemplate::FT_PNP:		type = _T("Plug && Play"); break;
				}	
				group->AddItem(new GraphStudio::PropItem(_T("DisplayName"), filter->moniker_name));
				group->AddItem(new GraphStudio::PropItem(_T("Type"), type));
				GraphStudio::GetFilterDetails(filter->clsid, group);

			tree_details.BuildPropertyTree(&info);

			// favorite filter ?
			GraphStudio::Favorites	*favorites = GraphStudio::Favorites::GetInstance();
			if (favorites->IsFavorite(*filter)) {
				check_favorite.SetCheck(TRUE);
			} else {
				check_favorite.SetCheck(FALSE);
			}
		}
		/*
		list_details.DeleteAllItems();

			int ni;
			ni = list_details.InsertItem(0, _T("Name"));	list_details.SetItemText(ni, 1, filter->name);
			ni = list_details.InsertItem(1, _T("File"));	list_details.SetItemText(ni, 1, filter->file);

			LPOLESTR	str;
			StringFromCLSID(filter->clsid, &str);
			CString		clsid_str(str);
			CoTaskMemFree(str);
			ni = list_details.InsertItem(2, _T("CLSID"));	list_details.SetItemText(ni, 1, clsid_str);

			CString		m;
			m.Format(_T("0x%08X"), filter->merit);
			ni = list_details.InsertItem(3, _T("Merit"));	list_details.SetItemText(ni, 1, m);

		}
		*/
	}

}

void CFiltersForm::OnBnClickedCheckFavorite()
{
	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		BOOL					check		= check_favorite.GetCheck();
		GraphStudio::Favorites	*favorites	= GraphStudio::Favorites::GetInstance();

		if (check) {
			favorites->AddFavorite(*filter);
			if (view && view->form_favorites) {
				view->form_favorites->UpdateTree();
			}
		} else {
			HTREEITEM item = favorites->RemoveFavorite(*filter);
			if (item != NULL) {
				// remove this item
				if (view && view->form_favorites) {
					view->form_favorites->RemoveFilter(item);
					view->form_favorites->UpdateFavoriteMenu();
				}
			}
		}

		// todo: update tree
		favorites->Save();
	}
}

DSUtil::FilterTemplate *CFiltersForm::GetSelected()
{
	POSITION pos = list_filters.GetFirstSelectedItemPosition();
	if (pos) {
		int item = list_filters.GetNextSelectedItem(pos);
		DSUtil::FilterTemplate *filter = (DSUtil::FilterTemplate*)list_filters.GetItemData(item);
		return filter;
	}

	return NULL;
}

void CFiltersForm::OnBnClickedButtonPropertypage()
{
	// create a new instance of a filter and display
	// it's property page.
	// the filter will be destroyed once the page is closed
	// now we try to add a filter

	DSUtil::FilterTemplate *filter = GetSelected();
	if (filter) {

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = filter->CreateInstance(&instance);
		if (FAILED(hr)) {
			// display error message
		} else {


			CString			title = filter->name + _T(" Properties");
			CPropertyForm	*page = new CPropertyForm();
			int ret = page->DisplayPages(instance, instance, title, view);
			if (ret < 0) {
				delete page;
				return ;
			}

			// add to the list
			view->property_pages.Add(page);


		}
		instance = NULL;
	}

}


int ConfigureInsertedFilter(IBaseFilter *filter)
{
	int	ret = 0;
	HRESULT hr;

	//-------------------------------------------------------------
	//	IFileSourceFilter
	//-------------------------------------------------------------
	CComPtr<IFileSourceFilter>	fs;
	hr = filter->QueryInterface(IID_IFileSourceFilter, (void**)&fs);
	if (SUCCEEDED(hr)) {
		CFileSrcForm		src_form;
		ret = src_form.DoModal();
		if (ret == IDOK) {
			hr = fs->Load((LPCOLESTR)src_form.result_file, NULL);
			if (FAILED(hr)) {
				MessageBox(NULL, _T("Cannot load specified file"), _T("Error"), MB_ICONERROR);
			}
			ret = 0;
		} else {
			// cancel the filter
			ret = -1;
		}
		fs = NULL;
	}

	if (ret < 0) return -1;

	//-------------------------------------------------------------
	//	IFileSinkFilter
	//-------------------------------------------------------------
	CComPtr<IFileSinkFilter>	fsink;
	hr = filter->QueryInterface(IID_IFileSinkFilter, (void**)&fsink);
	if (SUCCEEDED(hr)) {
		CFileSinkForm		sink_form;
		ret = sink_form.DoModal();
		if (ret == IDOK) {
			hr = fsink->SetFileName((LPCOLESTR)sink_form.result_file, NULL);
			if (FAILED(hr)) {
				MessageBox(NULL, _T("Cannot write specified file"), _T("Error"), MB_ICONERROR);
			}
			ret = 0;
		} else {
			// cancel the filter
			ret = -1;
		}
		fsink = NULL;
	}
	if (ret < 0) return -1;

	return ret;
}

