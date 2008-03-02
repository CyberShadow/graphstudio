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

	PropTreeCtrl::PropTreeCtrl() :
		CTreeCtrl()
	{
	}

	PropTreeCtrl::~PropTreeCtrl()
	{
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
		color_back_group = RGB(230, 230, 230);
		color_back_item = RGB(255, 255, 255);

		left_offset = 12;
		left_width  = 100;
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
		tree.Create(WS_CHILD | WS_VISIBLE  | 
					TVS_NOHSCROLL | TVS_NOTOOLTIPS | 
					TVS_HASBUTTONS | TVS_LINESATROOT, 
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
		case CDDS_ITEMPREPAINT:	
			{
				// we paint the item manually
				HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
				PaintItem(hItem, pNMCustomDraw);				

				*pResult = CDRF_SKIPDEFAULT; 
			}
			break;
		default:
			*pResult = CDRF_DODEFAULT;
		}
	}

	void PropertyTree::PaintItem(HTREEITEM item, NMCUSTOMDRAW *draw)
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

		if (prop->type == GraphStudio::PropItem::TYPE_STRUCT) {
			// we paint the whole background
			dc.Rectangle(rc);

		} else {
			// we paint just the left margin
			CRect	rc_left = rc;
			rc_left.right = rc_left.left + left_offset;
			dc.Rectangle(&rc_left);
			dc.MoveTo(rc.left, rc.bottom);
			dc.LineTo(rc.right, rc.bottom);

			dc.SelectObject(&brush_item);
			dc.SelectObject(&pen_item);
			dc.Rectangle(rc_left.right, rc_left.top+1, rc.right, rc.bottom -1);
		}

		CRect	rc_text = rc;
		rc_text.left += left_offset +2;
		rc_text.top += 1;
		rc_text.bottom -= 1;

		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(prop->name, &rc_text, DT_VCENTER | DT_SINGLELINE);


		dc.SelectObject(prev_brush);
		dc.SelectObject(prev_pen);
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

