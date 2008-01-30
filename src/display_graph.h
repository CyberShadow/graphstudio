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
	class DisplayView;
	class DisplayGraph;
	class Filter;

	//-------------------------------------------------------------------------
	//
	//	Pin class
	//
	//-------------------------------------------------------------------------
	class Pin
	{
	public:
		Filter					*filter;
		CString					name;
		CString					id;
		CComPtr<IPin>			pin;
		Pin						*peer;			// peer pin
		PIN_DIRECTION			dir;
		bool					connected;
		bool					selected;		// selected connection pin->peer (only valid for output pins)

	public:
		Pin(Filter *parent);
		virtual ~Pin();

		// draw the pin
		void Draw(CDC *dc, DisplayView *view, bool input, int x, int y);
		void GetCenterPoint(CPoint *pt);

		int Load(IPin *pin);
		void LoadPeer();
		bool IsConnected();
		void Select(bool select);

		// operations
		int Disconnect();
	};

	//-------------------------------------------------------------------------
	//
	//	Filter class
	//
	//-------------------------------------------------------------------------
	class Filter
	{
	public:
		static int				MINWIDTH;
		static int				MINHEIGHT;

		DisplayGraph			*graph;
		CString					name;				// name of the filter
		CString					display_name;		// name as it appears
		CLSID					clsid;				// it's CLASS_ID
		CString					clsid_str;			// string version

		CComPtr<IBaseFilter>	filter;
		CArray<Pin*>			input_pins;
		CArray<Pin*>			output_pins;

		int						posx, posy;			// position in the display
		int						width, height;
		CPoint					start_drag_pos;

		int						tag;				// helper
		int						depth;	
		bool					selected;
		bool					connected;

		enum {
			FILTER_UNKNOWN = 0,
			FILTER_STANDARD = 1,
			FILTER_WDM = 2,
			FILTER_DMO = 3
		};
		int						filter_type;				

	public:
		Filter(DisplayGraph *parent);
		virtual ~Filter();

		// kreslenie filtra
		void Draw(DisplayView *view);
		void DrawConnections(DisplayView *view);

		// I/O
		void Release();
		void RemovePins();
		void LoadFromFilter(IBaseFilter *f);
		void Refresh();
		void LoadPin(IPin *pin, PIN_DIRECTION dir);
		Pin *FindPin(IPin *pin);
		Pin *FindPinByPos(CPoint p, bool not_connected=true);
		bool HasPin(IPin *pin);
		void LoadPeers();
		void DeleteSelectedConnections();
		void DeleteFilter();

		// Helpers
		bool IsSource();
		bool IsRenderer();
		int NumOfDisconnectedPins(PIN_DIRECTION dir);

		// placement 
		void CalculatePlacementChain(int new_depth, int x);
		void UpdatePinPositions();

		// filter dragging
		void BeginDrag();
		void VerifyDrag(int *deltax, int *deltay);
		void Select(bool select);
		void SelectConnection(UINT flags, CPoint pt);
	};


	//-------------------------------------------------------------------------
	//
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------
	
	#define WM_GRAPH_EVENT		(WM_USER + 880)

	class GraphCallback
	{
	public:
		virtual void OnFilterRemoved(DisplayGraph *sender, Filter *filter) = 0;
	};

	class DisplayGraph
	{
	public:

		// graph itself
		CComPtr<IGraphBuilder>			gb;
		CComPtr<IMediaControl>			mc;
		CComPtr<IMediaEventEx>			me;
		CComPtr<IMediaSeeking>			ms;

		CArray<Filter*>					filters;
		CDC								*dc;
		CFont							*filter_font;

		// bins in filters are (smart placement)
		CArray<CPoint>					bins;

		// 
		HWND							wndEvents;
		GraphCallback					*callback;

	public:
		DisplayGraph();
		virtual ~DisplayGraph();

		// let's build something
		int MakeNew();
		int RenderFile(CString fn);
		int LoadGRF(CString fn);
		int SaveGRF(CString fn);

		// adding filters
		HRESULT AddFilter(IBaseFilter *filter, CString proposed_name);

		void RefreshFilters();
		void ZeroTags();
		Filter *FindFilter(IBaseFilter *filter);
		Filter *FindParentFilter(IPin *pin);
		Pin *FindPin(IPin *pin);
		void RemoveUnusedFilters();
		void SmartPlacement();
		void LoadPeers();
		void DeleteSelected();
		int ConnectPins(Pin *p1, Pin *p2);

		// rendering the graph
		void Draw(DisplayView *view);
		void DrawArrow(DisplayView *view, CPoint p1, CPoint p2);

		// mouse interaction
		Filter *FindFilterByPos(CPoint pt);
		Pin *FindPinByPos(CPoint pt);

		// state information
		int GetState(FILTER_STATE &state, DWORD timeout=INFINITE);

		// seeking helpers
		int GetPositions(double &current_ms, double &duration_ms);
		int Seek(double time_ms);
	};

	// helpers
	bool LineHit(CPoint p1, CPoint p2, CPoint hit); 
	void DoDrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color);

	// GUID helpers
	bool NameGuid(GUID guid, CString &str);

};
