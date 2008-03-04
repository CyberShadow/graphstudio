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
	//	PropItem class
	//
	//-------------------------------------------------------------------------
	
	class PropItem
	{
	public:
		CArray<PropItem*>	items;

		CString				name;
		CString				value;

		enum {
			TYPE_UNKNOWN	= 0,
			TYPE_STRUCT		= 1,
			TYPE_INT		= 2,
			TYPE_STRING		= 3,
			TYPE_BOOL		= 4,
			TYPE_GUID		= 5,
			TYPE_RECT		= 6
		};

		int					type;

	public:
		// struct
		PropItem(CString n);

		// constructors for separate types
		PropItem(CString n, GUID guid);
		PropItem(CString n, int val);
		PropItem(CString n, CString str);
		PropItem(CString n, bool val);
		PropItem(CString n, RECT rc);
		PropItem(CString n, __int64 i);

		virtual ~PropItem();

		void Clear();

		int GetCount() { return (int)items.GetCount(); }
		PropItem *GetItem(int n) { return ( n<0 || n >= items.GetCount() ? NULL : items[n]); }

		// build up the tree
		PropItem *AddItem(PropItem *item);
	};

	//-------------------------------------------------------------------------
	//
	//	PropTreeCtrl class
	//
	//-------------------------------------------------------------------------

	class PropTreeCtrl : public CTreeCtrl
	{
	protected:
		DECLARE_MESSAGE_MAP()
	public:
	public:
		PropTreeCtrl();
		~PropTreeCtrl();

		void OnLButtonDown(UINT nFlags, CPoint point);
	};

	//-------------------------------------------------------------------------
	//
	//	PropertyTree class
	//
	//-------------------------------------------------------------------------

	class PropertyTree : public CStatic
	{
	protected:
		DECLARE_MESSAGE_MAP()

	public:
		DECLARE_DYNCREATE(PropertyTree)

		PropTreeCtrl		tree;

		enum {
			ID_TREE = 1
		};

		// rendering stuff
		CFont				font_group;
		CFont				font_item;

		DWORD				color_group;
		DWORD				color_item;
		DWORD				color_back_item;
		DWORD				color_back_group;

		int					left_offset;
		int					left_width;

	public:
		PropertyTree();
		~PropertyTree();

		virtual void Initialize();
		virtual void RepositionControls();

		void OnSize(UINT nType, int cx, int cy);
		void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

		BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult);
		void PaintItem(HTREEITEM item, UINT state, NMCUSTOMDRAW *draw);

		void BuildPropertyTree(PropItem *root);
		void BuildNode(PropItem *node, HTREEITEM item);
	};

};

