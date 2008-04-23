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
//	CGraphView class
//
//-----------------------------------------------------------------------------

class CGraphView : public GraphStudio::DisplayView
{
protected: 
	CGraphView();
	DECLARE_DYNCREATE(CGraphView)
	DECLARE_MESSAGE_MAP()

	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:

	CFiltersForm				*form_filters;
	CEventsForm					*form_events;
	CTextInfoForm				*form_textinfo;
	CFavoritesForm				*form_favorites;
	CProgressForm				*form_progress;

	// active property pages
	CArray<CPropertyForm*>		property_pages;

	// most recently used list
	GraphStudio::MRUList		mru;
	GraphStudio::AutoRestart	auto_restart;

	// enumerated audio & video renderers
	DSUtil::FilterTemplates		audio_renderers;
	DSUtil::FilterTemplates		video_renderers;

	enum {
		TIMER_GRAPH_STATE = 1,
		TIMER_REMOTE_GRAPH_STATE = 2,
		TIMER_AUTO_RESTART = 3
	};

	CString						filename;
	bool						can_save;

	// filter state
	bool						state_ready;
	FILTER_STATE				graph_state;


public:
	virtual ~CGraphView();

	CGraphDoc* GetDocument() const;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Events
	void OnInit();
	void OnFileRenderdvd();
	void OnFileOpenClick();
	void OnFileSaveClick();
	void OnFileSaveAsClick();
	void OnRenderUrlClick();
	void OnPlayClick();
	void OnStopClick();
	void OnPauseClick();
	void OnPlayPauseToggleClick();
	void OnFrameStepClick();
	void OnRefreshFilters();
	void OnNewClick();
	void OnRenderFileClick();
	void OnGraphInsertfilter();
	void OnClearMRUClick();
	void OnMRUClick(UINT nID);
	void OnGraphScreenshot();
	void OnConnectRemote();
	void OnDisconnectRemote();
	void OnDummyEvent(UINT nID) { };

	// menu
	void UpdateMRUMenu();
	void UpdateRenderersMenu();

	void OnAudioRendererClick(UINT nID);
	void OnVideoRendererClick(UINT nID);
	void OnFavoriteFilterClick(UINT nID);
	int InsertFilterFromTemplate(DSUtil::FilterTemplate &filter);
	int InsertFilterFromFavorite(GraphStudio::FavoriteFilter *filter);

	// keyboard events
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnViewGraphEvents();
	void OnTimer(UINT_PTR nIDEvent);

	void UpdateGraphState();
	void OnGraphRunning();
	void OnGraphStopped();
	void OnGraphPaused();
	void OnUpdateTimeLabel(CString text);
	void OnUpdateSeekbar(double pos);

	void OnUpdateConnectRemote(CCmdUI *ui);
	void OnUpdateDisconnectRemote(CCmdUI *ui);
	void OnUpdatePlayButton(CCmdUI *ui);
	void OnUpdatePauseButton(CCmdUI *ui);
	void OnUpdateStopButton(CCmdUI *ui);
	void OnUpdateRenderMediaFile(CCmdUI *ui);
	void OnDropFiles(HDROP hDropInfo);
	int TryOpenFile(CString fn);
	int TryOpenXML(CString fn);

	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnWmCommand(WPARAM wParam, LPARAM lParam);

	// property pages
	virtual void OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title);
	virtual void OnFilterRemoved(GraphStudio::DisplayGraph *sender, GraphStudio::Filter *filter);
	virtual void OnPropertyPageClosed(CPropertyForm *page);
	void ClosePropertyPages();
	void ClosePropertyPage(IUnknown *filter);

	// save/load window position
	void LoadWindowPosition();
	void SaveWindowPosition();

	void OnViewTextInformation();
	void OnGraphInsertFileSource();
	void OnGraphInsertFileSink();

	void OnDestroy();
	
	void OnView50();
	void OnView75();
	void OnView100();
	void OnView150();
	void OnView200();
	void DoZoom(int z);
	void SelectZoomItem(int idc);
	void OnUpdateView50(CCmdUI *pCmdUI);
	void OnUpdateView75(CCmdUI *pCmdUI);
	void OnUpdateView100(CCmdUI *pCmdUI);
	void OnUpdateView150(CCmdUI *pCmdUI);
	void OnUpdateView200(CCmdUI *pCmdUI);
	void OnFileAddmediafile();
	void OnFiltersDouble();
	void OnViewDecreasezoomlevel();
	void OnViewIncreasezoomlevel();
	void OnFiltersManageFavorites();
	void OnAutorestart();
	void OnAutorestartDisabled();
	void OnUpdateAutorestart(CCmdUI *pCmdUI);
	void OnUpdateAutorestartDisabled(CCmdUI *pCmdUI);
	void OnFileOpenfromxml();
	void OnOptionsDisplayFileName();
	void OnUpdateOptionsDisplayFileName(CCmdUI *pCmdUI);
	void OnDirectConnectClick();
	void OnUpdateDirectConnect(CCmdUI *pCmdUI);
	void OnOptionsDirectConnectClick();
	void OnUpdateOptionsDirectConnect(CCmdUI *pCmdUI);

	afx_msg void OnViewProgressview();
};

#ifndef _DEBUG  // debug version in graphView.cpp
inline CGraphDoc* CGraphView::GetDocument() const
   { return reinterpret_cast<CGraphDoc*>(m_pDocument); }
#endif

