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
	//	Details
	//
	//-------------------------------------------------------------------------

	int GetVersionInfo(CString filename, PropItem *info);
	int GetFileDetails(CString filename, PropItem *filedetails);
	int GetFilterDetails(CLSID clsid, PropItem *info);

	int CLSIDToString(CLSID clsid, CString &str);

	int GetObjectName(GUID clsid, CString &name);
	int GetObjectFile(GUID clsid, CString &filename);

	int GetPinDetails(IPin *pin, PropItem *info);
	int GetMediaTypeDetails(CMediaType *pmt, PropItem *mtinfo);

	int GetWaveFormatExDetails(WAVEFORMATEX *wfx, PropItem *wfxinfo);
};


