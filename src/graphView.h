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

	CFiltersForm			*form_filters;
	CEventsForm				*form_events;
	CTextInfoForm			*form_textinfo;

	// active property pages
	CArray<CPropertyForm*>	property_pages;


	CString			filename;
	bool			can_save;

	// filter state
	bool			state_ready;
	FILTER_STATE	graph_state;

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
	void OnPlayClick();
	void OnStopClick();
	void OnPauseClick();
	void OnNewClick();
	void OnRenderFileClick();
	void OnGraphInsertfilter();

	// keyboard events
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnViewGraphEvents();
	void OnTimer(UINT_PTR nIDEvent);

	void UpdateGraphState();
	void OnGraphRunning();
	void OnGraphStopped();
	void OnGraphPaused();

	void OnUpdatePlayButton(CCmdUI *ui);
	void OnUpdatePauseButton(CCmdUI *ui);
	void OnUpdateStopButton(CCmdUI *ui);
	void OnUpdateRenderMediaFile(CCmdUI *ui);
	void OnDropFiles(HDROP hDropInfo);
	int TryOpenFile(CString fn);

	// property pages
	virtual void OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title);
	virtual void OnFilterRemoved(GraphStudio::DisplayGraph *sender, GraphStudio::Filter *filter);
	virtual void OnPropertyPageClosed(CPropertyForm *page);
	void ClosePropertyPages();
	void ClosePropertyPage(IUnknown *filter);


	afx_msg void OnViewTextInformation();
	afx_msg void OnGraphInsertFileSource();
	afx_msg void OnDestroy();
};

#ifndef _DEBUG  // debug version in graphView.cpp
inline CGraphDoc* CGraphView::GetDocument() const
   { return reinterpret_cast<CGraphDoc*>(m_pDocument); }
#endif

