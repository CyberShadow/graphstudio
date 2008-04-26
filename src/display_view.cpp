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

	//-------------------------------------------------------------------------
	//
	//	DisplayView class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(DisplayView, CScrollView)

	BEGIN_MESSAGE_MAP(DisplayView, CScrollView)
		// Standard printing commands
		ON_WM_ERASEBKGND()
		ON_WM_SIZE()
		ON_WM_MOUSEMOVE()
		ON_WM_LBUTTONDOWN()
		ON_WM_LBUTTONDBLCLK()
		ON_WM_RBUTTONDOWN()
		ON_WM_LBUTTONUP()

		ON_COMMAND(ID_PIN_RENDER, &DisplayView::OnRenderPin)
		ON_COMMAND(ID_PIN_NULL_STREAM, &DisplayView::OnRenderNullStream)
		ON_COMMAND(ID_PIN_DUMP_STREAM, &DisplayView::OnDumpStream)
		ON_COMMAND(ID_PROPERTYPAGE, &DisplayView::OnPropertyPage)
	END_MESSAGE_MAP()

	DisplayView::DisplayView()
	{
		back_width = 0;
		back_height = 0;
		overlay_filter = NULL;

		// nastavime DC
		graph.params = &render_params;
		graph.callback = this;
		graph.dc = &memDC;
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
		point += GetScrollPosition();

		// find out if there is any filter being selected
		current_filter = graph.FindFilterByPos(point);
		if (!current_filter) return ;

		// check for a pin - will have different menu
		current_pin = current_filter->FindPinByPos(point, false);
		OnPropertyPage();
	}

	void DisplayView::OnRButtonDown(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();

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
			
			menu.InsertMenu(0, MF_BYPOSITION | MF_STRING | flags, ID_PIN_RENDER, _T("Render Pin"));
			menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
			menu.InsertMenu(2, MF_BYPOSITION | MF_STRING | flags, ID_PIN_NULL_STREAM, _T("Insert Null Renderer"));
			menu.InsertMenu(3, MF_BYPOSITION | MF_STRING | flags, ID_PIN_DUMP_STREAM, _T("Insert Dump Filter"));
			menu.InsertMenu(4, MF_BYPOSITION | MF_SEPARATOR);
			menu.InsertMenu(5, MF_BYPOSITION | MF_STRING, ID_PROPERTYPAGE, _T("Properties"));

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
		point += GetScrollPosition();

		SetCapture();	
		start_drag_point = point;
		end_drag_point = point;

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

			// store the selection point
			drag_mode = DisplayView::DRAG_SELECTION;

			if (need_invalidate) {
				graph.Dirty();
				Invalidate();
			}
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

			int icon = current->CheckIcons(point);
			if (icon >= 0) {
				drag_mode = DisplayView::DRAG_OVERLAY_ICON;
				return ;
			}

			if (current->selected) {
				if (nFlags & MK_SHIFT) {
					current->Select(false);
					graph.Dirty();
					Invalidate();
				} else {
					// nothing here...
				}
			} else {
				if (nFlags & MK_SHIFT) {
					current->Select(true);
					graph.Dirty();
					Invalidate();
				} else {
					// deselect all filters but this
					for (int i=0; i<graph.filters.GetCount(); i++) {
						graph.filters[i]->Select(false);
					}
					current->Select(true);
					graph.Dirty();
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
		point += GetScrollPosition();

		// check for an overlay icon
		if (drag_mode == DisplayView::DRAG_OVERLAY_ICON) {
			Filter	*current = graph.FindFilterByPos(point);
			if (current && current == overlay_filter) {
				int icon = current->CheckIcons(point);
				if (icon >= 0) {
					OnOverlayIconClick(current->overlay_icons[icon], point); 
				}
			}
		}

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
		graph.Dirty();
		Invalidate();
	}

	void DisplayView::OnMouseMove(UINT nFlags, CPoint point)
	{
		point += GetScrollPosition();
		bool need_invalidate = false;

		// loop through the filters...
		if (nFlags & MK_LBUTTON) {


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
			case DisplayView::DRAG_SELECTION:
				{
					int	minx = start_drag_point.x;
					int miny = start_drag_point.y;
					int maxx = point.x;
					int maxy = point.y;

					if (minx > maxx) {
						minx = point.x;
						maxx = start_drag_point.x;
					}
					if (miny > maxy) {
						miny = point.y;
						maxy = start_drag_point.y;
					}

					end_drag_point = point;
					CRect	rc(minx, miny, maxx, maxy);

					for (int i=0; i<graph.filters.GetCount(); i++) {
						Filter *filter = graph.filters[i];

						CRect	rc2(filter->posx, filter->posy, 
									filter->posx+filter->width, 
									filter->posy+filter->height);
						CRect	rc3;						

						rc3.IntersectRect(&rc, &rc2);
						bool sel = (rc3.IsRectEmpty() ? false : true);

						if (sel != filter->selected) {
							filter->Select(sel);
							need_invalidate = true;
						}
					}

					if (!need_invalidate) {
						Invalidate();
					}
				}
				break;
			}

			if (need_invalidate) {
				graph.Dirty();
				Invalidate();
			}
		} else {

			/*
				No buttons are pressed. We only check for overlay icons
			*/

			Filter	*current = graph.FindFilterByPos(point);

			// if there was a filter active before
			if (overlay_filter) {
				// which was not ours
				if (overlay_filter != current) {
					// make it's overlay icon disappear
					overlay_filter->overlay_icon_active = -1;
					need_invalidate = true;
				}
			}

			overlay_filter = current;

			if (current) {		
				int	cur_icon = current->overlay_icon_active;

				int ret = current->CheckIcons(point);
				if (ret != cur_icon) {
					need_invalidate = true;
				}
			}

			if (need_invalidate) {
				graph.Dirty();
				Invalidate();
			}
		}
	}

	void DisplayView::OnSize(UINT nType, int cx, int cy)
	{
		UpdateScrolling();

		/*
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
		*/
	}

	void DisplayView::RepaintBackbuffer()
	{
		CSize	size = graph.GetGraphSize();

		if (size.cx != back_width || size.cy != back_height) {

			CDC		*dc = GetDC();

			// we initialize a new backbuffer with the size of the graph
			if (memDC.GetSafeHdc()) {
				memDC.DeleteDC();
				backbuffer.DeleteObject();
			}

			memDC.CreateCompatibleDC(dc);
			backbuffer.CreateCompatibleBitmap(dc, size.cx, size.cy);
			memDC.SelectObject(&backbuffer);
			back_width = size.cx;
			back_height = size.cy;

			ReleaseDC(dc);

			graph.dirty = true;
		}

		if (graph.dirty) {
			CRect	rect;

			// now we repaint the buffer
			CBrush backBrush(render_params.color_back);

			memDC.SelectObject(&backBrush);
			memDC.GetClipBox(&rect);
			memDC.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);

			graph.Draw(&memDC);

			// not dirty anymore
			graph.dirty = false;
			UpdateScrolling();
		}
	}

	void DisplayView::OnDraw(CDC *pDC)
	{
		CRect	r;
		CRect	rect;
		GetClientRect(&r);

		// Set brush to desired background color
		RepaintBackbuffer();
		pDC->BitBlt(0, 0, back_width, back_height, &memDC, 0, 0, SRCCOPY);

		// paint the rest of client area with background brush
		CBrush backBrush(render_params.color_back);
		CBrush *prev_brush = pDC->SelectObject(&backBrush);
		pDC->PatBlt(back_width, 0, r.Width(), r.Height(), PATCOPY);
		pDC->PatBlt(0, back_height, back_width, r.Height(), PATCOPY);

		pDC->SelectObject(prev_brush);

		// draw arrow
		if (drag_mode == DisplayView::DRAG_CONNECTION) {
			graph.DrawArrow(pDC, new_connection_start, new_connection_end);
		} else
		if (drag_mode == DisplayView::DRAG_SELECTION) {

			// select a null (hollow) brush
			CBrush	*pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
			CPen	pen(PS_DOT, 1, RGB(0,0,0));
			CPen	*pOldPen = pDC->SelectObject(&pen);

			// set pen pixels as the inverse of the screen color.
			int nOldROP2 = pDC->SetROP2(R2_XORPEN);

			// draw new or erase old selection rectangle
			pDC->Rectangle(start_drag_point.x, start_drag_point.y,
						   end_drag_point.x, end_drag_point.y);
			
			pDC->SetROP2(nOldROP2);
			pDC->SelectObject(pOldBrush);
			pDC->SelectObject(pOldPen);
		}
	}

	void DisplayView::OnDumpStream()
	{
		if (!current_pin) return ;

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = CoCreateInstance(DSUtil::CLSID_Dump, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
			// try our internal Dump Filter as an alternative
			CMonoDump	*dump = new CMonoDump(NULL, &hr);
			hr = dump->NonDelegatingQueryInterface(IID_IBaseFilter, (void**)&instance);
		} 
		
		if (SUCCEEDED(hr)){
			
			// now check for a few interfaces
			int ret = ConfigureInsertedFilter(instance);
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {

				IPin		*outpin = current_pin->pin;
				outpin->AddRef();

				// add the filter to graph
				hr = graph.AddFilter(instance, _T("Dump"));
				if (FAILED(hr)) {
					// display error message
				} else {
					// connect the pin to the renderer
					hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

					graph.RefreshFilters();
					graph.SmartPlacement();
					graph.Dirty();
					Invalidate();
				}

				outpin->Release();
			}
		}
		instance = NULL;
		current_pin = NULL;
	}

	void DisplayView::OnRenderNullStream()
	{
		if (!current_pin) return ;

		// now create an instance of this filter
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr;

		hr = CoCreateInstance(DSUtil::CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
		if (FAILED(hr)) {
			// display error message
		} else {
			
			// now check for a few interfaces
			int ret = ConfigureInsertedFilter(instance);
			if (ret < 0) {
				instance = NULL;
			}

			if (instance) {

				IPin		*outpin = current_pin->pin;
				outpin->AddRef();

				// add the filter to graph
				hr = graph.AddFilter(instance, _T("Null Renderer"));
				if (FAILED(hr)) {
					// display error message
				} else {
					// connect the pin to the renderer
					hr = DSUtil::ConnectPin(graph.gb, outpin, instance);

					graph.RefreshFilters();
					graph.SmartPlacement();
					graph.Dirty();
					Invalidate();
				}

				outpin->Release();
			}
		}
		instance = NULL;
		current_pin = NULL;
	}

	void DisplayView::OnRenderPin()
	{
		if (!current_pin) return ;
	
		HRESULT	hr = graph.gb->Render(current_pin->pin);
		if (SUCCEEDED(hr)) {
			graph.RefreshFilters();
			graph.SmartPlacement();
			graph.Dirty();
			Invalidate();
		}
		current_pin = NULL;
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

	void DisplayView::OnOverlayIconClick(OverlayIcon *icon, CPoint point)
	{
	}

	// scrolling aid
	void DisplayView::UpdateScrolling()
	{
		CSize	size = graph.GetGraphSize();
		
		SetScrollSizes(MM_TEXT, size);

	}

	void DisplayView::MakeScreenshot()
	{
		// find out the rectangle
		int	minx = 10000000;
		int	miny = 10000000;
		int maxx = 0;
		int maxy = 0;

		for (int i=0; i<graph.filters.GetCount(); i++) {
			Filter	*filter = graph.filters[i];
			if (filter->posx < minx) minx = filter->posx;
			if (filter->posy < miny) miny = filter->posy;
			if (filter->posx + filter->width > maxx) maxx = filter->posx+filter->width;
			if (filter->posy + filter->height > maxy) maxy = filter->posy+filter->height;
		}

		minx = minx &~ 0x07; minx -= 8;	if (minx < 0) minx = 0;
		miny = miny &~ 0x07; miny -= 8;	if (miny < 0) miny = 0;
		maxx = (maxx+7) &~ 0x07; maxx += 8;
		maxy = (maxy+7) &~ 0x07; maxy += 8;

		// now copy the bitmap
		int	cx = (maxx-minx);
		int cy = (maxy-miny);

		if (cx == 0 || cy == 0) {
			OpenClipboard();
			EmptyClipboard();
			CloseClipboard();
			return ;
		}

		CRect		imgrect(minx, miny, maxx, maxy);
		CRect		bufrect(0, 0, back_width, back_height);
		CDC			tempdc;
		CBitmap		tempbitmap;

		CRect		area=imgrect;
		area.IntersectRect(&imgrect, &bufrect);

		tempdc.CreateCompatibleDC(&memDC);
		tempbitmap.CreateBitmap(area.Width(), area.Height(), 1, 32, NULL);
		CBitmap *old = tempdc.SelectObject(&tempbitmap);
		tempdc.BitBlt(0, 0, area.Width(), area.Height(), &memDC, area.left, area.top, SRCCOPY);

		OpenClipboard();
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, tempbitmap.GetSafeHandle());
		CloseClipboard();

		tempdc.SelectObject(old);
		tempbitmap.DeleteObject();
		tempdc.DeleteDC();

	}

};





