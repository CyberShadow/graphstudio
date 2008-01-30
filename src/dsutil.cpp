//-----------------------------------------------------------------------------
//
//	Transkoder/Broadcaster
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <atlbase.h>
#include <atlpath.h>

namespace DSUtil
{

	CString get_next_token(CString &str, CString separator)
	{
		CString ret;
		int pos = str.Find(separator);
		if (pos < 0) {
			ret = str;
			ret = ret.Trim();
			str = _T("");
			return ret;
		}

		// mame ho
		ret = str.Left(pos);
		ret = ret.Trim();
		str.Delete(0, pos + separator.GetLength());
		str = str.Trim();
		return ret;
	}

	//-------------------------------------------------------------------------
	//
	//	URI class
	//
	//-------------------------------------------------------------------------

	URI::URI() :
		protocol(_T("")),
		host(_T("")),
		request_url(_T("")),
		complete_request(_T("")),
		port(80)
	{
	}

	URI::URI(const URI &u) :
		protocol(u.protocol),
		host(u.host),
		request_url(u.request_url),
		complete_request(u.complete_request),
		port(u.port)
	{
	}

	URI::URI(CString url)
	{
		Parse(url);
	}

	URI::~URI()
	{
		// zatial nic
	}

	URI &URI::operator =(const URI &u)
	{
		protocol = u.protocol;
		host = u.host;
		request_url = u.request_url;
		complete_request = u.complete_request;
		port = u.port;
		return *this;
	}

	URI &URI::operator =(CString url)
	{
		Parse(url);
		return *this;
	}

	int URI::Parse(CString url)
	{
		// protocol://host:port/request_url
		protocol = _T("http");
		host = _T("");
		request_url = _T("");

		int pos;
		pos = url.Find(_T("://"));
		if (pos > 0) {
			protocol = url.Left(pos);
			url.Delete(0, pos+3);
		}
		port = 80;		// map protocol->port

		// najdeme lomitko
		pos = url.Find(_T("/"));
		if (pos < 0) {
			request_url = _T("/");
			host = url;
		} else {
			host = url.Left(pos);
			url.Delete(0, pos);
			request_url = url;
		}

		// parsneme host a port
		pos = host.Find(_T(":"));
		if (pos > 0) {
			CString temp_host = host;
			host = temp_host.Left(pos);
			temp_host.Delete(0, pos+1);

			temp_host.Trim();
			_stscanf_s(temp_host, _T("%d"), &port);
		}

		// testy spravnosti
		if (host == _T("") || request_url == _T("")) return -1;

		// spravime kompletny request
		complete_request.Format(_T("%s://%s:%d%s"), protocol, host, port, request_url);
		return 0;
	}

	Pin::Pin() :
		filter(NULL),
		pin(NULL),
		name(_T("")),
		dir(PINDIR_INPUT)
	{
	}

	Pin::Pin(const Pin &p) :
		dir(p.dir),
		name(p.name)
	{
		filter = p.filter;	if (filter) filter->AddRef();
		pin = p.pin;		if (pin) pin->AddRef();
	}

	Pin::~Pin()
	{
		if (filter) filter->Release();
		if (pin) pin->Release();
	}

	Pin &Pin::operator =(const Pin &p)
	{
		dir = p.dir;
		name = p.name;
		if (filter) filter->Release();
		filter = p.filter;
		if (filter) filter->AddRef();

		if (pin) pin->Release();
		pin = p.pin;
		if (pin) pin->AddRef();

		return *this;
	}

	FilterTemplate::FilterTemplate() :
		name(_T("")),
		file(_T("")),
		file_exists(false),
		clsid(GUID_NULL),
		moniker(NULL),
		version(0),
		merit(0)
	{
	}

	FilterTemplate::FilterTemplate(const FilterTemplate &ft) :
		name(ft.name),
		file(ft.file),
		file_exists(ft.file_exists),
		clsid(ft.clsid),
		version(ft.version),
		merit(ft.merit),
		moniker(NULL)
	{
		if (ft.moniker) {
			moniker = ft.moniker;
			moniker->AddRef();
		}
	}

	FilterTemplate::~FilterTemplate()
	{
		if (moniker) {
			moniker->Release();
			moniker = NULL;
		}
	}

	FilterTemplate &FilterTemplate::operator =(const FilterTemplate &ft)
	{
		if (moniker) { moniker->Release(); moniker = NULL; }
		moniker = ft.moniker;
		if (moniker) moniker->AddRef();

		name = ft.name;
		file = ft.file;
		file_exists = ft.file_exists;
		clsid = ft.clsid;
		version = ft.version;
		merit = ft.merit;
		return *this;
	}

	HRESULT FilterTemplate::FindFilename()
	{
		// HKEY_CLASSES_ROOT\CLSID\{07C9CB2C-F51C-47EA-B551-7DA02541D586}
		LPOLESTR	str;
		StringFromCLSID(clsid, &str);
		CString		str_clsid(str);
		CString		key_name;
		if (str) CoTaskMemFree(str);

		key_name.Format(_T("CLSID\\%s\\InprocServer32"), str_clsid);
		CRegKey		key;
		if (key.Open(HKEY_CLASSES_ROOT, key_name, KEY_READ) != ERROR_SUCCESS) { 
			file_exists = false;
			return -1;
		}

		TCHAR		temp[4*1024];
		ULONG		chars=4*1024;

		key.QueryStringValue(_T(""), temp, &chars);
		temp[chars]=0;
		file = temp;

		key.Close();

		// now we try this file
		CPath	path(file);
		if (path.FileExists()) {
			file_exists = true;
		} else {
			file_exists = true; //false;
		}

		return NOERROR;
	}

	// vytvorenie instancie
	HRESULT FilterTemplate::CreateInstance(IBaseFilter **filter)
	{
		HRESULT hr;
		if (!filter) return E_POINTER;

		// ak mame moniker, ideme cez neho
		if (moniker) {
			return moniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)filter);
		}

		// inak skusime klasicky
		return CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)filter);
	}

	FilterCategory::FilterCategory() :
		name(_T("")),
		clsid(GUID_NULL)
	{
	}

	FilterCategory::FilterCategory(const FilterCategory &fc) :
		name(fc.name),
		clsid(fc.clsid)
	{
	}

	FilterCategory::~FilterCategory()
	{
	}

	FilterCategory &FilterCategory::operator =(const FilterCategory &fc)
	{
		name = fc.name;
		clsid = fc.clsid;
		return *this;
	}


	FilterCategories::FilterCategories()
	{
		categories.RemoveAll();
		Enumerate();
	}

	FilterCategories::~FilterCategories()
	{
		categories.RemoveAll();
	}

	int FilterCategories::Enumerate()
	{
		// ideme nato
		ICreateDevEnum		*sys_dev_enum = NULL;
		IEnumMoniker		*enum_moniker = NULL;
		IMoniker			*moniker = NULL;
		IPropertyBag		*propbag = NULL;
		ULONG				f;
		HRESULT				hr;
		int					ret = -1;

		do {
			hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sys_dev_enum);
			if (FAILED(hr)) break;

			// ideme enumerovat clasy
			hr = sys_dev_enum->CreateClassEnumerator(CLSID_ActiveMovieCategories, &enum_moniker, 0);
			if (FAILED(hr)) break;

			enum_moniker->Reset();
			while (enum_moniker->Next(1, &moniker, &f) == NOERROR) {
				hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
				if (SUCCEEDED(hr)) {
					VARIANT				var;
					FilterCategory		category;

					VariantInit(&var);
					hr = propbag->Read(L"FriendlyName", &var, 0);
					if (SUCCEEDED(hr)) {
						category.name = CString(var.bstrVal);
					}
					VariantClear(&var);

					VariantInit(&var);
					hr = propbag->Read(L"CLSID", &var, 0);
					if (SUCCEEDED(hr)) {
						if (SUCCEEDED(CLSIDFromString(var.bstrVal, &category.clsid))) {
							// mame novu kategoriu
							categories.Add(category);
						}
					}
					VariantClear(&var);


					propbag->Release();
					propbag = NULL;
				}
				moniker->Release();
				moniker = NULL;
			}

			// sme okej
			ret = 0;
		} while (0);

	label_done:
		if (propbag) propbag->Release();
		if (moniker) moniker->Release();
		if (enum_moniker) enum_moniker->Release();
		if (sys_dev_enum) sys_dev_enum->Release();

		return ret;
	}


	FilterTemplates::FilterTemplates()
	{
		filters.RemoveAll();
	}

	FilterTemplates::~FilterTemplates()
	{
		filters.RemoveAll();
	}

	int FilterTemplates::Enumerate(FilterCategory &cat)
	{
		return Enumerate(cat.clsid);
	}

	int FilterTemplates::Find(CString name, FilterTemplate *filter)
	{
		if (!filter) return -1;
		for (int i=0; i<filters.GetCount(); i++) {
			if (name == filters[i].name) {
				*filter = filters[i];
				return 0;
			}
		}

		// nemame nic
		return -1;
	}

	int FilterTemplates::Find(GUID clsid, FilterTemplate *filter)
	{
		if (!filter) return -1;
		for (int i=0; i<filters.GetCount(); i++) {
			if (clsid == filters[i].clsid) {
				*filter = filters[i];
				return 0;
			}
		}

		// nemame nic
		return -1;
	}

	// vytvaranie
	HRESULT FilterTemplates::CreateInstance(CString name, IBaseFilter **filter)
	{
		FilterTemplate	ft;
		if (Find(name, &ft) >= 0) {
			return ft.CreateInstance(filter);
		}
		return E_FAIL;
	}

	HRESULT FilterTemplates::CreateInstance(GUID clsid, IBaseFilter **filter)
	{
		FilterTemplate	ft;
		if (Find(clsid, &ft) >= 0) {
			return ft.CreateInstance(filter);
		}
		return E_FAIL;
	}

	int FilterTemplates::Enumerate(GUID clsid)
	{
		filters.RemoveAll();

		// ideme nato
		ICreateDevEnum		*sys_dev_enum = NULL;
		IEnumMoniker		*enum_moniker = NULL;
		IMoniker			*moniker = NULL;
		IPropertyBag		*propbag = NULL;
		ULONG				f;
		HRESULT				hr;
		int					ret = -1;

		do {
			hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&sys_dev_enum);
			if (FAILED(hr)) break;

			// ideme enumerovat filtre
			hr = sys_dev_enum->CreateClassEnumerator(clsid, &enum_moniker, 0);
			if (hr != NOERROR) break;

			enum_moniker->Reset();
			while (enum_moniker->Next(1, &moniker, &f) == NOERROR) {
				hr = moniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&propbag);
				if (SUCCEEDED(hr)) {
					VARIANT				var;
					FilterTemplate		filter;

					VariantInit(&var);
					hr = propbag->Read(L"FriendlyName", &var, 0);
					if (SUCCEEDED(hr)) {
						filter.name = CString(var.bstrVal);
					}
					VariantClear(&var);

					VariantInit(&var);
					hr = propbag->Read(L"FilterData", &var, 0);
					if (SUCCEEDED(hr)) {
						SAFEARRAY	*ar = var.parray;
						int	size = ar->rgsabound[0].cElements;

						// load merit and version
						if (size >= 8) {
							DWORD	*ptr = (DWORD*)ar->pvData;
							filter.version = ptr[0];
							filter.merit = ptr[1];
						}
					}
					VariantClear(&var);

					VariantInit(&var);
					hr = propbag->Read(L"CLSID", &var, 0);
					if (SUCCEEDED(hr)) {
						if (SUCCEEDED(CLSIDFromString(var.bstrVal, &filter.clsid))) {
							// mame novy filter
							filter.moniker = moniker;
							filter.moniker->AddRef();
							filter.FindFilename();

							filters.Add(filter);
						}
					}
					VariantClear(&var);

					propbag->Release();
					propbag = NULL;
				}
				moniker->Release();
				moniker = NULL;
			}

			// sme okej
			ret = 0;
		} while (0);

	label_done:
		if (propbag) propbag->Release();
		if (moniker) moniker->Release();
		if (enum_moniker) enum_moniker->Release();
		if (sys_dev_enum) sys_dev_enum->Release();

		return ret;
	}




	HRESULT DisplayPropertyPage(IBaseFilter *filter, HWND parent)
	{
		if (!filter) return E_POINTER;

		ISpecifyPropertyPages *pProp = NULL;
		HRESULT hr = filter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
		if (SUCCEEDED(hr)) {
			// Get the filter's name and IUnknown pointer.
			FILTER_INFO FilterInfo;
			hr = filter->QueryFilterInfo(&FilterInfo); 
			IUnknown *pFilterUnk;
			filter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

			// Show the page. 
			CAUUID caGUID;
			pProp->GetPages(&caGUID);
			pProp->Release();

			OleCreatePropertyFrame(
				parent,                 // Parent window
				0, 0,                   // Reserved
				FilterInfo.achName,     // Caption for the dialog box
				1,                      // Number of objects (just the filter)
				&pFilterUnk,            // Array of object pointers. 
				caGUID.cElems,          // Number of property pages
				caGUID.pElems,          // Array of property page CLSIDs
				0,                      // Locale identifier
				0, NULL                 // Reserved
			);

			// Clean up.
			pFilterUnk->Release();
			if (FilterInfo.pGraph) FilterInfo.pGraph->Release(); 
			CoTaskMemFree(caGUID.pElems);

			return NOERROR;
		}
		return E_FAIL;
	}

	HRESULT EnumMediaTypes(IPin *pin, MediaTypes &types)
	{
		types.RemoveAll();
		if (!pin) return E_POINTER;

		IEnumMediaTypes	*emt;
		HRESULT			hr;
		AM_MEDIA_TYPE	*pmt;
		ULONG			f;

		hr = pin->EnumMediaTypes(&emt);
		if (FAILED(hr)) return hr;

		emt->Reset();
		while (emt->Next(1, &pmt, &f) == NOERROR) {
			CMediaType		mt(*pmt);
			types.Add(mt);
			DeleteMediaType(pmt);
		}

		emt->Release();
		return NOERROR;
	}


	// enumeracie pinov
	HRESULT EnumPins(IBaseFilter *filter, PinArray &pins, int flags)
	{
		pins.RemoveAll();
		if (!filter) return NOERROR;

		IEnumPins	*epins;
		IPin		*pin;
		ULONG		f;
		HRESULT		hr;

		hr = filter->EnumPins(&epins);
		if (FAILED(hr)) return hr;

		epins->Reset();
		while (epins->Next(1, &pin, &f) == NOERROR) {
			PIN_DIRECTION	dir;
			PIN_INFO		info;
			Pin				npin;

			pin->QueryDirection(&dir);

			// pozreme, ci zodpoveda flagom
			if (dir == PINDIR_INPUT && (!(flags&Pin::PIN_FLAG_INPUT))) goto label_next;
			if (dir == PINDIR_OUTPUT && (!(flags&Pin::PIN_FLAG_OUTPUT))) goto label_next;

			IPin	*other_pin = NULL;
			bool	is_connected;
			pin->ConnectedTo(&other_pin);
			is_connected = (other_pin == NULL ? false : true);
			if (other_pin) other_pin->Release();

			// zodpoveda ?
			if (is_connected && (!(flags&Pin::PIN_FLAG_CONNECTED))) goto label_next;
			if (!is_connected && (!(flags&Pin::PIN_FLAG_NOT_CONNECTED))) goto label_next;

			// pin info
			pin->QueryPinInfo(&info);

			// naplnime info
			npin.name = CString(info.achName);
			npin.filter = info.pFilter;				// nereleasujeme, lebo npin drzi referenciu
			npin.pin = pin;	pin->AddRef();
			npin.dir = dir;
			pins.Add(npin);

		label_next:
			pin->Release();
		}
		epins->Release();		
		return NOERROR;
	}

	HRESULT ConnectFilters(IGraphBuilder *gb, IBaseFilter *output, IBaseFilter *input, bool direct)
	{
		PinArray		opins;
		PinArray		ipins;
		HRESULT			hr;

		EnumPins(output, opins, Pin::PIN_FLAG_OUTPUT | Pin::PIN_FLAG_NOT_CONNECTED);
		EnumPins(input, ipins, Pin::PIN_FLAG_INPUT | Pin::PIN_FLAG_NOT_CONNECTED);

		// a teraz skusame
		for (int i=0; i<opins.GetCount(); i++) {
			for (int j=0; j<ipins.GetCount(); j++) {

				if (direct) {
					hr = gb->ConnectDirect(opins[i].pin, ipins[j].pin, NULL);
				} else {
					hr = gb->Connect(opins[i].pin, ipins[j].pin);
				}
				if (SUCCEEDED(hr)) {
					return NOERROR;
				}

				gb->Disconnect(opins[i].pin);
				gb->Disconnect(ipins[j].pin);
			}
		}

		return E_FAIL;
	}

	HRESULT ConnectPin(IGraphBuilder *gb, IPin *output, IBaseFilter *input, bool direct)
	{
		PinArray		ipins;
		HRESULT			hr;

		EnumPins(input, ipins, Pin::PIN_FLAG_INPUT | Pin::PIN_FLAG_NOT_CONNECTED);
		for (int j=0; j<ipins.GetCount(); j++) {

			if (direct) {
				hr = gb->ConnectDirect(output, ipins[j].pin, NULL);
			} else {
				hr = gb->Connect(output, ipins[j].pin);
			}
			if (SUCCEEDED(hr)) {
				return NOERROR;
			}

			gb->Disconnect(output);
			gb->Disconnect(ipins[j].pin);
		}
		return E_FAIL;
	}


};


