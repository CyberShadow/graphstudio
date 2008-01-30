//-----------------------------------------------------------------------------
//
//	MONOGRAM GraphStudio
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------
#pragma once


namespace DSUtil
{


	//-------------------------------------------------------------------------
	//
	//	URI class
	//
	//-------------------------------------------------------------------------

	class URI
	{
	public:
		CString		protocol;
		CString		host;
		CString		request_url;
		CString		complete_request;
		int			port;

	public:
		URI();
		URI(const URI &u);
		URI(CString url);
		virtual ~URI();

		URI &operator =(const URI &u);
		URI &operator =(CString url);

		int Parse(CString url);
	public:
		// standardne operatory
		operator CString() { return complete_request; }
	};

	//-------------------------------------------------------------------------
	//
	//	Helper Classes
	//
	//-------------------------------------------------------------------------

	class FilterTemplate
	{
	public:
		CString		name;
		GUID		clsid;
		DWORD		version;
		DWORD		merit;
		IMoniker	*moniker;
		CString		file;
		bool		file_exists;

	public:
		FilterTemplate();
		FilterTemplate(const FilterTemplate &ft);
		virtual ~FilterTemplate();
		FilterTemplate &operator =(const FilterTemplate &ft);

		// vytvorenie instancie
		HRESULT CreateInstance(IBaseFilter **filter);
		HRESULT FindFilename();
	};

	class FilterCategory
	{
	public:
		CString		name;
		GUID		clsid;
	public:
		FilterCategory();
		FilterCategory(const FilterCategory &fc);
		virtual ~FilterCategory();
		FilterCategory &operator =(const FilterCategory &fc);
	};

	class FilterCategories
	{
	public:
		CArray<FilterCategory>	categories;
	public:
		FilterCategories();
		virtual ~FilterCategories();

		// enumeracia kategorii
		int Enumerate();
	};


	class FilterTemplates
	{
	public:
		CArray<FilterTemplate>	filters;
	public:
		FilterTemplates();
		virtual ~FilterTemplates();

		// enumeracia kategorii
		int Enumerate(FilterCategory &cat);
		int Enumerate(GUID clsid);

		// vyhladavanie
		int Find(CString name, FilterTemplate *filter);
		int Find(GUID clsid, FilterTemplate *filter); 

		// vytvaranie
		HRESULT CreateInstance(CString name, IBaseFilter **filter);
		HRESULT CreateInstance(GUID clsid, IBaseFilter **filter);
	};

	class Pin
	{
	public:
		IBaseFilter		*filter;
		IPin			*pin;
		CString			name;
		PIN_DIRECTION	dir;

		enum {
			PIN_FLAG_INPUT = 1,
			PIN_FLAG_OUTPUT = 2,
			PIN_FLAG_CONNECTED = 4,
			PIN_FLAG_NOT_CONNECTED = 8,
			PIN_FLAG_ALL = 0xffff
		};
	public:
		Pin();
		Pin(const Pin &p);
		virtual ~Pin();
		Pin &operator =(const Pin &p);
	};

	typedef CArray<Pin>			PinArray;
	typedef CArray<CMediaType>	MediaTypes;


	// zobrazenie property pagesy
	HRESULT DisplayPropertyPage(IBaseFilter *filter, HWND parent = NULL);
	
	// enumeracie pinov
	HRESULT EnumPins(IBaseFilter *filter, PinArray &pins, int flags);
	HRESULT EnumMediaTypes(IPin *pin, MediaTypes &types);
	HRESULT ConnectFilters(IGraphBuilder *gb, IBaseFilter *output, IBaseFilter *input, bool direct=false);
	HRESULT ConnectPin(IGraphBuilder *gb, IPin *output, IBaseFilter *input, bool direct=false);


};






