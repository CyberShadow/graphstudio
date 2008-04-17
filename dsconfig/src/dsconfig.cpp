//-----------------------------------------------------------------------------
//
//	DSConfig
//
//	Author : Igor Janos
//
//-----------------------------------------------------------------------------

#include "stdafx.h"


CWinApp theApp;
using namespace std;


GUID			clsid;
HMODULE			library  = NULL;

typedef HRESULT (_stdcall *GetClassObjectProc)(REFCLSID rClsId, REFIID riid, void **ppv); 

void usage()
{
	_tprintf(_T("\n"));
	_tprintf(_T("MONOGRAM DSconfig - Tool to display property pages for DirectShow filters.\n"));
	_tprintf(_T("This software is free under the GNU GPL licence\n"));
	_tprintf(_T("http://blog.monogram.sk/janos\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("Usage:     dsconfig <filter_clsid> <dll>\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("<filter_clsid>        CLSID of the filter to display property page for\n"));
	_tprintf(_T("<dll>                 Optional. Path to DLL(AX) file to be used\n"));
	_tprintf(_T("\n"));
	_tprintf(_T("Example: Display property page of ffdshow video decoder\n"));
	_tprintf(_T("         dsconfig {04FE9017-F873-410E-871E-AB91661A4EF7}\n"));
	_tprintf(_T("\n"));
	exit(-1);
}

int parse_params(int argc, TCHAR *argv[])
{
	if (argc < 2) usage();

	if (argc > 1) {
		if (FAILED(CLSIDFromString((LPOLESTR)argv[1], &clsid))) {
			_tprintf(_T("\"%s\" is not a valid CLSID !!!\n"), argv[1]);
			exit(-2);
		}
	}

	if (argc > 2) {
		library = LoadLibrary(argv[2]);
		if (!library) {
			_tprintf(_T("Cannot load library: \"%s\"\n"), argv[2]);
			exit(-4);
		}
	}

	return 0;
}

int display_page()
{
	CComPtr<IUnknown>		obj;
	HRESULT					hr;

	obj = NULL;

	if (library) {
		// create from DLL
		GetClassObjectProc GetClass = (GetClassObjectProc)GetProcAddress(library, "DllGetClassObject"); 
		if (!GetClass) {
			_tprintf(_T("Provided library does not contain DllGetClassObject!!!\n"));
			return -1; 
		}
	 
		 // class factory interface 
		 CComPtr<IClassFactory>		factory = NULL; 
	 
		 hr = GetClass(clsid, IID_IClassFactory, (void**)&factory); 
		 if (FAILED(hr)) {
			 _tprintf(_T("Provided library does not contain requested object!!\n"));
			 return -1; 
		 }
	 
		 hr = factory->CreateInstance(NULL, IID_IUnknown, (void**)&obj); 
		 if (FAILED(hr)) {
			 _tprintf(_T("Cannot create object!!\n"));
			 return -1; 
		 }

	     // not needed any more
		 factory = NULL;
	} else {

		// load from registry
		hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&obj);
		if (FAILED(hr)) {
			_tprintf(_T("Cannot create object!!\n"));
			return -1;	
		}
	}

	// check for ISpecifyPropertyPages
	CComPtr<ISpecifyPropertyPages>		pages;
	hr = obj->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pages);
	if (FAILED(hr)) {
		_tprintf(_T("Object does not have a property page\n"));
		return 0;
	}

    // Show the page. 
    CAUUID caGUID;
    pages->GetPages(&caGUID);

	IUnknown	*unk = (IUnknown*)obj;

    OleCreatePropertyFrame(
		GetDesktopWindow(),     // Parent window
        0, 0,                   // Reserved
        L"Properties",			// Caption for the dialog box
        1,                      // Number of objects (just the filter)
        &unk,					// Array of object pointers. 
        caGUID.cElems,          // Number of property pages
        caGUID.pElems,          // Array of property page CLSIDs
        0,                      // Locale identifier
        0, NULL                 // Reserved
    );

    // Clean up.
    CoTaskMemFree(caGUID.pElems);

	pages = NULL;
	obj   = NULL;
	
	return 0;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) return 1;
	if (FAILED(CoInitialize(NULL))) {
		_tprintf(_T("Cannot Initialize COM\n"));
		exit(-1);
	}

	parse_params(argc, argv);

	int ret = display_page();

	if (library != NULL) {
		FreeLibrary(library);
		library = NULL;
	}
	CoUninitialize();
	return ret;
}
