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
//	CMainFrame class
//
//-----------------------------------------------------------------------------

class CMainFrame : public CFrameWnd
{	
protected: 
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	DECLARE_MESSAGE_MAP()

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

public:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CToolBar		m_wndPlaybackBar;
	CSeekingBar		m_wndSeekingBar;
	CReBar			m_wndReBar;

	// Overrides
public:
	virtual ~CMainFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};


