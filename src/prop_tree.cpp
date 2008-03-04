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
	//	PropItem class
	//
	//-------------------------------------------------------------------------
	
	PropItem::PropItem(CString n) :
		name(n),
		value(_T("")),
		type(TYPE_STRUCT)
	{
	}

	PropItem::PropItem(CString n, GUID guid) :
		name(n),
		value(_T("")),
		type(TYPE_GUID)
	{
		// convert to string
		LPOLESTR	str = NULL;
		StringFromCLSID(guid, &str);

		value = str;
		if (str) CoTaskMemFree(str);
	}

	PropItem::PropItem(CString n, int val) :
		name(n),
		type(TYPE_INT)
	{
		value.Format(_T("%d"), val);
	}

	PropItem::PropItem(CString n, CString str) :
		name(n),
		type(TYPE_INT)
	{
		value = str;
	}

	PropItem::PropItem(CString n, bool val) :
		name(n),
		type(TYPE_INT)
	{
		value = (val ? _T("TRUE") : _T("FALSE"));
	}

	PropItem::~PropItem()
	{
		Clear();
	}

	void PropItem::Clear()
	{
		for (int i=0; i<items.GetCount(); i++) {
			PropItem *item = items[i];
			delete item;
		}
		items.RemoveAll();
	}

	// build up the tree
	PropItem *PropItem::AddItem(PropItem *item)
	{
		items.Add(item);
		return item;
	}


	//-------------------------------------------------------------------------
	//
	//	PropTreeCtrl class
	//
	//-------------------------------------------------------------------------

	BEGIN_MESSAGE_MAP(PropTreeCtrl, CTreeCtrl)
		ON_WM_LBUTTONDOWN()
	END_MESSAGE_MAP()

	PropTreeCtrl::PropTreeCtrl() :
		CTreeCtrl()
	{
	}

	PropTreeCtrl::~PropTreeCtrl()
	{
	}

	void PropTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
	{
		HTREEITEM	item = HitTest(point);
		if (item) {
			if (!ItemHasChildren(item)) return ;

			CRect	rc_check;
			CRect	rc;
			GetItemRect(item, &rc, FALSE);

			rc_check.left = rc.left + 2;
			rc_check.top  = rc.top  + 4;
			rc_check.right = rc_check.left + 10;
			rc_check.bottom = rc_check.top + 10;

			// toggle item
			if (rc_check.PtInRect(point)) {
				Expand(item, TVE_TOGGLE);
			}
		}
	}


	//-------------------------------------------------------------------------
	//
	//	PropertyTree class
	//
	//-------------------------------------------------------------------------

	IMPLEMENT_DYNCREATE(PropertyTree, CStatic)
	BEGIN_MESSAGE_MAP(PropertyTree, CStatic)
		ON_WM_SIZE()
	END_MESSAGE_MAP()


	PropertyTree::PropertyTree() :
		CStatic()
	{
		MakeFont(font_group, _T("Tahoma"), 8, true, false);
		MakeFont(font_item, _T("Tahoma"), 8, false, false);

		color_group = RGB(0,0,0);
		color_item = RGB(0,0,0);
		color_back_group = RGB(241, 239, 226);
		color_back_item = RGB(255, 255, 255);

		left_offset = 14;
		left_width  = 130;
	}

	PropertyTree::~PropertyTree()
	{
	}


	void PropertyTree::Initialize()
	{
		if (tree.m_hWnd) return ;

		CRect rcClient;
		GetClientRect(&rcClient);

		// create tree and header controls as children
		tree.Create(WS_CHILD | WS_VISIBLE  | TVS_FULLROWSELECT |
					TVS_NOHSCROLL | TVS_NOTOOLTIPS,				
					CRect(), this, ID_TREE);

		RepositionControls();
	}

	void PropertyTree::OnSize(UINT nType, int cx, int cy)
	{
		RepositionControls();
	}

	void PropertyTree::BuildPropertyTree(PropItem *root)
	{
		tree.DeleteAllItems();
		BuildNode(root, tree.GetRootItem());
	}

	void PropertyTree::BuildNode(PropItem *node, HTREEITEM item)
	{
		int i;
		for (i=0; i<node->GetCount(); i++) {
			PropItem *pi = node->GetItem(i);

			HTREEITEM	newitem = tree.InsertItem(pi->name, item);
			tree.SetItemData(newitem, (DWORD_PTR)pi);

			BuildNode(pi, newitem);
		}

		// rozbalime
		tree.Expand(item, TVE_EXPAND);
	}

	void PropertyTree::RepositionControls()
	{
		if (tree.m_hWnd) {
			CRect		rcClient;
			GetClientRect(&rcClient);

			tree.MoveWindow(&rcClient);
		}
	}

	void PropertyTree::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
	{
		NMCUSTOMDRAW	*pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
		NMTVCUSTOMDRAW	*pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;

		switch (pNMCustomDraw->dwDrawStage) {
		case CDDS_PREPAINT:		*pResult = CDRF_NOTIFYITEMDRAW;	break;
		case CDDS_PREERASE:		*pResult = CDRF_SKIPDEFAULT; break;
		case CDDS_ITEMPREPAINT:	
			{
				// we paint the item manually
				HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
				PaintItem(hItem, pNMCustomDraw->uItemState, pNMCustomDraw);				

				*pResult = CDRF_SKIPDEFAULT; 
			}
			break;
		default:
			*pResult = CDRF_DODEFAULT;
		}
	}

	void PropertyTree::PaintItem(HTREEITEM item, UINT state, NMCUSTOMDRAW *draw)
	{
		GraphStudio::PropItem	*prop = (GraphStudio::PropItem*)tree.GetItemData(item);
		if (!prop) return ;

		CRect	rc = draw->rc;
		CDC		dc;
		dc.Attach(draw->hdc);

		// we either draw a node or a leaf
		CBrush		brush_back(color_back_group);
		CBrush		brush_item(color_back_item);
		CPen		pen_back(PS_SOLID, 1, color_back_group);
		CPen		pen_item(PS_SOLID, 1, color_back_item);

		CBrush		*prev_brush = dc.SelectObject(&brush_back);
		CPen		*prev_pen   = dc.SelectObject(&pen_back);
		CFont		*prev_font	= dc.SelectObject(&font_group);

		int			text_left = left_offset + 2;

		dc.SetBkMode(TRANSPARENT);

		if (prop->type == GraphStudio::PropItem::TYPE_STRUCT) {
			// we paint the whole background
			dc.Rectangle(rc);
			dc.SetTextColor(color_group);

			// draw the text
			CRect	rc_text = rc;
			rc_text.left += text_left;
			rc_text.top += 1;
			rc_text.bottom -= 1;
			dc.DrawText(prop->name, &rc_text, DT_VCENTER | DT_SINGLELINE);

			// draw the + mark
			state = tree.GetItemState(item, TVIF_STATE);
			BOOL expanded = (state & TVIS_EXPANDED ? TRUE : FALSE);
			
			CPen	black_pen(PS_SOLID, 1, RGB(0,0,0));
			CRect	rc_mark;
			dc.SelectObject(&black_pen);

			rc_mark.left   = rc.left + 2;
			rc_mark.top    = rc.top  + 4;
			rc_mark.right  = rc_mark.left + 9;
			rc_mark.bottom = rc_mark.top + 9;
			dc.Rectangle(&rc_mark);

			dc.MoveTo(rc_mark.left + 2, rc_mark.top + 4);
			dc.LineTo(rc_mark.right - 2, rc_mark.top + 4);
			if (!expanded) {
				dc.MoveTo(rc_mark.left + 4, rc_mark.top + 2);
				dc.LineTo(rc_mark.left + 4, rc_mark.bottom - 2);
			}

		} else {
			// we paint just the left margin
			CRect	rc_left = rc;
			rc_left.right = rc_left.left + left_offset;
			dc.Rectangle(&rc_left);
			dc.MoveTo(rc.left, rc.bottom);
			dc.LineTo(rc.right, rc.bottom);

			// middle line
			int	mid_line = rc.left + left_offset + left_width;

			dc.MoveTo(mid_line, rc.top);
			dc.LineTo(mid_line, rc.bottom);

			dc.SelectObject(&brush_item);
			dc.SelectObject(&pen_item);
			dc.Rectangle(rc_left.right, rc_left.top+1, mid_line, rc.bottom -2);
			dc.Rectangle(mid_line+1, rc_left.top+1, rc.right, rc.bottom -2);

			dc.SelectObject(&font_item);
			text_left += 2;
			dc.SetTextColor(color_item);

			CRect	rc_text = rc;
			rc_text.top += 1;
			rc_text.bottom -= 1;

			rc_text.left += text_left;
			rc_text.right = mid_line - 2;
			dc.DrawText(prop->name, &rc_text, DT_VCENTER | DT_SINGLELINE);

			rc_text.left = mid_line + 4;
			rc_text.right = rc.right - 2;
			dc.DrawText(prop->value, &rc_text, DT_VCENTER | DT_SINGLELINE);			
		}

		dc.SelectObject(prev_brush);
		dc.SelectObject(prev_pen);
		dc.SelectObject(prev_font);
		dc.Detach();
	}

	BOOL PropertyTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
	{
		LPNMHDR pHdr = (LPNMHDR)lParam;

		switch (pHdr->code) {
		case NM_CUSTOMDRAW:
			{
				OnTreeCustomDraw(pHdr, pResult);
				return TRUE;
			}
			break;
		case HDN_ITEMCHANGING:
		case HDN_ITEMCHANGED:
			{
				// header ?
				return TRUE;
			}
			break;
		case TVN_ITEMEXPANDING:			Invalidate();
		case TVN_ITEMEXPANDED:			RepositionControls();
		}

		// forward notifications from children to the control owner
		pHdr->hwndFrom = GetSafeHwnd();
		pHdr->idFrom = GetWindowLong(GetSafeHwnd(),GWL_ID);
		return (BOOL)GetParent()->SendMessage(WM_NOTIFY,wParam,lParam);			
	}

};

