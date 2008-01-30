//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "graphView.h"
#include <atlbase.h>
#include <atlpath.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//-----------------------------------------------------------------------------
//
//	CGraphView class
//
//-----------------------------------------------------------------------------

IMPLEMENT_DYNCREATE(CGraphView, DisplayView)

BEGIN_MESSAGE_MAP(CGraphView, GraphStudio::DisplayView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	//ON_COMMAND(ID_FILE_RENDERDVD, &CGraphView::OnFileRenderdvd)
	ON_COMMAND(ID_BUTTON_PLAY, &CGraphView::OnPlayClick)
	ON_COMMAND(ID_BUTTON_PAUSE, &CGraphView::OnPauseClick)
	ON_COMMAND(ID_BUTTON_STOP, &CGraphView::OnStopClick)
	ON_COMMAND(ID_FILE_NEW, &CGraphView::OnNewClick)
	ON_COMMAND(ID_FILE_OPEN, &CGraphView::OnFileOpenClick)
	ON_COMMAND(ID_FILE_SAVE, &CGraphView::OnFileSaveClick)
	ON_COMMAND(ID_FILE_SAVE_AS, &CGraphView::OnFileSaveAsClick)
	ON_COMMAND(ID_FILE_RENDERFILE, &CGraphView::OnRenderFileClick)
	ON_COMMAND(ID_GRAPH_INSERTFILTER, &CGraphView::OnGraphInsertfilter)
	ON_COMMAND(ID_VIEW_GRAPHEVENTS, &CGraphView::OnViewGraphEvents)

	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_DROPFILES()

	ON_UPDATE_COMMAND_UI(ID_BUTTON_PLAY, &CGraphView::OnUpdatePlayButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PAUSE, &CGraphView::OnUpdatePauseButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_STOP, &CGraphView::OnUpdateStopButton)
	ON_UPDATE_COMMAND_UI(ID_FILE_RENDERFILE, &CGraphView::OnUpdateRenderMediaFile)

	
	ON_COMMAND(ID_VIEW_TEXTINFORMATION, &CGraphView::OnViewTextInformation)
	ON_COMMAND(ID_GRAPH_INSERTFILESOURCE, &CGraphView::OnGraphInsertFileSource)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//	CGraphView class
//
//-----------------------------------------------------------------------------

CGraphView::CGraphView()
{
	// TODO: add construction code here
	form_filters = NULL;
	form_events = NULL;
	form_textinfo = NULL;
	filename = _T("");
	can_save = false;
}

CGraphView::~CGraphView()
{
	if (form_filters) { form_filters->DestroyWindow(); delete form_filters; }
	if (form_events) { form_events->DestroyWindow(); delete form_events; }
	if (form_textinfo) { form_textinfo->DestroyWindow(); delete form_textinfo; }
}

BOOL CGraphView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!__super::PreCreateWindow(cs)) return FALSE;

	return TRUE;
}

// CGraphView printing

BOOL CGraphView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGraphView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CGraphView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CGraphView::OnDestroy()
{
	ClosePropertyPages();
	graph.MakeNew();

	__super::OnDestroy();
}


// CGraphView diagnostics

#ifdef _DEBUG
void CGraphView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGraphDoc* CGraphView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGraphDoc)));
	return (CGraphDoc*)m_pDocument;
}
#endif //_DEBUG


// CGraphView message handlers
void CGraphView::OnInit()
{
	DragAcceptFiles(TRUE);

	// initialize our event logger
	form_events = new CEventsForm(NULL);
	form_events->view = this;
	form_events->DoCreateDialog();

	form_textinfo = new CTextInfoForm(NULL);
	form_textinfo->Create(IDD_DIALOG_TEXTVIEW);
	form_textinfo->view = this;
	form_textinfo->OnInitialize();

	graph.wndEvents = *form_events;
	graph.MakeNew();

	// set the seeking bar data
	CMainFrame *frame = (CMainFrame*)GetParentFrame();
	frame->m_wndSeekingBar.SetGraphView(this);

	UpdateGraphState();
}

void CGraphView::OnFileRenderdvd()
{
}

void CGraphView::OnPlayClick()
{
	if (graph.mc) {
		graph.mc->Run();
	}
	UpdateGraphState();
}

void CGraphView::OnStopClick()
{
	if (graph.mc) {
		graph.Seek(0);
		graph.mc->Stop();
	}
	UpdateGraphState();
}

void CGraphView::OnPauseClick()
{
	if (graph.mc) {
		graph.mc->Pause();
	}
	UpdateGraphState();
}

void CGraphView::OnNewClick()
{
	OnStopClick();
	GetDocument()->SetTitle(_T("Untitled"));
	ClosePropertyPages();
	graph.MakeNew();

	filename = _T("");
	can_save = false;

	Invalidate();
}

void CGraphView::OnFileSaveClick()
{
	if (can_save) {
		int ret = graph.SaveGRF(filename);
		if (ret < 0) {
			// error
			MessageBox(_T("Cannot save file"));
		}
	} else {
		OnFileSaveAsClick();
	}
}

void CGraphView::OnFileSaveAsClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("GraphEdit Files|*.grf");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {

		ret = graph.SaveGRF(filename);	
		if (ret < 0) {
			MessageBox(_T("Cannot save file"));
			return ;
		}

		this->filename = filename;
		can_save = true;

		CPath	path(filename);
		CGraphDoc *doc = GetDocument();
		int pos = path.FindFileName();
		CString	short_fn = filename;
		short_fn.Delete(0, pos);
		doc->SetTitle(short_fn);
	}
}

int CGraphView::TryOpenFile(CString fn)
{
	int		ret;
	CPath	path(fn);
	CString	ext = path.GetExtension();

	ext = ext.MakeLower();
	if (ext == _T(".grf")) {
		OnNewClick();
		ret = graph.LoadGRF(fn);	

		if (ret == 0) {
			filename = fn;
			can_save = true;
		}

	} else {
		OnNewClick();
		ret = graph.RenderFile(fn);
	}

	if (ret < 0) return ret;

	CGraphDoc *doc = GetDocument();
	int pos = path.FindFileName();
	CString	short_fn = fn;
	short_fn.Delete(0, pos);
	doc->SetTitle(short_fn);

	graph.RefreshFilters();
	graph.SmartPlacement();
	Invalidate();
	return 0;
}

void CGraphView::OnFileOpenClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("GraphEdit Files|*.grf");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		ret = TryOpenFile(filename);
		if (ret < 0) {
			MessageBox(_T("Cannot open file"));
		}
	}

}

void CGraphView::OnRenderFileClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("All Files|*.*");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {

		int ret = graph.RenderFile(filename);
		if (ret < 0) {
			MessageBox(_T("Cannot render file"));
		}

		graph.RefreshFilters();
		graph.SmartPlacement();
		Invalidate();
	}
}

void CGraphView::OnGraphInsertfilter()
{
	if (!form_filters) {	
		form_filters = new CFiltersForm();
		form_filters->view = this;
		form_filters->DoCreateDialog();
	}

	// display the form
	form_filters->ShowWindow(SW_SHOW);
}

void CGraphView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!(nFlags & (MK_SHIFT | MK_ALT | MK_CONTROL))) {
		switch (nChar) {
		case VK_DELETE:
			{
				FILTER_STATE	state = State_Running;
				if (graph.GetState(state, 0) != 0) {
					state = State_Running;
				}

				if (state != State_Stopped) return ;

				// delete selected objects
				graph.DeleteSelected();
				Invalidate();
			}
			break;
		}
	}
}

void CGraphView::OnViewGraphEvents()
{
	if (form_events) form_events->ShowWindow(SW_SHOW);
}

void CGraphView::UpdateGraphState()
{
	// now we will ask the graph for the state.
	// if we're in the middle of change, we setup a timer
	// that will call us back
	int ret = graph.GetState(graph_state, 10);
	if (ret < 0) {
		// error ?

	} else
	if (ret == 0) {

		state_ready = true;
		switch (graph_state) {
		case State_Stopped:		OnGraphStopped(); break;
		case State_Paused:		OnGraphPaused(); break;
		case State_Running:		OnGraphRunning(); break;
		}

	} else
	if (ret == VFW_S_CANT_CUE) {
		// we handle this one as paused
		state_ready = true;
		OnGraphPaused();
	} else
	if (ret == VFW_S_STATE_INTERMEDIATE) {
		// schedule timer
		SetTimer(1, 50, NULL);
		state_ready = false;
	}
}

void CGraphView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case 1:
		{
			KillTimer(1);
			UpdateGraphState();
		}
		break;
	}
}

void CGraphView::OnUpdateRenderMediaFile(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Stopped ? TRUE : FALSE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnUpdatePlayButton(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Running ? FALSE : TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnUpdatePauseButton(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Paused ? FALSE : TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnUpdateStopButton(CCmdUI *ui)
{
	if (state_ready) {
		ui->Enable(graph_state == State_Stopped ? FALSE : TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::OnGraphRunning()
{
	CMainFrame	*frame = (CMainFrame*)GetParentFrame();
	CToolBarCtrl &toolbar = frame->m_wndToolBar.GetToolBarCtrl();

	toolbar.EnableButton(ID_BUTTON_PLAY, FALSE);
	toolbar.EnableButton(ID_BUTTON_PAUSE, TRUE);
	toolbar.EnableButton(ID_BUTTON_STOP, TRUE);
}

void CGraphView::OnGraphStopped()
{
	CMainFrame	*frame = (CMainFrame*)GetParentFrame();
	CToolBarCtrl &toolbar = frame->m_wndToolBar.GetToolBarCtrl();

	toolbar.EnableButton(ID_BUTTON_PLAY, TRUE);
	toolbar.EnableButton(ID_BUTTON_PAUSE, TRUE);
	toolbar.EnableButton(ID_BUTTON_STOP, FALSE);
}

void CGraphView::OnGraphPaused()
{
	CMainFrame	*frame = (CMainFrame*)GetParentFrame();
	CToolBarCtrl &toolbar = frame->m_wndToolBar.GetToolBarCtrl();

	if (toolbar.EnableButton(ID_BUTTON_PLAY, TRUE) == FALSE) {
		Sleep(1);
	}
	toolbar.EnableButton(ID_BUTTON_PAUSE, FALSE);
	toolbar.EnableButton(ID_BUTTON_STOP, TRUE);
}

void CGraphView::OnViewTextInformation()
{
	form_textinfo->ShowWindow(SW_SHOW);
}

void CGraphView::OnDropFiles(HDROP hDropInfo)
{
	// accept dropped files
	TCHAR	temp[4*1024];
	int num_files = DragQueryFile(hDropInfo, (UINT)-1, temp, 4*1024);
	for (int i=0; i<num_files; i++) {
		int ret = DragQueryFile(hDropInfo, i, temp, 4*1024);
		if (ret > 0) {
			temp[ret] = 0;

			// only take one
			CString fn(temp);
			ret = TryOpenFile(fn);
			if (ret == 0) return ;
		}
	}
	MessageBox(_T("Cannot open file"));
}

void CGraphView::OnGraphInsertFileSource()
{
	// directly insert a file source filter
	CComPtr<IBaseFilter>	instance;
	HRESULT					hr;

	hr = CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
	if (FAILED(hr)) {
		// display error message
		MessageBox(_T("Cannot create File Source (Async.)"), _T("Error"), MB_ICONERROR);
		return ;
	} else {
		
		// now check for a few interfaces
		CComPtr<IFileSourceFilter>	fs;
		hr = instance->QueryInterface(IID_IFileSourceFilter, (void**)&fs);
		if (SUCCEEDED(hr)) {
			CFileSrcForm		src_form;
			int ret = src_form.DoModal();
			if (ret == IDOK) {
				hr = fs->Load((LPCOLESTR)src_form.result_file, NULL);
				if (FAILED(hr)) {
					MessageBox(_T("Cannot load specified file"), _T("Error"), MB_ICONERROR);
				}
			} else {
				// cancel the filter
				instance = NULL;
			}
			fs = NULL;
		}

		if (instance) {
			// add the filter to graph
			hr = graph.AddFilter(instance, _T("File Source (Async.)"));
			if (FAILED(hr)) {
				// display error message
			} else {
				graph.SmartPlacement();
				Invalidate();
			}
		}
	}
	instance = NULL;
}

void CGraphView::OnPropertyPageClosed(CPropertyForm *page)
{
	for (int i=0; i<property_pages.GetCount(); i++) {
		if (page == property_pages[i]) {
			if (IsWindow(*page)) {
				page->DestroyWindow();
			}
			delete page;
			property_pages.RemoveAt(i);
			return ;
		}
	}
}

void CGraphView::OnFilterRemoved(GraphStudio::DisplayGraph *sender, GraphStudio::Filter *filter)
{
	// close the property pages associated with this filter
	ClosePropertyPage(filter->filter);
}

void CGraphView::OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title)
{
	// scan through our objects...
	for (int i=0; i<property_pages.GetCount(); i++) {
		CPropertyForm	*page = property_pages[i];
		if (object == page->object) {
			property_pages[i]->ShowWindow(SW_SHOW);
			return ;
		}
	}

	CPropertyForm	*page = new CPropertyForm();
	int ret = page->DisplayPages(object, filter, title, this);
	if (ret < 0) {
		delete page;
		return ;
	}

	// add to the list
	property_pages.Add(page);
}

void CGraphView::ClosePropertyPage(IUnknown *filter)
{
	// scan through our objects...
	for (int i=0; i<property_pages.GetCount(); i++) {
		CPropertyForm	*page = property_pages[i];
		if (filter == page->filter) {

			// kill the page
			if (IsWindow(*page)) page->DestroyWindow();
			delete page;
			property_pages.RemoveAt(i);

			return ;
		}
	}
}

void CGraphView::ClosePropertyPages()
{
	for (int i=0; i<property_pages.GetCount(); i++) {
		CPropertyForm	*page = property_pages[i];
		if (IsWindow(*page)) page->DestroyWindow();
		delete page;
	}
	property_pages.RemoveAll();
}


