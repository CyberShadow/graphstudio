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
	ON_COMMAND(ID_BUTTON_STEP, &CGraphView::OnFrameStepClick)
	ON_COMMAND(ID_BUTTON_PLAYPAUSE, &CGraphView::OnPlayPauseToggleClick)
	ON_COMMAND(ID_BUTTON_DIRECT, &CGraphView::OnDirectConnectClick)
	ON_COMMAND(ID_BUTTON_REFRESH, &CGraphView::OnRefreshFilters)
	ON_COMMAND(ID_OPTIONS_DIRECT, &CGraphView::OnOptionsDirectConnectClick)
	ON_COMMAND(ID_FILE_NEW, &CGraphView::OnNewClick)
	ON_COMMAND(ID_FILE_OPEN, &CGraphView::OnFileOpenClick)
	ON_COMMAND(ID_FILE_SAVE, &CGraphView::OnFileSaveClick)
	ON_COMMAND(ID_FILE_SAVE_AS, &CGraphView::OnFileSaveAsClick)
	ON_COMMAND(ID_FILE_RENDERFILE, &CGraphView::OnRenderFileClick)
	ON_COMMAND(ID_FILE_RENDERURL, &CGraphView::OnRenderUrlClick)
	ON_COMMAND(ID_FILE_CONNECTTOREMOTEGRAPH, &CGraphView::OnConnectRemote)
	ON_COMMAND(ID_FILE_DISCONNECTFROMREMOTEGRAPH, &CGraphView::OnDisconnectRemote)
	ON_COMMAND(ID_GRAPH_INSERTFILTER, &CGraphView::OnGraphInsertfilter)
	ON_COMMAND(ID_VIEW_GRAPHEVENTS, &CGraphView::OnViewGraphEvents)
	ON_COMMAND(ID_LIST_MRU_CLEAR, &CGraphView::OnClearMRUClick)
	ON_COMMAND(ID_GRAPH_MAKEGRAPHSCREENSHOT, &CGraphView::OnGraphScreenshot)
	ON_COMMAND(ID_GRAPH_USECLOCK, &CGraphView::OnUseClock)
	ON_COMMAND_RANGE(ID_LIST_MRU_FILE0, ID_LIST_MRU_FILE0+10, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_AUDIO_RENDERER0, ID_AUDIO_RENDERER0+100, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_VIDEO_RENDERER0, ID_VIDEO_RENDERER0+100, &CGraphView::OnDummyEvent)
	ON_COMMAND_RANGE(ID_FAVORITE_FILTER, ID_FAVORITE_FILTER+500, &CGraphView::OnDummyEvent)

	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_DROPFILES()

	ON_UPDATE_COMMAND_UI(ID_GRAPH_USECLOCK, &CGraphView::OnUpdateUseClock)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_DIRECT, &CGraphView::OnUpdateDirectConnect)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_DIRECT, &CGraphView::OnUpdateOptionsDirectConnect)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PLAY, &CGraphView::OnUpdatePlayButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_PAUSE, &CGraphView::OnUpdatePauseButton)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_STOP, &CGraphView::OnUpdateStopButton)
	ON_UPDATE_COMMAND_UI(ID_FILE_RENDERFILE, &CGraphView::OnUpdateRenderMediaFile)
	ON_UPDATE_COMMAND_UI(ID_FILE_CONNECTTOREMOTEGRAPH, &CGraphView::OnUpdateConnectRemote)
	ON_UPDATE_COMMAND_UI(ID_FILE_DISCONNECTFROMREMOTEGRAPH, &CGraphView::OnUpdateDisconnectRemote)
	
	ON_COMMAND(ID_VIEW_TEXTINFORMATION, &CGraphView::OnViewTextInformation)
	ON_COMMAND(ID_GRAPH_INSERTFILESOURCE, &CGraphView::OnGraphInsertFileSource)
	ON_COMMAND(ID_GRAPH_INSERTFILEWRITER, &CGraphView::OnGraphInsertFileSink)
	ON_COMMAND(ID_VIEW_50, &CGraphView::OnView50)
	ON_COMMAND(ID_VIEW_75, &CGraphView::OnView75)
	ON_COMMAND(ID_VIEW_100, &CGraphView::OnView100)
	ON_COMMAND(ID_VIEW_150, &CGraphView::OnView150)
	ON_COMMAND(ID_VIEW_200, &CGraphView::OnView200)
	ON_UPDATE_COMMAND_UI(ID_VIEW_50, &CGraphView::OnUpdateView50)
	ON_UPDATE_COMMAND_UI(ID_VIEW_75, &CGraphView::OnUpdateView75)
	ON_UPDATE_COMMAND_UI(ID_VIEW_100, &CGraphView::OnUpdateView100)
	ON_UPDATE_COMMAND_UI(ID_VIEW_150, &CGraphView::OnUpdateView150)
	ON_UPDATE_COMMAND_UI(ID_VIEW_200, &CGraphView::OnUpdateView200)
	ON_COMMAND(ID_FILE_ADDMEDIAFILE, &CGraphView::OnFileAddmediafile)
	ON_COMMAND(ID_FILTERS_DOUBLESELECTEDFILTERS, &CGraphView::OnFiltersDouble)
	ON_COMMAND(ID_VIEW_DECREASEZOOMLEVEL, &CGraphView::OnViewDecreasezoomlevel)
	ON_COMMAND(ID_VIEW_INCREASEZOOMLEVEL, &CGraphView::OnViewIncreasezoomlevel)
	ON_COMMAND(ID_FILTERS_MANAGEFAVORITES, &CGraphView::OnFiltersManageFavorites)
	ON_COMMAND(ID_AUTORESTART, &CGraphView::OnAutorestart)
	ON_COMMAND(ID_AUTORESTART_DISABLED, &CGraphView::OnAutorestartDisabled)
	ON_UPDATE_COMMAND_UI(ID_AUTORESTART, &CGraphView::OnUpdateAutorestart)
	ON_UPDATE_COMMAND_UI(ID_AUTORESTART_DISABLED, &CGraphView::OnUpdateAutorestartDisabled)
	ON_COMMAND(ID_FILE_OPENFROMXML, &CGraphView::OnFileOpenfromxml)
	ON_COMMAND(ID_OPTIONS_DISPLAYASFILENAME, &CGraphView::OnOptionsDisplayFileName)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_DISPLAYASFILENAME, &CGraphView::OnUpdateOptionsDisplayFileName)
	ON_COMMAND(ID_VIEW_PROGRESSVIEW, &CGraphView::OnViewProgressview)
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
	form_favorites = NULL;
	form_progress = NULL;
	form_volume = NULL;
	filename = _T("");
	can_save = false;
}

CGraphView::~CGraphView()
{
	if (form_volume) { form_volume->DestroyWindow(); delete form_volume; }
	if (form_progress) { form_progress->DestroyWindow(); delete form_progress; }
	if (form_filters) { form_filters->DestroyWindow(); delete form_filters; }
	if (form_events) { form_events->DestroyWindow(); delete form_events; }
	if (form_textinfo) { form_textinfo->DestroyWindow(); delete form_textinfo; }
	if (form_favorites) { form_favorites->DestroyWindow(); delete form_favorites; }
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
	SaveWindowPosition();

	ClosePropertyPages();
	graph.MakeNew();

	__super::OnDestroy();
}

void CGraphView::LoadWindowPosition()
{
	int	x,y,cx,cy;

	x = AfxGetApp()->GetProfileInt(_T("Settings"), _T("left"), 100);
	y = AfxGetApp()->GetProfileInt(_T("Settings"), _T("top"), 100);
	cx = AfxGetApp()->GetProfileInt(_T("Settings"), _T("width"), 640);
	cy = AfxGetApp()->GetProfileInt(_T("Settings"), _T("height"), 320);

	GetParentFrame()->SetWindowPos(NULL, x, y, cx, cy, SWP_SHOWWINDOW);
}

void CGraphView::SaveWindowPosition()
{
	CRect	rc;
	GetParentFrame()->GetWindowRect(&rc);

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("left"), rc.left);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("top"), rc.top);
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("width"), rc.Width());
	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("height"), rc.Height());
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

	// set the parent view
	CMainFrame	*frm = (CMainFrame*)GetParent();
	frm->view = this;

	LoadWindowPosition();


	// initialize our event logger
	form_events = new CEventsForm(NULL);
	form_events->view = this;
	form_events->DoCreateDialog();

	form_textinfo = new CTextInfoForm(NULL);
	form_textinfo->Create(IDD_DIALOG_TEXTVIEW);
	form_textinfo->view = this;
	form_textinfo->OnInitialize();

	form_progress = new CProgressForm(NULL);
	form_progress->view = this;
	form_progress->Create(IDD_DIALOG_PROGRESS);

	graph.wndEvents = *form_events;
	graph.MakeNew();

	// set the seeking bar data
	CMainFrame *frame = (CMainFrame*)GetParentFrame();
	frame->m_wndSeekingBar.SetGraphView(this);

	mru.Load();

	int zoom_level = AfxGetApp()->GetProfileInt(_T("Settings"), _T("Zoom"), 100);
	switch (zoom_level) {
	case 200:	OnView200(); break;
	case 150:	OnView150(); break;
	case 75:	OnView75(); break;
	case 50:	OnView50(); break;
	default:	OnView100(); break;
	}

	UpdateGraphState();
	UpdateMRUMenu();

	// auto restart
	auto_restart.hwndTarget = *this;
	auto_restart.Enable(false);

	// load favorites
	GraphStudio::Favorites	*favorites = GraphStudio::Favorites::GetInstance();

	form_favorites = new CFavoritesForm();
	form_favorites->view = this;
	favorites->Load();
	form_favorites->DoCreateDialog();


	// trick to refresh menu in a while...
	SetTimer(1001, 20, NULL);
}

void CGraphView::OnFileRenderdvd()
{
}

LRESULT CGraphView::OnWmCommand(WPARAM wParam, LPARAM lParam)
{
	/*
		For some strange reason MFC kept blocking several IDs
		from the command range so I had to do a little bypass.
	*/

	int		id = LOWORD(wParam);

	if (id >= ID_AUDIO_RENDERER0 && id < ID_AUDIO_RENDERER0 + 100) {
		OnAudioRendererClick(id);
	} else
	if (id >= ID_VIDEO_RENDERER0 && id < ID_VIDEO_RENDERER0 + 100) {
		OnVideoRendererClick(id);
	} else
	if (id >= ID_FAVORITE_FILTER && id < ID_FAVORITE_FILTER + 500) {
		OnFavoriteFilterClick(id);
	} else
	if (id >= ID_LIST_MRU_FILE0 && id < ID_LIST_MRU_FILE0 + 10) {
		OnMRUClick(id);
	}	

	return 0;
}

BOOL CGraphView::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (message == WM_AUTORESTART) {

		// restart the graph
		OnStopClick();
		OnPlayClick();

		// schedule next restart
		auto_restart.Schedule();

	}
	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CGraphView::UpdateRenderersMenu()
{
	int		i;

	audio_renderers.filters.RemoveAll();
	video_renderers.filters.RemoveAll();

	audio_renderers.EnumerateAudioRenderers();
	video_renderers.EnumerateVideoRenderers();

	CMenu	*mainmenu  = GetParentFrame()->GetMenu();
	CMenu	*graphmenu = mainmenu->GetSubMenu(2);
	CMenu	audio_menu, video_menu;

	// fill in audio renderers
	audio_menu.CreatePopupMenu();
	for (i=0; i<audio_renderers.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = audio_renderers.filters[i];
		audio_menu.InsertMenu(i, MF_STRING, ID_AUDIO_RENDERER0 + i, filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTAUDIORENDERER,
						  MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)audio_menu.m_hMenu,
						  _T("Insert Audio Renderer"));

	audio_menu.Detach();

	// fill in video renderers
	video_menu.CreatePopupMenu();
	for (i=0; i<video_renderers.filters.GetCount(); i++) {
		DSUtil::FilterTemplate	&filter = video_renderers.filters[i];
		video_menu.InsertMenu(i, MF_STRING, ID_VIDEO_RENDERER0 + i, filter.name);
	}

	graphmenu->ModifyMenu(ID_GRAPH_INSERTVIDEORENDERER,
						  MF_BYCOMMAND | MF_POPUP | MF_STRING, 
						  (UINT_PTR)video_menu.m_hMenu,
						  _T("Insert Video Renderer"));

	video_menu.Detach();
}

void CGraphView::UpdateMRUMenu()
{
	CMenu	*mainmenu = GetParentFrame()->GetMenu();
	if (!mainmenu) return ;
	CMenu	*filemenu = mainmenu->GetSubMenu(0);
	mru.GenerateMenu(filemenu);
}

void CGraphView::OnFrameStepClick()
{
	// do a frame step
	graph.DoFrameStep();

	// update the graph state timers
	UpdateGraphState();
}

void CGraphView::OnPlayClick()
{
	if (SUCCEEDED(graph.DoPlay())) {
		if (!graph.is_remote) {
			SetTimer(CGraphView::TIMER_AUTO_RESTART, 1000, NULL);
		}
	}
	UpdateGraphState();
}

void CGraphView::OnStopClick()
{
	if (SUCCEEDED(graph.DoStop())) {
		KillTimer(CGraphView::TIMER_AUTO_RESTART);
	}
	UpdateGraphState();
}

void CGraphView::OnPauseClick()
{
	graph.DoPause();
	UpdateGraphState();
}

void CGraphView::OnPlayPauseToggleClick()
{
	FILTER_STATE	state;
	int ret = graph.GetState(state, 20);
	if (ret < 0) {
		return ;
	}

	// toggle state
	if (state == State_Stopped || state == State_Paused) {
		OnPlayClick();
	} else {
		OnPauseClick();
	}
}

void CGraphView::OnNewClick()
{
	KillTimer(CGraphView::TIMER_REMOTE_GRAPH_STATE);
	KillTimer(CGraphView::TIMER_AUTO_RESTART);

	if (!graph.is_remote) {
		OnStopClick();
	}

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

		// updatujeme MRU list
		mru.NotifyEntry(filename);
		UpdateMRUMenu();
	} else {
		OnFileSaveAsClick();
	}
}

void CGraphView::OnFileSaveAsClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter = _T("GraphEdit Files|*.grf|");

	CFileDialog dlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_PATHMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {

		CPath	path(filename);
		if (path.GetExtension() == _T("")) {
			path.AddExtension(_T(".grf"));

			filename = CString(path);
		}

		ret = graph.SaveGRF(filename);	
		if (ret < 0) {
			MessageBox(_T("Cannot save file"));
			return ;
		}

		// updatujeme MRU list
		mru.NotifyEntry(filename);
		UpdateMRUMenu();

		this->filename = filename;
		can_save = true;

		
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

	} else 
	if (ext == _T(".xml")) {
		OnNewClick();
		ret = graph.LoadXML(fn);
		if (ret == 0) {
			filename = fn;
			can_save = false;
		}

	} else {
		OnNewClick();
		ret = graph.RenderFile(fn);
	}

	if (ret < 0) return ret;

	// updatujeme MRU list
	mru.NotifyEntry(fn);
	UpdateMRUMenu();

	CGraphDoc *doc = GetDocument();
	int pos = path.FindFileName();
	CString	short_fn = fn;
	short_fn.Delete(0, pos);
	doc->SetTitle(short_fn);

	UpdateGraphState();
	graph.SetClock(true, NULL);
	graph.RefreshFilters();
	graph.SmartPlacement();
	Invalidate();
	return 0;
}

void CGraphView::OnFileOpenfromxml()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter =  _T("");
	filter += _T("GraphStudio XML Files (xml)|*.xml|");
	filter += _T("All Files|*.*|");

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

void CGraphView::OnFileOpenClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter =  _T("");
	filter += _T("GraphEdit Files (grf)|*.grf|");
	filter += _T("GraphStudio XML Files (xml)|*.xml|");
	filter += _T("Video Files (avi,mp4,mpg,mpeg,ts,mkv,ogg,ogm,pva,evo,flv,mov,hdmov,ifo,vob,rm,rmvb,wmv,asf)|*.avi;*.mp4;*.mpg;*.mpeg;*.ts;*.mkv;*.ogg;*.ogm;*.pva;*.evo;*.flv;*.mov;*.hdmov;*.ifo;*.vob;*.rm;*.rmvb;*.wmv;*.asf|");
	filter += _T("Audio Files (aac,ac3,mp3,wma,mka,ogg,mpc,flac,ape,wav,ra,wv,m4a,tta,dts,spx,mp2,ofr,ofs,mpa)|*.aac;*.ac3;*.mp3;*.wma;*.mka;*.ogg;*.mpc;*.flac;*.ape;*.wav;*.ra;*.wv;*.m4a;*.tta;*.dts;*.spx;*.mp2;*.ofr;*.ofs;*.mpa|");
	filter += _T("All Files|*.*|");

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

void CGraphView::OnFileAddmediafile()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter =  _T("");
	filter += _T("Video Files (avi,mp4,mpg,mpeg,ts,mkv,ogg,ogm,pva,evo,flv,mov,hdmov,ifo,vob,rm,rmvb,wmv,asf)|*.avi;*.mp4;*.mpg;*.mpeg;*.ts;*.mkv;*.ogg;*.ogm;*.pva;*.evo;*.flv;*.mov;*.hdmov;*.ifo;*.vob;*.rm;*.rmvb;*.wmv;*.asf|");
	filter += _T("Audio Files (aac,ac3,mp3,wma,mka,ogg,mpc,flac,ape,wav,ra,wv,m4a,tta,dts,spx,mp2,ofr,ofs,mpa)|*.aac;*.ac3;*.mp3;*.wma;*.mka;*.ogg;*.mpc;*.flac;*.ape;*.wav;*.ra;*.wv;*.m4a;*.tta;*.dts;*.spx;*.mp2;*.ofr;*.ofs;*.mpa|");
	filter += _T("All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {

		int ret = graph.RenderFile(filename);
		if (ret < 0) {
			MessageBox(_T("Cannot render file"));
		}

		// updatujeme MRU list
		mru.NotifyEntry(filename);
		UpdateMRUMenu();

		graph.SetClock(true, NULL);
		graph.RefreshFilters();
		graph.SmartPlacement();
		graph.Dirty();
		Invalidate();
	}
}

void CGraphView::OnRenderUrlClick()
{
	CRenderUrlForm		dlg;
	int ret = dlg.DoModal();
	if (ret == IDOK) {

		OnNewClick();
		int ret = graph.RenderFile(dlg.result_file);
		if (ret < 0) {
			MessageBox(_T("Cannot render URL"));
		}

		graph.SetClock(true, NULL);
		graph.RefreshFilters();
		graph.SmartPlacement();
		graph.Dirty();
		Invalidate();
	}
}

void CGraphView::OnRenderFileClick()
{
	// nabrowsujeme subor
	CString		filter;
	CString		filename;

	filter =  _T("");
	filter += _T("Video Files (avi,mp4,mpg,mpeg,ts,mkv,ogg,ogm,pva,evo,flv,mov,hdmov,ifo,vob,rm,rmvb,wmv,asf)|*.avi;*.mp4;*.mpg;*.mpeg;*.ts;*.mkv;*.ogg;*.ogm;*.pva;*.evo;*.flv;*.mov;*.hdmov;*.ifo;*.vob;*.rm;*.rmvb;*.wmv;*.asf|");
	filter += _T("Audio Files (aac,ac3,mp3,wma,mka,ogg,mpc,flac,ape,wav,ra,wv,m4a,tta,dts,spx,mp2,ofr,ofs,mpa)|*.aac;*.ac3;*.mp3;*.wma;*.mka;*.ogg;*.mpc;*.flac;*.ape;*.wav;*.ra;*.wv;*.m4a;*.tta;*.dts;*.spx;*.mp2;*.ofr;*.ofs;*.mpa|");
	filter += _T("All Files|*.*|");

	CFileDialog dlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT|OFN_ENABLESIZING|OFN_FILEMUSTEXIST,filter);
    int ret = dlg.DoModal();

	filename = dlg.GetPathName();
	if (ret == IDOK) {
		OnNewClick();
		int ret = graph.RenderFile(filename);
		if (ret < 0) {
			MessageBox(_T("Cannot render file"));
		}

		// updatujeme MRU list
		mru.NotifyEntry(filename);
		UpdateMRUMenu();

		graph.SetClock(true, NULL);
		graph.RefreshFilters();
		graph.SmartPlacement();
		graph.Dirty();
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

				if (state != State_Stopped) {
					// play sound to warn the user
					MessageBeep(MB_ICONASTERISK);
					return ;
				}

				// avoid some unnecessary crashes
				overlay_filter = NULL;

				// delete selected objects
				graph.DeleteSelected();
				Invalidate();
			}
			break;
		}
	}
}

void CGraphView::OnRefreshFilters()
{
	graph.RefreshFilters();
	graph.SmartPlacement();
	Invalidate();
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
		SetTimer(CGraphView::TIMER_GRAPH_STATE, 50, NULL);
		state_ready = false;
	}
}

void CGraphView::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent) {
	case CGraphView::TIMER_GRAPH_STATE:
		{
			KillTimer(CGraphView::TIMER_GRAPH_STATE);
			UpdateGraphState();
		}
		break;
	case CGraphView::TIMER_REMOTE_GRAPH_STATE:
		{
			UpdateGraphState();
		}
		break;
	case CGraphView::TIMER_AUTO_RESTART:
		{
			auto_restart.OnTimer();
		}
		break;
	case 1001:
		{
			KillTimer(1001);
			UpdateRenderersMenu();
		}
		break;
	}
}

void CGraphView::OnUseClock()
{
	if (graph.uses_clock) {
		graph.SetClock(false, NULL);
	} else {
		graph.SetClock(true, NULL);
	}

	graph.Dirty();
	Invalidate();
}

void CGraphView::OnUpdateUseClock(CCmdUI *ui)
{
	ui->SetCheck(graph.uses_clock);
}

void CGraphView::OnUpdateTimeLabel(CString text)
{
	if (form_progress) {
		form_progress->UpdateTimeLabel(text);
	}
}

void CGraphView::OnUpdateSeekbar(double pos)
{
	if (form_progress) {
		form_progress->UpdateProgress(pos);
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

	// send the event to the progress form
	if (form_progress) {
		form_progress->OnGraphStopped();
	}
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
		
		int ret = ConfigureInsertedFilter(instance);
		if (ret < 0) {
			instance = NULL;
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

void CGraphView::OnGraphInsertFileSink()
{
	// directly insert a file source filter
	CComPtr<IBaseFilter>	instance;
	HRESULT					hr;

	hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
	if (FAILED(hr)) {
		// display error message
		MessageBox(_T("Cannot create File Sink"), _T("Error"), MB_ICONERROR);
		return ;
	} else {
		
		int ret = ConfigureInsertedFilter(instance);
		if (ret < 0) {
			instance = NULL;
		}

		if (instance) {
			// add the filter to graph
			hr = graph.AddFilter(instance, _T("File Sink"));
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

void CGraphView::OnClearMRUClick()
{
	mru.Clear();
	UpdateMRUMenu();
}

void CGraphView::OnMRUClick(UINT nID)
{
	int idx = nID - ID_LIST_MRU_FILE0;

	// let's try to open this one
	if (idx < 0 || idx >= mru.list.GetCount()) return ;

	CString	fn = mru.list[idx];
	TryOpenFile(fn);
}

void CGraphView::OnGraphScreenshot()
{
	MakeScreenshot();
}

void CGraphView::OnConnectRemote()
{
	CRemoteGraphForm	remote_form;
	int ret = remote_form.DoModal();
	if (ret == IDOK) {
		if (remote_form.sel_graph) {

			// get a graph object
			CComPtr<IRunningObjectTable>	rot;
			CComPtr<IUnknown>				unk;
			CComPtr<IFilterGraph>			fg;
			HRESULT							hr;

			hr = GetRunningObjectTable(0, &rot);
			ASSERT(SUCCEEDED(hr));

			hr = rot->GetObject(remote_form.sel_graph, &unk);
			if (SUCCEEDED(hr)) {

				hr = unk->QueryInterface(IID_IFilterGraph, (void**)&fg);
				if (SUCCEEDED(hr)) {

					ret = graph.ConnectToRemote(fg);
					if (ret == 0) {
						SetTimer(CGraphView::TIMER_REMOTE_GRAPH_STATE, 200, NULL);
					}

					// get all filters
					graph.RefreshFilters();
					graph.SmartPlacement();
					Invalidate();
				}
				fg = NULL;
			}
			unk = NULL;

			rot = NULL;
		}
	}
}

void CGraphView::OnDisconnectRemote()
{
	OnNewClick();
}

void CGraphView::OnUpdateConnectRemote(CCmdUI *ui)
{
	ui->Enable(TRUE);
}

void CGraphView::OnUpdateDisconnectRemote(CCmdUI *ui)
{
	if (graph.is_remote) {
		ui->Enable(TRUE);
	} else {
		ui->Enable(FALSE);
	}
}

void CGraphView::DoZoom(int z)
{	
	render_params.Zoom(z);
	graph.SmartPlacement();
	graph.Dirty();
	Invalidate();

	AfxGetApp()->WriteProfileInt(_T("Settings"), _T("Zoom"), z);
}

void CGraphView::OnView50()		{ DoZoom(50);  }
void CGraphView::OnView75()		{ DoZoom(75);  }
void CGraphView::OnView100()	{ DoZoom(100); }
void CGraphView::OnView150()	{ DoZoom(150); }
void CGraphView::OnView200()	{ DoZoom(200); }

void CGraphView::OnUpdateView50(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 50);
}

void CGraphView::OnUpdateView75(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 75);
}

void CGraphView::OnUpdateView100(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 100);
}

void CGraphView::OnUpdateView150(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 150);
}

void CGraphView::OnUpdateView200(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.zoom == 200);
}


void CGraphView::OnViewDecreasezoomlevel()
{
	int	zl = 100;
	switch (render_params.zoom) {
	case 50:	zl = 50; break;
	case 75:	zl = 50; break;
	case 100:	zl = 75; break;
	case 150:	zl = 100; break;
	case 200:	zl = 150; break;
	}
	DoZoom(zl);
}

void CGraphView::OnViewIncreasezoomlevel()
{
	int	zl = 100;
	switch (render_params.zoom) {
	case 50:	zl = 75; break;
	case 75:	zl = 100; break;
	case 100:	zl = 150; break;
	case 150:	zl = 200; break;
	case 200:	zl = 200; break;
	}
	DoZoom(zl);
}


void CGraphView::OnAudioRendererClick(UINT nID)
{
	int	n = nID - ID_AUDIO_RENDERER0;
	if (n < 0 || n >= audio_renderers.filters.GetCount()) return ;

	InsertFilterFromTemplate(audio_renderers.filters[n]);
}

void CGraphView::OnVideoRendererClick(UINT nID)
{
	int	n = nID - ID_VIDEO_RENDERER0;
	if (n < 0 || n >= video_renderers.filters.GetCount()) return ;

	InsertFilterFromTemplate(video_renderers.filters[n]);
}

void CGraphView::OnFavoriteFilterClick(UINT nID)
{
	CMenu	*mainmenu = GetParentFrame()->GetMenu();
	CMenu	*filtersmenu = mainmenu->GetSubMenu(3);

	MENUITEMINFO	info;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_DATA;
	filtersmenu->GetMenuItemInfo(nID, &info);

	// let's insert the filter
	GraphStudio::FavoriteFilter	*filter = (GraphStudio::FavoriteFilter *)info.dwItemData;
	InsertFilterFromFavorite(filter);
}

int CGraphView::InsertFilterFromFavorite(GraphStudio::FavoriteFilter *filter)
{
	CComPtr<IMoniker>		moniker;
	CComPtr<IBaseFilter>	instance;
	CComPtr<IBindCtx>		bind;

	HRESULT					hr;
	ULONG					eaten = 0;

	hr = CreateBindCtx(0, &bind);
	if (FAILED(hr)) return -1;

	hr = MkParseDisplayName(bind, filter->moniker_name, &eaten, &moniker);
	if (hr != NOERROR) {
		bind = NULL;
		return -1;
	}

	hr = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&instance);
	if (SUCCEEDED(hr)) {

		// now check for a few interfaces
		int ret = ConfigureInsertedFilter(instance);
		if (ret < 0) {
			instance = NULL;
		}

		if (instance) {
			// add the filter to graph
			hr = graph.AddFilter(instance, filter->name);
			if (FAILED(hr)) {
				// display error message
			} else {
				graph.SmartPlacement();
				Invalidate();
			}
		}
	}

	bind = NULL;
	instance = NULL;
	moniker = NULL;
	return 0;
}

int CGraphView::InsertFilterFromTemplate(DSUtil::FilterTemplate &filter)
{
	// now create an instance of this filter
	CComPtr<IBaseFilter>	instance;
	HRESULT					hr;

	hr = filter.CreateInstance(&instance);
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
			hr = graph.AddFilter(instance, filter.name);
			if (FAILED(hr)) {
				// display error message
			} else {
				graph.SmartPlacement();
				Invalidate();
			}
		}
	}
	instance = NULL;
	return 0;
}

void CGraphView::OnFiltersDouble()
{
	graph.DoubleSelected();
	graph.SmartPlacement();
	Invalidate();
}

void CGraphView::OnFiltersManageFavorites()
{
	form_favorites->ShowWindow(SW_SHOW);
}

void CGraphView::OnAutorestart()
{
	auto_restart.Enable(true);
}

void CGraphView::OnAutorestartDisabled()
{
	auto_restart.Enable(false);
}

void CGraphView::OnUpdateAutorestart(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(auto_restart.enabled);
}

void CGraphView::OnUpdateAutorestartDisabled(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(!auto_restart.enabled);
}

void CGraphView::OnOptionsDisplayFileName()
{
	render_params.display_file_name = !render_params.display_file_name;
	graph.RefreshFilters();
	graph.SmartPlacement();
	graph.Dirty();
	Invalidate();
}

void CGraphView::OnUpdateOptionsDisplayFileName(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.display_file_name ? true : false);
}

void CGraphView::OnDirectConnectClick()
{
	// toggle
	render_params.direct_connect = !render_params.direct_connect;
}

void CGraphView::OnUpdateDirectConnect(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(render_params.direct_connect);
}

void CGraphView::OnOptionsDirectConnectClick()
{
	// toggle
	render_params.direct_connect = !render_params.direct_connect;
}

void CGraphView::OnUpdateOptionsDirectConnect(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(render_params.direct_connect);
}

void CGraphView::OnViewProgressview()
{
	form_progress->UpdateCaption(graph.graph_name);
	form_progress->ShowWindow(SW_SHOW);
	AfxGetMainWnd()->ShowWindow(SW_HIDE);
}

void CGraphView::OnOverlayIconClick(GraphStudio::OverlayIcon *icon, CPoint point)
{
	if (!icon) return ;

	switch (icon->id) {
	case GraphStudio::OverlayIcon::ICON_VOLUME:
		{
			// let's bring up some slider bar for the volume
			if (!form_volume) {
				form_volume = new CVolumeBarForm();
				form_volume->DoCreateDialog();
			}
			
			form_volume->DoHide();
			form_volume->DisplayVolume(icon->filter->filter);
		}
		break;
	case GraphStudio::OverlayIcon::ICON_CLOCK:
		{
			// set this new clock
			if (icon->filter && icon->filter->clock) {
				graph.SetClock(false, icon->filter->clock);
				graph.Dirty();
				Invalidate();
			}
		}
		break;
	}
}
