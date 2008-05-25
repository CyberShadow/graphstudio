#include "stdafx.h"
#include "dsconfig-win.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

GUID			clsid = GUID_NULL;


// CdsconfigwinApp
BEGIN_MESSAGE_MAP(CdsconfigwinApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CdsconfigwinApp theApp;

// CdsconfigwinApp construction
CdsconfigwinApp::CdsconfigwinApp()
{
}

int display_page()
{
	CComPtr<IUnknown>		obj;
	HRESULT					hr;

	obj = NULL;

	// load from registry
	hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&obj);
	if (FAILED(hr)) {
		return -1;	
	}

	// check for ISpecifyPropertyPages
	CComPtr<ISpecifyPropertyPages>		pages;
	hr = obj->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pages);
	if (FAILED(hr)) {
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

BOOL CdsconfigwinApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	if (!AfxOleInit()) return FALSE;

	SetRegistryKey(_T("DSConfig-Win"));

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// we take only one parameter = the CLSID of the filter
	// to display property page for
	// if we've been started with a command line parameter
	// do open the file
	if (cmdInfo.m_strFileName != _T("")) {
		if (FAILED(CLSIDFromString(cmdInfo.m_strFileName.GetBuffer(), &clsid))) {
			return FALSE;
		}
	}

	// display the page
	display_page();

	return FALSE;
}
