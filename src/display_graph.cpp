//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <math.h>
#include <atlbase.h>
#include <atlpath.h>

#pragma warning(disable: 4244)			// DWORD -> BYTE warning

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
	//	DisplayGraph class
	//
	//-------------------------------------------------------------------------

	DisplayGraph::DisplayGraph()
	{
		callback = NULL;

		mc = NULL;
		me = NULL;
		ms = NULL;
		gb = NULL;
		dc = NULL;
		is_remote = false;

		MakeNew();

		dirty = true;
	}

	DisplayGraph::~DisplayGraph()
	{
		mc = NULL;
		me = NULL;
		gb = NULL;
		if (ms) ms = NULL;

		ZeroTags();
		RemoveUnusedFilters();
	}

	int DisplayGraph::ConnectToRemote(IFilterGraph *remote_graph)
	{
		int ret = MakeNew();
		if (ret < 0) return -1;

		// release graph objects
		mc = NULL;
		ms = NULL;
		if (me) {
			// clear events...
			me->SetNotifyWindow(NULL, 0, NULL);
			me = NULL;
		}
		gb = NULL;
		is_remote = false;

		// attach remote graph
		HRESULT hr;
		hr = remote_graph->QueryInterface(IID_IGraphBuilder, (void**)&gb);
		if (FAILED(hr)) return -1;

		// get hold of interfaces
		gb->QueryInterface(IID_IMediaControl, (void**)&mc);
		gb->QueryInterface(IID_IMediaSeeking, (void**)&ms);

		// now we're a remote graph
		is_remote = true;
		return 0;
	}

	int DisplayGraph::MakeNew()
	{
		if (ms) ms = NULL;

		// we only do this for our own graph so we don't mess up
		// the host application when connected to remote graph
		if (!is_remote) {
			if (mc) {
				mc->Stop();
				mc = NULL;
			}

			if (me) {
				// clear events...
				me->SetNotifyWindow(NULL, 0, NULL);
				me = NULL;
			}

			ZeroTags();
			RemoveAllFilters();
			RemoveUnusedFilters();
			bins.RemoveAll();
			gb = NULL;
		} else {
			mc = NULL;
			me = NULL;

			ZeroTags();
			RemoveUnusedFilters();
			bins.RemoveAll();
			gb = NULL;
		}

		is_remote = false;


		// create new instance of filter graph
		HRESULT hr;
		do {
			hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&gb);
			if (FAILED(hr)) break;

			gb->QueryInterface(IID_IMediaControl, (void**)&mc);

			// setup event handler
			gb->QueryInterface(IID_IMediaEventEx, (void**)&me);
			me->SetNotifyWindow((OAHWND)wndEvents, WM_GRAPH_EVENT, (LONG_PTR)this);
			gb->QueryInterface(IID_IMediaSeeking, (void**)&ms);

		} while (0);

		if (FAILED(hr)) {
			gb = NULL;
			mc = NULL;
			return -1;
		}

		return 0;
	}

	void DisplayGraph::RemoveAllFilters()
	{
		// we find all filters, disconnect them and remove from graph
		for (int i=0; i<filters.GetCount(); i++) {
			if (callback) callback->OnFilterRemoved(this, filters[i]);
			filters[i]->DeleteFilter();
		}
		RefreshFilters();
		Dirty();
	}

	CSize DisplayGraph::GetGraphSize()
	{
		// find out the rectangle
		int maxx = 0;
		int maxy = 0;

		for (int i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			if (filter->posx + filter->width > maxx) maxx = filter->posx+filter->width;
			if (filter->posy + filter->height > maxy) maxy = filter->posy+filter->height;
		}

		// 8-pixel alignment
		maxx = (maxx+7) &~ 0x07; maxx += 8;
		maxy = (maxy+7) &~ 0x07; maxy += 8;

		return CSize(maxx, maxy);
	}

	int DisplayGraph::GetState(FILTER_STATE &state, DWORD timeout)
	{
		if (!mc) {
			state = State_Stopped;
			return -1;
		}

		HRESULT hr = mc->GetState(timeout, (OAFilterState*)&state);
		if (FAILED(hr)) return hr;

		return hr;
	}

	int DisplayGraph::Seek(double time_ms)
	{
		if (!ms) return -1;

		REFERENCE_TIME	rtpos = time_ms * 10000;
		ms->SetPositions(&rtpos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
		return 0;
	}

	// seeking helpers
	int DisplayGraph::GetPositions(double &current_ms, double &duration_ms)
	{
		if (!ms) {
			current_ms = 0;
			duration_ms = 0;
			return 0;
		}

		HRESULT hr = NOERROR;
		do {
			GUID			time_format;
			REFERENCE_TIME	rtDur, rtCur;
			hr = ms->GetTimeFormat(&time_format);
			if (FAILED(hr)) break;

			// get duration
			hr = ms->GetDuration(&rtDur);
			if (FAILED(hr)) {
				duration_ms = 0;
			} else {
				// do we need to convert the time ?
				if (time_format != TIME_FORMAT_MEDIA_TIME) {
					REFERENCE_TIME	temp;
					GUID			out_format = TIME_FORMAT_MEDIA_TIME;

					hr = ms->ConvertTimeFormat(&temp, &out_format, rtDur, &time_format);
					if (FAILED(hr)) {
						temp = 0;
					}
					rtDur = temp;
				}

				// in seconds
				duration_ms = (double)rtDur / 10000.0;
			}

			// get position
			hr = ms->GetCurrentPosition(&rtCur);
			if (FAILED(hr)) {
				current_ms = 0;
			} else {
				// do we need to convert the time ?
				if (time_format != TIME_FORMAT_MEDIA_TIME) {
					REFERENCE_TIME	temp;
					GUID			out_format = TIME_FORMAT_MEDIA_TIME;

					hr = ms->ConvertTimeFormat(&temp, &out_format, rtCur, &time_format);
					if (FAILED(hr)) {
						temp = 0;
					}
					rtCur = temp;
				}

				// in seconds
				current_ms = (double)rtCur / 10000.0;
			}

			hr = NOERROR;
		} while (0);

		if (FAILED(hr)) {
			current_ms = 0;
			duration_ms = 0;
			return hr;
		}

		return 0;
	}

	void DisplayGraph::DeleteSelected()
	{
		// first delete connections and then filters
		for (int i=0; i<filters.GetCount(); i++) {
			filters[i]->DeleteSelectedConnections();
		}
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->selected) {
				if (callback) callback->OnFilterRemoved(this, filters[i]);
				filters[i]->DeleteFilter();
			}
		}
		RefreshFilters();
	}

	HRESULT DisplayGraph::AddFilter(IBaseFilter *filter, CString proposed_name)
	{
		if (!gb) return E_FAIL;

		// find the best name
		CComPtr<IBaseFilter>	temp;
		CString					name = proposed_name;

		HRESULT hr = gb->FindFilterByName(name, &temp);
		if (SUCCEEDED(hr)) {
			temp = NULL;
			int i=0;
			do {
				name.Format(_T("%s %.04d"), proposed_name, i);
				hr = gb->FindFilterByName(name, &temp);
				temp = NULL;
				i++;
			} while (hr == NOERROR);
		}

		// now we have unique name
		hr = gb->AddFilter(filter, name);
		if (FAILED(hr)) {
			// cannot add filter
			return hr;
		}

		// refresh our filters
		RefreshFilters();
		return NOERROR;
	}

	int DisplayGraph::LoadXML(CString fn)
	{
		XML::XMLFile			xml;
		int						ret;

		ret = xml.LoadFromFile(fn);
		if (ret < 0) {
			return -1;
		}

		// load graph
		XML::XMLNode			*root = xml.root;
		XML::XMLIterator		it;
		if (root->Find(_T("graph"), &it) < 0) return -1;

		XML::XMLNode			*gn = *it;
		for (it = gn->nodes.begin(); it != gn->nodes.end(); it++) {
			XML::XMLNode	*node = *it;

			if (node->name == _T("filter"))	ret = LoadXML_Filter(node); else
			if (node->name == _T("render")) ret = LoadXML_Render(node); else
			if (node->name == _T("connect")) ret = LoadXML_Connect(node); else
			{
				ret = -1;
			}

			// is everything okay ?
			if (ret < 0) {
				return -1;
			}
		}

		return 0;
	}

	int DisplayGraph::LoadXML_Render(XML::XMLNode *node)
	{
		CString		pin_path = node->GetValue(_T("pin"));
		Pin			*pin = FindPin(pin_path);
		if (!pin) return -1;

		// try to render
		HRESULT		hr = gb->Render(pin->pin);
		if (FAILED(hr)) return -1;

		// reload newly added filters
		RefreshFilters();
		return 0;
	}

	int DisplayGraph::LoadXML_Connect(XML::XMLNode *node)
	{
		CString		opin_path = node->GetValue(_T("out"));
		CString		ipin_path = node->GetValue(_T("in"));

		CString		direct    = node->GetValue(_T("direct"));
		if (direct == _T("")) direct = _T("false");

		Pin			*opin = FindPin(opin_path);
		Pin			*ipin = FindPin(ipin_path);

		if (!opin || !ipin) return -1;

		HRESULT hr;
		if (direct == _T("false")) {
			hr = gb->Connect(opin->pin, ipin->pin);
		} else {
			hr = gb->ConnectDirect(opin->pin, ipin->pin, NULL);
		}
		if (FAILED(hr)) return -1;

		// reload newly added filters
		RefreshFilters();
		return 0;
	}

	int DisplayGraph::LoadXML_Filter(XML::XMLNode *node)
	{
		CString					name		= node->GetValue(_T("name"));
		CString					clsid_str	= node->GetValue(_T("clsid"));
		CString					dn			= node->GetValue(_T("displayname"));
		GUID					clsid;
		CComPtr<IBaseFilter>	instance;
		HRESULT					hr = NOERROR;
		int						filter_id_type = -1, ret = 0;
		bool					is_configured = false;

		// detect how the filter is described
		if (clsid_str != _T("")) {
			filter_id_type = 0;
			if (FAILED(CLSIDFromString((LPOLESTR)clsid_str.GetBuffer(), &clsid))) return -1;
		} else
		if (dn != _T("")) {
			filter_id_type = 1;
		}

		// create the filter
		switch (filter_id_type) {
		case 0:
			{
				// create by CLSID
				hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
			}
			break;
		case 1:
			{
				// create by display name
				CComPtr<IMoniker>		moniker;
				CComPtr<IBindCtx>		bind;
				ULONG					eaten = 0;

				hr = CreateBindCtx(0, &bind);
				if (SUCCEEDED(hr)) {
					hr = MkParseDisplayName(bind, dn, &eaten, &moniker);
					if (SUCCEEDED(hr)) {
						hr = moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&instance);
					}
				}

				if (FAILED(hr)) instance = NULL;
				bind = NULL;
				moniker = NULL;
			}
			break;
		default:
			{
				hr = E_FAIL;
			}
			break;
		}

		// add the filter instance
		if (SUCCEEDED(hr) && instance) {
			// add the filter to graph
			hr = AddFilter(instance, name);
			if (FAILED(hr)) {
				// display error message
				ret = -1;
			} else {
				SmartPlacement();
			}
		}

		// check for known interfaces
		if (SUCCEEDED(hr)) {
			ret = LoadXML_Interfaces(node, instance);
			is_configured = (ret == 0 ? true : false);
		}

		if (ret == 0 && instance != NULL && !is_configured) {

			// now check for a few interfaces
			int r = ConfigureInsertedFilter(instance);
			if (r < 0) {
				instance = NULL;
			}
		}

		RefreshFilters();

		// we're done
		instance = NULL;
		return ret;
	}

	int DisplayGraph::LoadXML_Interfaces(XML::XMLNode *node, IBaseFilter *filter)
	{
		/*
			Returns 0 if configured and 1 if untouched
		*/
		int		ret = 1;
		HRESULT	hr;

		XML::XMLIterator		it;
		for (it = node->nodes.begin(); it != node->nodes.end(); it++) {
			XML::XMLNode	*iface = *it;

			if (iface->name == _T("ifilesourcefilter")) {

				CComPtr<IFileSourceFilter>		fsource;
				hr = filter->QueryInterface(IID_IFileSourceFilter, (void**)&fsource);
				if (SUCCEEDED(hr)) {
					// load the file
					CString	source = iface->GetValue(_T("source"));
					hr = fsource->Load((LPCOLESTR)source.GetBuffer(), NULL);
					if (FAILED(hr)) return -1;			// cannot open file
					ret = 0;
				}
			}

		}

		return ret;
	}

	int DisplayGraph::LoadGRF(CString fn)
	{
		IStorage *pStorage = 0;
		if (StgIsStorageFile(fn) != S_OK) return -1;

		HRESULT hr = StgOpenStorage(fn, 0, STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 0, 0, &pStorage);
		if (FAILED(hr)) return hr;

		IPersistStream *pPersistStream = 0;
		hr = gb->QueryInterface(IID_IPersistStream, (void**)&pPersistStream);
		if (SUCCEEDED(hr)) {
			IStream *pStream = 0;
			hr = pStorage->OpenStream(L"ActiveMovieGraph", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
			if (SUCCEEDED(hr)) {
				hr = pPersistStream->Load(pStream);
				pStream->Release();
			}
			pPersistStream->Release();
		}
		pStorage->Release();
		return hr;
	}

	int DisplayGraph::SaveGRF(CString fn)
	{
		const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
		HRESULT hr;
		    
		IStorage *pStorage = NULL;
		hr = StgCreateDocfile(fn, STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &pStorage);
		if (FAILED(hr)) return hr;

		IStream *pStream;
		hr = pStorage->CreateStream(wszStreamName, STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,	0, 0, &pStream);
		if (FAILED(hr)) {
			pStorage->Release();    
			return hr;
		}

		IPersistStream *pPersist = NULL;
		gb->QueryInterface(IID_IPersistStream, (void**)&pPersist);
		hr = pPersist->Save(pStream, TRUE);
		pStream->Release();
		pPersist->Release();
		if (SUCCEEDED(hr)) {
			hr = pStorage->Commit(STGC_DEFAULT);
		}
		pStorage->Release();
		return hr;
	}

	int DisplayGraph::RenderFile(CString fn)
	{
		HRESULT	hr = E_FAIL;

		do {

			hr = gb->RenderFile(fn, NULL);
			if (FAILED(hr)) break;

		} while (0);

		if (FAILED(hr)) {
			gb = NULL;
			mc = NULL;
			return -1;
		}

		return 0;
	}

	Pin *DisplayGraph::FindPin(CString pin_path)
	{
		// find the filter
		CString	filter_name = DSUtil::get_next_token(pin_path, _T("/"));
		Filter	*filter = FindFilter(filter_name);
		if (!filter) return NULL;

		// try to find the pin
		return filter->FindPin(pin_path);
	}

	Filter *DisplayGraph::FindFilter(CString name)
	{
		for (int i=0; i<filters.GetCount(); i++) {
			if (filters[i]->name == name) {
				return filters[i];
			}
		}
		return NULL;
	}

	Filter *DisplayGraph::FindFilter(IBaseFilter *filter)
	{
		// find by filter interface
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filt = filters[i];
			if (filt->filter == filter) {
				return filt;
			}
		}
		return NULL;		
	}

	void DisplayGraph::ZeroTags()
	{
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filt = filters[i];
			filt->tag = 0;
		}
	}

	void DisplayGraph::RemoveUnusedFilters()
	{
		// remove all those, whose TAG is zero
		for (int i=filters.GetCount()-1; i>=0; i--) {
			Filter *filt = filters[i];
			if (filt->tag == 0) {				
				if (callback) callback->OnFilterRemoved(this, filt);
				delete filt;
				filters.RemoveAt(i);
			}
		}
	}

	int DisplayGraph::ConnectPins(Pin *p1, Pin *p2)
	{
		// verify a few conditions first
		if (!p1 || !p2) return -1;								// need 2 pins
		if (p1->connected || p2->connected) return -1;			// 2 free pins
		if (p1->filter == p2->filter) return -1;				// not on the same filter
		if (p1->dir == p2->dir) return -1;						// oposite directions

		// swap order
		if (p1->dir == PINDIR_INPUT) {
			Pin *temp = p1;
			p1 = p2;
			p2 = temp;
		}

		// TODO: DirectConnect / Intelligent Connect
		HRESULT hr = gb->Connect(p1->pin, p2->pin);
		if (FAILED(hr)) return hr;

		RefreshFilters();
		SmartPlacement();

		return 0;
	}

	Pin *DisplayGraph::FindPinByPos(CPoint pt)
	{
		Filter *filter = FindFilterByPos(pt);
		if (!filter) return NULL;
		return filter->FindPinByPos(pt);
	}

	Filter *DisplayGraph::FindFilterByPos(CPoint pt)
	{
		int border = 6;
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			CRect	rc(filter->posx-border, filter->posy-2, 
					   filter->posx + filter->width+border, filter->posy + filter->height+2);
			if (rc.PtInRect(pt)) return filter;
		}
		return NULL;
	}

	void DisplayGraph::RefreshFilters()
	{
		ZeroTags();

		IEnumFilters	*efilt;
		IBaseFilter		*filt;
		ULONG			ff;

		if (!gb) {
			RemoveUnusedFilters();
			return ;
		}

		gb->EnumFilters(&efilt);
		efilt->Reset();
		while (efilt->Next(1, &filt, &ff) == NOERROR) {
			Filter	*filter = FindFilter(filt);
			if (!filter) {
				filter = new Filter(this);
				filter->LoadFromFilter(filt);
				filters.InsertAt(0, filter);
			} else {
				filter->Refresh();
			}

			// mark this one as active...
			if (filter) {
				filter->tag = 1;
			}
			filt->Release();
		}
		efilt->Release();
	
		// kill those inactive
		RemoveUnusedFilters();
		LoadPeers();
		Dirty();

	}

	void DisplayGraph::LoadPeers()
	{
		for (int i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			filter->LoadPeers();
		}
	}


	// kreslenie grafu
	void DisplayGraph::Draw(CDC *dc)
	{
		int i;
		for (i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			filter->Draw(dc);
		}

		// now draw all connections
		for (i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			filter->DrawConnections(dc);
		}
	}

	void DisplayGraph::DrawArrow(CDC *dc, CPoint p1, CPoint p2)
	{
		DoDrawArrow(dc, p1, p2, RGB(0,0,0));
	}

	Pin *DisplayGraph::FindPin(IPin *pin)
	{
		Filter	*filter = FindParentFilter(pin);
		if (!filter) return NULL;
		Pin *ret = filter->FindPin(pin);
		return ret;
	}

	Filter *DisplayGraph::FindParentFilter(IPin *pin)
	{
		Filter	*ret = NULL;
		for (int i=0; i<filters.GetCount(); i++) {
			ret = filters[i];
			if (ret->HasPin(pin)) {
				return ret;
			}
		}
		return NULL;
	}

	void DisplayGraph::DoubleSelected()
	{
		CArray<CLSID>	clsid;
		CArray<CString>	names;
		int				i;

		for (i=0; i<filters.GetCount(); i++) {
			Filter *filter = filters[i];
			if (filter->selected) {
				clsid.Add(filter->clsid);
				names.Add(filter->name);
			}
		}

		// now insert them
		for (i=0; i<clsid.GetCount(); i++) {

			// now create an instance of this filter
			CComPtr<IBaseFilter>	instance;
			HRESULT					hr;
			int						ret;

			hr = CoCreateInstance(clsid[i], NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&instance);
			if (SUCCEEDED(hr)) {
				
				// now check for a few interfaces
				int ret = ConfigureInsertedFilter(instance);
				if (ret < 0) {
					instance = NULL;
				}

				if (instance) {
					// add the filter to graph
					hr = AddFilter(instance, names[i]);
					if (FAILED(hr)) {
						// display error message
					}
				}

				instance = NULL;
			}

		}
	}

	void DisplayGraph::SmartPlacement()
	{
		bins.RemoveAll();

		// do some nice automatic filter placement
		int i;
		for (i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			filter->Refresh();

			// reset placement helpers
			filter->depth = 0;
			filter->posy = 0;
			filter->posx = 0;
		}

		// now calculate from each source filter
		for (i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			filter->LoadPeers();
			if (filter->NumOfDisconnectedPins(PINDIR_INPUT) == filter->input_pins.GetCount() && 
				filter->output_pins.GetCount() > 0) {
				filter->CalculatePlacementChain(0, 8);
			}
		}

		// then align the not connected fitlers
		for (i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			if (filter->depth == 0) {
				filter->CalculatePlacementChain(0, 8);
			}
		}

		// now set proper posX
		for (i=0; i<filters.GetCount(); i++) {
			Filter	*filter = filters[i];
			CPoint	&pt     = bins[filter->depth];
			filter->posx = pt.x;
		}
	}

	//-------------------------------------------------------------------------
	//
	//	Filter class
	//
	//-------------------------------------------------------------------------

	Filter::Filter(DisplayGraph *parent)
	{
		graph = parent;
		params = graph->params;
		name = _T("");
		clsid = CLSID_VideoMixingRenderer9;
		clsid_str = _T("");
		filter = NULL;
		posx = 0;
		posy = 0;
		selected = false;
	}

	Filter::~Filter()
	{
		Release();
	}

	void Filter::Release()
	{
		RemovePins();
		name = _T("");
		if (filter != NULL) {
			filter = NULL;
		}
	}

	void Filter::RemovePins()
	{
		int i=0;
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			delete pin;
		}
		input_pins.RemoveAll();
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			delete pin;
		}
		output_pins.RemoveAll();
	}

	int Filter::NumOfDisconnectedPins(PIN_DIRECTION dir)
	{
		int ret = 0;
		if (dir == PINDIR_INPUT) {
			for (int i=0; i<input_pins.GetCount(); i++) {
				Pin *pin = input_pins[i];
				if (!pin->IsConnected()) ret++;
			}
		} else
		if (dir == PINDIR_OUTPUT) {
			for (int i=0; i<output_pins.GetCount(); i++) {
				Pin *pin = output_pins[i];
				if (!pin->IsConnected()) ret++;
			}
		}
		return ret;
	}

	void Filter::Refresh()
	{
		LoadFromFilter(filter);
	}

	void Filter::DeleteSelectedConnections()
	{
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->selected) {
				pin->Disconnect();
			}
		}
	}

	void Filter::DeleteFilter()
	{
		// now all our connections need to be broken
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->connected) pin->Disconnect();
		}
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			if (pin->connected) pin->Disconnect();
		}

		// we can now try to remove us from the graph
		graph->gb->RemoveFilter(filter);
		selected = false;
	}


	void Filter::LoadFromFilter(IBaseFilter *f)
	{
		if (f != filter) {
			Release();

			// keep a reference
			f->QueryInterface(IID_IBaseFilter, (void**)&filter);
			width = params->min_filter_width;
			height = params->min_filter_height;
		}
		if (!filter || !f) return ;
		
		// get the filter CLSID
		f->GetClassID(&clsid);
		NameGuid(clsid, clsid_str);

		FILTER_INFO	info;
		memset(&info, 0, sizeof(info));
		filter->QueryFilterInfo(&info);

		// load name
		name = CString(info.achName);
		if (name == _T("")) name = _T("(Unnamed filter)");

		// todo: check for IFileSourceFilter
		display_name = name;

		//---------------------------------------------------------------------
		//	Check FileSource & FileSink
		//---------------------------------------------------------------------
		CComPtr<IFileSourceFilter>	fs;
		CComPtr<IFileSinkFilter>	fsink;
		HRESULT						hr;
		CString						url_name = _T("");

		hr = f->QueryInterface(IID_IFileSourceFilter, (void**)&fs);
		if (SUCCEEDED(hr)) {
			LPOLESTR	url_name_ole;
			hr = fs->GetCurFile(&url_name_ole, NULL);
			if (SUCCEEDED(hr)) {
				url_name = CString(url_name_ole);
				CoTaskMemFree(url_name_ole);
			}
			fs = NULL;
		}
		hr = f->QueryInterface(IID_IFileSinkFilter, (void**)&fsink);
		if (SUCCEEDED(hr)) {
			LPOLESTR	url_name_ole;
			hr = fsink->GetCurFile(&url_name_ole, NULL);
			if (SUCCEEDED(hr)) {
				url_name = CString(url_name_ole);
				CoTaskMemFree(url_name_ole);
			}
			fsink = NULL;
		}

		if (url_name != _T("")) {
			CPath		path(url_name);
			int fstart = path.FindFileName();
			if (fstart >= 0) {
				url_name.Delete(0, fstart);
				if (url_name != ""){
					display_name = url_name;
				}
			}
		}

		// now scan for pins
		IEnumPins	*epins;
		IPin		*pin;
		ULONG		ff;
		filter->EnumPins(&epins);
		epins->Reset();
		RemovePins();
		while (epins->Next(1, &pin, &ff) == NOERROR) {
			PIN_DIRECTION	dir;
			pin->QueryDirection(&dir);
			LoadPin(pin, dir);
			pin->Release();
		}
		epins->Release();

		connected = false;
		int i;
		for (i=0; i<input_pins.GetCount(); i++) {
			if (input_pins[i]->connected) {
				connected = true;
				break;
			}
		}
		if (!connected) {
			for (i=0; i<output_pins.GetCount(); i++) {
				if (output_pins[i]->connected) {
					connected = true;
					break;
				}
			}
		}

		// this will be okay for now...
		filter_type = Filter::FILTER_STANDARD;

		// if it's a DMO Wrapper we'll identify it as DMO
		if (clsid == CLSID_DMOWrapperFilter) {
			filter_type = Filter::FILTER_DMO;
		} else
		if (clsid == DSUtil::CLSID_KSProxy) {
			filter_type = Filter::FILTER_WDM;
		}

		//---------------------------------------------------------------------
		// calculate size
		//---------------------------------------------------------------------		
		graph->dc->SelectObject(&params->font_filter);
		CSize	size = graph->dc->GetTextExtent(display_name);
		size.cx += 2 * 24;
		width = (size.cx + 15) &~ 0x0f;		if (width < params->min_filter_width) width = params->min_filter_width;
		height = (size.cy + 15) &~ 0x0f;	if (height < params->min_filter_height) height = params->min_filter_height;

		int		maxpins = max(input_pins.GetCount(), output_pins.GetCount());
		int		minsize = (((1 + maxpins)*params->pin_spacing) + (params->pin_spacing/2)) &~ 0x0f;
		if (height < minsize) height = minsize;

		// we don't need it anymore
		if (info.pGraph) info.pGraph->Release();
	}

	bool Filter::HasPin(IPin *pin)
	{
		return (FindPin(pin) != NULL);
	}

	Pin *Filter::FindPin(IPin *pin)
	{
		int i;
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *p = input_pins[i];
			if (p->pin == pin) return p;
		}
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *p = output_pins[i];
			if (p->pin == pin) return p;
		}
		return NULL;
	}

	void Filter::LoadPin(IPin *pin, PIN_DIRECTION dir)
	{
		Pin	*npin = new Pin(this);
		if (npin->Load(pin) < 0) {
			delete npin;
			return ;
		}
	
		// to the proper pool
		if (dir == PINDIR_INPUT) input_pins.Add(npin); else 
		if (dir == PINDIR_OUTPUT) output_pins.Add(npin);
	}

	void Filter::LoadPeers()
	{
		for (int i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			pin->LoadPeer();
		}
	}


	// Helpers
	bool Filter::IsSource()
	{
		if (input_pins.GetCount() == 0 && output_pins.GetCount() > 0) return true;
		return false;
	}

	bool Filter::IsRenderer()
	{
		if (input_pins.GetCount() > 0 && output_pins.GetCount() == 0) return true;
		return false;
	}

	void DoDrawArrow(CDC *dc, CPoint p1, CPoint p2, DWORD color)
	{
		CPen	pen(PS_SOLID, 1, color);
		CBrush	brush(color);

		// direction vector
		double	vx, vy;
		if (p1 == p2) { vx = 0;	vy = 1.0; } else {
			vx = (p2.x - p1.x);
			vy = (p2.y - p1.y);
		}

		// vector length
		double	vs = sqrt(vx*vx + vy*vy); vx /= vs;	vy /= vs;
		double	arrow_size = 8;

		// find the midpoint
		double	mx, my;
		mx = p2.x - arrow_size * vx;
		my = p2.y - arrow_size * vy;

		// find the arrow points
		double	tx, ty;
		tx = vy;
		ty = -vx;

		CPoint	a1(mx + (tx*arrow_size/2.4), my + (ty*arrow_size/2.4));
		CPoint	a2(mx - (tx*arrow_size/2.4), my - (ty*arrow_size/2.4));

		dc->SelectObject(&pen);
		dc->SelectObject(&brush);

		POINT	pts[3] = { a1, a2, p2 };
		dc->Polygon((const POINT*)&pts, 3);
		dc->MoveTo(p1);
		dc->LineTo(p2);
	}

	void Filter::DrawConnections(CDC *dc)
	{		
		// render directed arrows for all connected output pins
		for (int i=0; i<output_pins.GetCount(); i++) {
			Pin		*pin = output_pins[i];
			Pin		*peer = pin->peer;
			DWORD	color = RGB(0,0,0);

			if (pin && peer) {
				CPoint	pt1, pt2;
				pin->GetCenterPoint(&pt1);
				peer->GetCenterPoint(&pt2);
				if (pin->selected) color = params->select_color;
			
				DoDrawArrow(dc, pt1, pt2, color);
			}
		}
	}

	void Filter::Draw(CDC *dc)
	{
		DWORD	back_color = params->filter_type_colors[0];
		if (connected) {
			back_color = params->filter_type_colors[filter_type];
		}

		CPen	pen_light(PS_SOLID, 1, params->color_filter_border_light);
		CPen	pen_dark(PS_SOLID, 1, params->color_filter_border_dark);
		CPen	pen_back(PS_SOLID, 1, back_color);
		CBrush	brush_back(back_color);
		dc->SetBkMode(TRANSPARENT);

		CPen	*prev_pen   = dc->SelectObject(&pen_back);
		CBrush	*prev_brush = dc->SelectObject(&brush_back);
		CFont	*prev_font	= dc->SelectObject(&params->font_filter);

		//---------------------------------------------------------------------
		// draw the selection frame
		//---------------------------------------------------------------------
		if (selected) {
			CPen	sel_pen(PS_SOLID, 1, params->select_color);
			CBrush	sel_brush(params->select_color);
			dc->SelectObject(&sel_pen);
			dc->SelectObject(&sel_brush);
			dc->Rectangle(posx-2, posy-2, posx+width+2, posy+height+2);
		}

		//---------------------------------------------------------------------
		// draw the filter background
		//---------------------------------------------------------------------
		dc->SelectObject(&pen_back);
		dc->SelectObject(&brush_back);
		dc->Rectangle(posx, posy, posx+width, posy+height);

		//---------------------------------------------------------------------
		// draw the 3d edge
		//---------------------------------------------------------------------
		dc->SelectObject(&pen_light);
		dc->MoveTo(posx, posy+height-2);		dc->LineTo(posx, posy);		dc->LineTo(posx+width-1, posy);
		dc->MoveTo(posx+1, posy+height-3);	dc->LineTo(posx+1, posy+1);	dc->LineTo(posx+width-2, posy+1);
		dc->MoveTo(posx+width-4, posy+4);	
		dc->LineTo(posx+width-4, posy+height-4);	dc->LineTo(posx+3, posy+height-4);

		dc->SelectObject(&pen_dark);
		dc->MoveTo(posx+0, posy+height-1);	dc->LineTo(posx+width-1, posy+height-1);	dc->LineTo(posx+width-1, posy-1);
		dc->MoveTo(posx+1, posy+height-2);	dc->LineTo(posx+width-2, posy+height-2);	dc->LineTo(posx+width-2, posy);
		dc->MoveTo(posx+3, posy+height-4);	
		dc->LineTo(posx+3, posy+3);	dc->LineTo(posx+width-3, posy+3);

		//---------------------------------------------------------------------
		// draw the font
		//---------------------------------------------------------------------
		CRect	rc(posx, posy+8, posx+width, posy+height);
		dc->DrawText(display_name, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		dc->SelectObject(prev_pen);
		dc->SelectObject(prev_brush);
		dc->SelectObject(prev_font);

		//---------------------------------------------------------------------
		// draw the pins
		//---------------------------------------------------------------------
		int i, x, y;
		x = posx-5;
		y = posy + params->pin_spacing;
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			pin->Draw(dc, true, x, y);
			y += params->pin_spacing;
		}
		x = posx+width-2;
		y = posy + params->pin_spacing;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			pin->Draw(dc, false, x, y);
			y += params->pin_spacing;
		}


	}

	void Filter::CalculatePlacementChain(int new_depth, int x)
	{
		if (new_depth > graph->bins.GetCount()) {
			// this is an error case !!
			return ;
		} else
		if (new_depth == graph->bins.GetCount()) {
			// add one more
			CPoint	pt;
			pt.x = x;
			pt.y = 8;
			graph->bins.Add(pt);
			pt.x = (x+width+40+7) &~ 0x07;
			graph->bins.Add(pt);
		} else
		if (new_depth == graph->bins.GetCount()-1) {
			// check the next bin X position
			CPoint	pt;
			int		newx = (x+width+40+7)&~ 0x07;
			pt.x = newx;
			pt.y = 8;
			graph->bins.Add(pt);
		}

		CPoint		&pt = graph->bins[new_depth];

		// we distribute new values, if they are larger
		if (new_depth == 0 || new_depth > depth || x > pt.x) {
			depth = new_depth;

			if (x > pt.x) {
				pt.x = x;

				if (x > pt.x) {
					int dif = x - pt.x;
					// move all following bins
					for (int i=new_depth; i<graph->bins.GetCount(); i++) {
						CPoint	&p = graph->bins[i];
						p.x += dif;
					}
				}
			}
			int	newx = ((pt.x + width + 40) + 7) &~ 0x07;

			if (posy == 0)	{
				// next row
				posy = pt.y;
				posy = (posy + 7) &~ 0x07;
				pt.y += height + 30;
				pt.y = (pt.y + 7) &~ 0x07;
			}

			// find downstream filters
			for (int i=0; i<output_pins.GetCount(); i++) {
				Pin *pin = output_pins[i];
				IPin *peer_pin = NULL;
				if (pin && pin->pin) pin->pin->ConnectedTo(&peer_pin);

				// find parent of the downstream filter
				if (peer_pin) {
					Filter	*down_filter = graph->FindParentFilter(peer_pin);
					if (down_filter) {
						down_filter->CalculatePlacementChain(new_depth + 1, newx);
					}
					peer_pin->Release();
				}
			}
		}
	}

	void Filter::SelectConnection(UINT flags, CPoint pt)
	{
		// we check our output pins and test for hit
		for (int i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			Pin *peer = pin->peer;
			if (!pin->connected) continue;

			if (pin && peer) {
				CPoint	pt1, pt2;
				pin->GetCenterPoint(&pt1);
				peer->GetCenterPoint(&pt2);

				bool hit = LineHit(pt1, pt2, pt);
				if (hit) {
					
					// testing ...
					pin->Select(true);
				} else {
					pin->Select(false);
				}
			}

		}
	}

	void Filter::BeginDrag()
	{
		start_drag_pos.x = posx;
		start_drag_pos.y = posy;
	}

	void Filter::VerifyDrag(int *deltax, int *deltay)
	{
		// verify so we don't go outside the area
		if (start_drag_pos.x + (*deltax) < 8) {
			*deltax = 8 - start_drag_pos.x;
		}
		if (start_drag_pos.y + (*deltay) < 8) {
			*deltay = 8 - start_drag_pos.y;
		}
	}

	void Filter::Select(bool select)
	{
		selected = select;

		// select our connections as well
		int i;
		for (i=0; i<input_pins.GetCount(); i++) input_pins[i]->Select(select);
		for (i=0; i<output_pins.GetCount(); i++) output_pins[i]->Select(select);
	}

	Pin *Filter::FindPin(CString name)
	{
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			if (output_pins[i]->name == name) return output_pins[i];
		}
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			if (pin->name == name) return pin;
		}
		return NULL;
	}

	Pin *Filter::FindPinByPos(CPoint p, bool not_connected)
	{
		int i;
		for (i=0; i<output_pins.GetCount(); i++) {
			Pin *pin = output_pins[i];
			if (pin->connected && not_connected) continue;

			CPoint	cp;
			pin->GetCenterPoint(&cp);

			float	dist = (float)sqrt((float)((p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y)));
			if (dist < 8.0) return pin;
		}
		for (i=0; i<input_pins.GetCount(); i++) {
			Pin *pin = input_pins[i];
			if (pin->connected && not_connected) continue;

			CPoint	cp;
			pin->GetCenterPoint(&cp);

			float	dist = (float)sqrt((float)((p.x-cp.x)*(p.x-cp.x) + (p.y-cp.y)*(p.y-cp.y)));
			if (dist < 8.0) return pin;
		}
		return NULL;
	}

	//-------------------------------------------------------------------------
	//
	//	Pin class
	//
	//-------------------------------------------------------------------------
	Pin::Pin(Filter *parent) :
		name(_T("")),
		filter(parent),
		id(_T(""))
	{
		params = parent->params;
		pin = NULL;
		peer = NULL;
		selected = false;
	}

	Pin::~Pin()
	{
		pin = NULL;
	}

	int Pin::Disconnect()
	{
		if (!connected || !peer) return 0;

		// we need to disconnect both pins
		HRESULT	hr;
		hr = filter->graph->gb->Disconnect(peer->pin);
		if (FAILED(hr)) return -1;
		hr = filter->graph->gb->Disconnect(pin);
		if (FAILED(hr)) return -1;

		// clear variables
		peer->peer = NULL;
		peer->selected = false;
		peer->connected = false;
		peer = NULL;
		connected = false;
		selected = false;

		// we're okay
		return 0;
	}

	void Pin::Select(bool select)
	{
		selected = select;
		if (peer) {
			peer->selected = select;
		}
	}

	void Pin::GetCenterPoint(CPoint *pt)
	{
		// find out our index
		int	index = 0;
		if (dir == PINDIR_INPUT) {
			for (int i=0; i<filter->input_pins.GetCount(); i++) {
				if (filter->input_pins[i] == this) {
					index = i;
					break;
				}
			}
		} else {
			for (int i=0; i<filter->output_pins.GetCount(); i++) {
				if (filter->output_pins[i] == this) {
					index = i;
					break;
				}
			}
		}

		// calculate X and Y
		pt->y = filter->posy + (1+index)*params->pin_spacing + 4;
		if (dir == PINDIR_INPUT) {
			pt->x = filter->posx - 1;
		} else {
			pt->x = filter->posx + filter->width;
		}
	}

	int Pin::Load(IPin *pin)
	{
		if (this->pin) {
			this->pin = NULL;
		}
		if (!pin) return -1;

		// keep a reference
		pin->QueryInterface(IID_IPin, (void**)&this->pin);

		IPin *peerpin = NULL;
		pin->ConnectedTo(&peerpin);
		connected = (peerpin != NULL);
		if (peerpin) peerpin->Release();

		PIN_INFO	info;
		memset(&info, 0, sizeof(info));
		pin->QueryPinInfo(&info);
		dir = info.dir;

		// find out name
		name = CString(info.achName);
		if (info.pFilter) info.pFilter->Release();

		return 0;
	}

	void Pin::LoadPeer()
	{
		IPin *p = NULL;
		pin->ConnectedTo(&p);
		if (p) {
			// find the peer pin
			peer = filter->graph->FindPin(p);

			// backward link
			if (peer) {
				peer->peer = this;
			}

			p->Release();
		} else {
			peer = NULL;
		}
	}

	bool Pin::IsConnected()
	{
		if (!pin) return false;
		IPin *peer = NULL;
		if (SUCCEEDED(pin->ConnectedTo(&peer))) {
			if (peer) {
				peer->Release();
				return true;
			}
		}
		return false;
	}

	void Pin::Draw(CDC *dc, bool input, int x, int y)
	{
		CPen	pen_light(PS_SOLID, 1, params->color_filter_border_light);
		CPen	pen_dark(PS_SOLID, 1, params->color_filter_border_dark);
		CPen	pen_back(PS_SOLID, 1, params->filter_type_colors[0]);
		CBrush	brush_back(params->filter_type_colors[0]);

		int		pinsize = 5;
		dc->SelectObject(&params->font_pin);
		CSize	size = dc->GetTextExtent(name);

		if (input) {
			dc->SelectObject(&pen_dark);
			dc->MoveTo(x+1, y+2+pinsize);		dc->LineTo(x+1+pinsize+1, y+2+pinsize);
			dc->MoveTo(x,   y+2+pinsize+1);		dc->LineTo(x+1+pinsize,	y+2+pinsize+1);

			dc->SelectObject(&pen_light);
			dc->MoveTo(x+1, y+2+pinsize-1);		dc->LineTo(x+1, y+1);	dc->LineTo(x+pinsize, y+1);
			dc->MoveTo(x,   y+2+pinsize);		dc->LineTo(x, y);		dc->LineTo(x+pinsize, y);

			dc->SelectObject(&pen_back);
			dc->SelectObject(&brush_back);
			dc->Rectangle(x+2, y+2, x+2+pinsize, y+2+pinsize);

			// dot in the middle
			dc->SelectObject(&pen_dark);
			dc->SetPixel(x+2+pinsize/2, y+pinsize/2 + 2, params->color_filter_border_dark);

			// pin name
			CRect	rc(x+pinsize+6, y - 10, x+pinsize+6+size.cx, y + 4+pinsize + 10);
			dc->DrawText(name, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		} else {
			dc->SelectObject(&pen_light);
			dc->MoveTo(x+1, y);		dc->LineTo(x+1 + pinsize, y);
			dc->MoveTo(x, y+1);		dc->LineTo(x+pinsize,	  y+1);
			dc->SelectObject(&pen_dark);
			dc->MoveTo(x, y+2+pinsize+1);		
			dc->LineTo(x+1+pinsize, y+2+pinsize+1);
			dc->LineTo(x+1+pinsize, y-1);
			dc->MoveTo(x, y+2+pinsize);			
			dc->LineTo(x+1+pinsize-1, y+2+pinsize);
			dc->LineTo(x+1+pinsize-1, y);

			dc->SelectObject(&pen_back);
			dc->SelectObject(&brush_back);
			dc->Rectangle(x, y+2, x+pinsize, y+2+pinsize);

			// dot in the middle
			dc->SelectObject(&pen_dark);
			dc->SetPixel(x+pinsize/2, y+pinsize/2 + 2, params->color_filter_border_dark);

			// pin name
			CRect	rc(x-4-size.cx, y - 10, x-4, y + 4+pinsize + 10);
			dc->DrawText(name, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

	}

	typedef struct {
		float x;
		float y;
	} FlPoint;

	float Magnitude(FlPoint *p1, FlPoint *p2)
	{
		FlPoint v;
		v.x = (p2->x - p1->x);
		v.y = (p2->y - p1->y);
		return (float)sqrt(v.x*v.x + v.y*v.y);
	}

	int DistancePointLine(FlPoint *p1, FlPoint *p2, FlPoint *hit, float *distance)
	{
		float mag = Magnitude(p1, p2);
		float u;
		u = ( ( (hit->x - p1->x) * (p2->x - p1->x) ) +
			  ( (hit->y - p1->y) * (p2->y - p1->y) ) ) /
		    (mag*mag);
		if (u < 0.0 || u > 1.0) return -1;

		FlPoint intersect;
		intersect.x = p1->x + u*(p2->x - p1->x);
		intersect.y = p1->y + u*(p2->y - p1->y);
		*distance = Magnitude(hit, &intersect);
		return 0;
	}

	bool LineHit(CPoint p1, CPoint p2, CPoint hit)
	{
		// we calculate the distance of the "hit" point
		// from the line defined by p1,p2
		// if it's close enough we call it a hit
		if (p1 == p2) return false;

		FlPoint f1, f2, h;
		f1.x = p1.x;	f1.y = p1.y;
		f2.x = p2.x;	f2.y = p2.y;
		h.x = hit.x;	h.y = hit.y;

		float dist;
		if (DistancePointLine(&f1, &f2, &h, &dist) < 0) return false;
		if (dist < 3.5) return true;
		return false;
	}


	//-------------------------------------------------------------------------
	//
	//	RenderParameters class
	//
	//-------------------------------------------------------------------------

	RenderParameters::RenderParameters()
	{
		color_back = RGB(192, 192, 192);		// default background color
		select_color = RGB(0,0,255);

		// filter colors
		color_filter_border_light = RGB(255, 255, 255);
		color_filter_border_dark = RGB(128, 128, 128);

		filter_type_colors[Filter::FILTER_UNKNOWN] = RGB(192,192,192);
		filter_type_colors[Filter::FILTER_STANDARD] = RGB(192,192,255);
		filter_type_colors[Filter::FILTER_WDM] = RGB(255,128,0);
		filter_type_colors[Filter::FILTER_DMO] = RGB(0,192,64);

		// default size at 100%
		def_min_width = 92;
		def_min_height = 86;
		def_pin_spacing = 27;
		def_filter_text_size = 10;
		def_pin_text_size = 7;

		Zoom(1.0);
	}

	RenderParameters::~RenderParameters()
	{
	}

	void RenderParameters::Zoom(int z)
	{
		zoom = z;

		min_filter_width = (((int)(z * def_min_width / 100.0))); // &~ 0x08;
		min_filter_height = (((int)(z * def_min_height / 100.0))); // &~ 0x08;
		pin_spacing = (int)(z * def_pin_spacing / 100.0);

		if (font_filter.m_hObject != 0) { font_filter.DeleteObject(); }
		if (font_pin.m_hObject != 0) { font_pin.DeleteObject(); }

		int size = 5 + (5.0*z / 100.0);
		MakeFont(font_filter, _T("Arial"), size, false, false); 
		size = 5 + (2.0*z / 100.0);
		MakeFont(font_pin, _T("Arial"), size, false, false);

	}




};

