//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once

namespace GraphStudio
{

	//-------------------------------------------------------------------------
	//
	//	DisplayView class
	//
	//-------------------------------------------------------------------------
	class DisplayView : public CView, public GraphCallback
	{
	protected:
		DECLARE_DYNCREATE(DisplayView)
		DECLARE_MESSAGE_MAP()

	public:

		// color and font settings
		DWORD			color_back;
		
		// filter settings
		DWORD			filter_color;
		DWORD			color_filter_border_light;
		DWORD			color_filter_border_dark;
		DWORD			select_color;
		DWORD			filter_type_colors[4];

		CFont			font_filter;
		CFont			font_pin;

		// graph currently displayed
		DisplayGraph	graph;

		// zoom
		double			zoom;

		// double buffered view
		CBitmap			backbuffer;
		CDC				memDC;
		int				back_width, back_height;

		CPoint			start_drag_point;

		// creating new connection
		CPoint			new_connection_start;
		CPoint			new_connection_end;

		enum {
			DRAG_GROUP = 0,
			DRAG_CONNECTION = 1
		};
		int				drag_mode;

		// helpers for rightclick menu
		Filter			*current_filter;
		Pin				*current_pin;

	public:
		DisplayView();
		~DisplayView();

		BOOL OnEraseBkgnd(CDC* pDC);
		virtual void OnDraw(CDC *pDC);
		void OnSize(UINT nType, int cx, int cy);

		void OnMouseMove(UINT nFlags, CPoint point);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnRButtonDown(UINT nFlags, CPoint point);
		void OnLButtonDblClk(UINT nFlags, CPoint point);

		void OnRenderPin();
		void OnPropertyPage();

		// to be overriden
		virtual void OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title);
		virtual void OnFilterRemoved(DisplayGraph *sender, Filter *filter);
	};

	void MakeFont(CFont &f, CString name, int size, bool bold, bool italic);

};


