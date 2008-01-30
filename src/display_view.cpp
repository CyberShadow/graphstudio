//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"

namespace GraphStudio
{

	void MakeFont(CFont &f, CString name, int size, bool bold, bool italic)
	{
		HDC dc = CreateCompatibleDC(NULL);
		int nHeight    = -MulDiv(size, (int)(GetDeviceCaps(dc, LOGPIXELSY)), 72 );
		DeleteDC(dc);

		DWORD dwBold   = (bold ? FW_BOLD : 0);
		DWORD dwItalic = (italic ? TRUE : FALSE);

		f.CreateFont(nHeight, 0, 0, 0, dwBold, dwItalic, FALSE, FALSE, DEFAULT_CHARSET,
					  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, VARIABLE_PITCH, name);
	}

	//-------------------------------------------------------------------------
	//
	//	DisplayView class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(DisplayView, CView)

	BEGIN_MESSAGE_MAP(DisplayView, CView)
		// Standard printing commands
		ON_WM_ERASEBKGND()
		ON_WM_SIZE()
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_RBUTTONDOWN()
		ON_WM_LBUTTONUP()

		ON_COMMAND(ID_PIN_RENDER, &DisplayView::OnRenderPin)
		ON_COMMAND(ID_PROPERTYPAGE, &DisplayView::OnPropertyPage)
	END_MESSAGE_MAP()

	DisplayView::DisplayView()
	{
		color_back = RGB(192, 192, 192);		// default background color
		select_color = RGB(0,0,255);

		back_width = 0;
		back_height = 0;

		// filter colors
		color_filter_border_light = RGB(255, 255, 255);
		color_filter_border_dark = RGB(128, 128, 128);

		filter_type_colors[Filter::FILTER_UNKNOWN] = RGB(192,192,192);
		filter_type_colors[Filter::FILTER_STANDARD] = RGB(192,192,255);
		filter_type_colors[Filter::FILTER_WDM] = RGB(255,128,0);
		filter_type_colors[Filter::FILTER_DMO] = RGB(0,192,64);

		MakeFont(font_filter, _T("Arial"), 10, false, false); 
		MakeFont(font_pin, _T("Arial"), 7, false, false);

		// 100% zoom
		zoom = 1.0;

		// nastavime DC
		graph.callback = this;
		graph.dc = &memDC;
		graph.filter_font = &font_filter;
	}

	DisplayView::~DisplayView()
	{
		if (memDC.GetSafeHdc()) {
			memDC.DeleteDC();
			backbuffer.DeleteObject();
		}
	}

	BOOL DisplayView::OnEraseBkgnd(CDC* pDC)
	{
		return TRUE;
	}

	void DisplayView::OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		// find out if there is any filter being selected
		current_filter = graph.FindFilterByPos(point);
		if (!current_filter) return ;

		// check for a pin - will have different menu
		current_pin = current_filter->FindPinByPos(point, false);
		OnPropertyPage();
	}

	void DisplayView::OnRButtonDown(UINT nFlags, CPoint point)
	{
		CMenu	menu;
		// find out if there is any filter being selected
		current_filter = graph.FindFilterByPos(point);
		if (!current_filter) return ;

		if (!menu.CreatePopupMenu()) return ;

		FILTER_STATE	state = State_Running;
		if (graph.GetState(state, 0) != 0) {
			state = State_Running;
		}

		// check for a pin - will have different menu
		current_pin = current_filter->FindPinByPos(point, false);
		if (current_pin) {

			// make rendering inactive for connected pins
			UINT	flags = 0;
			if (current_pin->connected) flags |= MF_GRAYED;
			if (current_pin->dir != PINDIR_OUTPUT) flags |= MF_GRAYED;
			if (state != State_Stopped) flags |= MF_GRAYED;
			
			menu.InsertMenu(0, MF_STRING | flags, ID_PIN_RENDER, _T("Render pin"));
			menu.InsertMenu(0, MF_STRING, ID_PROPERTYPAGE, _T("Properties"));

		} else {
			menu.InsertMenu(0, MF_STRING, ID_PROPERTYPAGE, _T("Properties"));
		}

		CPoint	pt;
		GetCursorPos(&pt);

		// display menu
		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}

	void DisplayView::OnLButtonDown(UINT nFlags, CPoint point)
	{
		SetCapture();
		start_drag_point = point;

		Filter	*current = graph.FindFilterByPos(point);
		if (!current) {
			// deselect all filters
			bool need_invalidate = false;
			for (int i=0; i<graph.filters.GetCount(); i++) {
				if (graph.filters[i]->selected) {
					graph.filters[i]->Select(false);
				}
				graph.filters[i]->SelectConnection(nFlags, point);
				need_invalidate = true;
			}
			if (need_invalidate) Invalidate();
			return ;
		}

		// check if we hit a pin
		Pin *hitpin = current->FindPinByPos(point);
		if (hitpin) {
			// deselect all filters
			for (int i=0; i<graph.filters.GetCount(); i++) {
				graph.filters[i]->Select(false);
			}

			// remember the start point
			hitpin->GetCenterPoint(&new_connection_start);
			new_connection_end = new_connection_start;
			drag_mode = DisplayView::DRAG_CONNECTION;

		} else {
			if (current->selected) {
				if (nFlags & MK_SHIFT) {
					current->Select(false);
					Invalidate();
				} else {
					// nothing here...
				}
			} else {
				if (nFlags & MK_SHIFT) {
					current->Select(true);
					Invalidate();
				} else {
					// deselect all filters but this
					for (int i=0; i<graph.filters.GetCount(); i++) {
						graph.filters[i]->Select(false);
					}
					current->Select(true);
					Invalidate();
				}
			}

			drag_mode = DisplayView::DRAG_GROUP;

			// start dragging operation on all selected filters
			start_drag_point = point;
			for (int i=0; i<graph.filters.GetCount(); i++) {
				graph.filters[i]->BeginDrag();
			}
		}
	}

	void DisplayView::OnLButtonUp(UINT nFlags, CPoint point)
	{
		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			Pin *p1 = graph.FindPinByPos(new_connection_start);
			Pin *p2 = graph.FindPinByPos(new_connection_end);

			int ret = graph.ConnectPins(p1, p2);
			if (ret < 0) {
				// TODO: error message
			}
		}
		new_connection_start = CPoint(-100,-100);
		new_connection_end = CPoint(-101, -101);
		drag_mode = DisplayView::DRAG_GROUP;
		ReleaseCapture();
		Invalidate();
	}

	void DisplayView::OnMouseMove(UINT nFlags, CPoint point)
	{
		// loop through the filters...
		if (nFlags & MK_LBUTTON) {

			bool need_invalidate = false;

			switch (drag_mode) {
			case DisplayView::DRAG_GROUP:
				{
					// we are dragging now
					int	deltax = point.x - start_drag_point.x;
					int deltay = point.y - start_drag_point.y;

					// verify the deltas
					int i, selected_count = 0;
					for (i=0; i<graph.filters.GetCount(); i++) {
						Filter *filter = graph.filters[i];
						if (filter->selected) {
							selected_count ++;
							filter->VerifyDrag(&deltax, &deltay);
						}
					}

					// exit if there's no selected filter
					if (selected_count == 0) return ;

					// update their position
					for (i=0; i<graph.filters.GetCount(); i++) {
						Filter *filter = graph.filters[i];
						if (filter->selected) {
							int px = filter->start_drag_pos.x + deltax;
							int py = filter->start_drag_pos.y + deltay;

							// snap to grid
							px = (px+7)&~0x07;
							py = (py+7)&~0x07;

							if (px != filter->posx || py != filter->posy) {
								filter->posx = px;
								filter->posy = py;
								need_invalidate = true;
							}
						}
					}
				}
				break;

			case DisplayView::DRAG_CONNECTION:
				{
					new_connection_end = point;

					Filter	*current = graph.FindFilterByPos(point);
					if (current) {
						Pin *drop_end = current->FindPinByPos(point);
						if (drop_end) {
							drop_end->GetCenterPoint(&new_connection_end);
						}
					}

					need_invalidate = true;
				}
				break;
			}

			if (need_invalidate) Invalidate();
		}
	}

	void DisplayView::OnSize(UINT nType, int cx, int cy)
	{
		CRect	r;
		CDC		*dc = GetDC();
		GetClientRect(&r);

		if ((back_width != r.Width()) || (back_height != r.Height())) {

			if (memDC.GetSafeHdc()) {
				memDC.DeleteDC();
				backbuffer.DeleteObject();
			}

			memDC.CreateCompatibleDC(dc);
			backbuffer.CreateCompatibleBitmap(dc, r.Width(), r.Height());
			memDC.SelectObject(&backbuffer);
			back_width = r.Width();
			back_height = r.Height();
		}

		ReleaseDC(dc);
	}

	void DisplayView::OnDraw(CDC *pDC)
	{
		CRect	r;
		CRect	rect;
		GetClientRect(&r);

		// Set brush to desired background color
		CBrush backBrush(color_back);
		memDC.SelectObject(&backBrush);
		memDC.GetClipBox(&rect);     // Erase the area needed
		memDC.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
		graph.Draw(this);

		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			graph.DrawArrow(this, new_connection_start, new_connection_end);
		}

		pDC->BitBlt(0, 0, r.Width(), r.Height(), &memDC, 0, 0, SRCCOPY);
	}

	void DisplayView::OnRenderPin()
	{
		if (!current_pin) return ;
	
		HRESULT	hr = graph.gb->Render(current_pin->pin);
		if (SUCCEEDED(hr)) {
			graph.RefreshFilters();
			graph.SmartPlacement();
			Invalidate();
		}

	}

	void DisplayView::OnPropertyPage()
	{
		CString	title;
		if (current_pin) {
			title = current_pin->name + _T(" Properties");
			OnDisplayPropertyPage(current_pin->pin, current_filter->filter, title);
			return ;
		}
		if (current_filter) {
			title = current_filter->name + _T(" Properties");
			OnDisplayPropertyPage(current_filter->filter, current_filter->filter, title);
			return ;
		}
	}

	void DisplayView::OnFilterRemoved(DisplayGraph *sender, Filter *filter)
	{
		// to be overriden
	}

	void DisplayView::OnDisplayPropertyPage(IUnknown *object, IUnknown *filter, CString title)
	{
	}


};





