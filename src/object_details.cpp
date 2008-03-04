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
	//	Details
	//
	//-------------------------------------------------------------------------

	int GetFileDetails(CString filename, PropItem *filedetails)
	{
		// we load file details
		CFileFind	find;
		BOOL		found;

		found = find.FindFile(filename);
		if (!found) return -1;
		find.FindNextFile();
		
		CString		file_path;
		CString		file_name;

		file_name = find.GetFileName();
		file_path = find.GetFilePath();
		
		CPath path(file_path);
		int p = path.FindFileName();
		if (p >= 0) file_path = file_path.Left(p);

		filedetails->AddItem(new PropItem(_T("File Name"), file_name));
		filedetails->AddItem(new PropItem(_T("File Path"), file_path));

		// creation and modification date
		CTime	time_cr, time_mod;
		find.GetCreationTime(time_cr);
		find.GetLastWriteTime(time_mod);

		CString		date;
		date = time_cr.Format(_T("%d/%b/%Y  %H:%M:%S"));
		filedetails->AddItem(new PropItem(_T("Created"), date));
		date = time_mod.Format(_T("%d/%b/%Y  %H:%M:%S"));
		filedetails->AddItem(new PropItem(_T("Modified"), date));

		__int64 size = find.GetLength();
		CString	fsize;
		fsize.Format(_T("%I64d"), size);
		filedetails->AddItem(new PropItem(_T("File Size"), fsize));

		PropItem	*vi = new PropItem(_T("Version Info"));
		if (GetVersionInfo(filename, vi) < 0) {
			delete vi;
		} else {
			filedetails->AddItem(vi);
		}

		return 0;
	}

	int GetVersionInfo(CString filename, PropItem *info)
	{
		DWORD		infosize = 0;
		DWORD		handle;

		infosize = GetFileVersionInfoSize(filename, &handle);
		if (infosize == 0) return -1;
	
		LPVOID		vi;
		vi = malloc(infosize);
		GetFileVersionInfo(filename, 0, infosize, vi);

		LPVOID		val;
		UINT		vallen;

		if (VerQueryValue((LPCVOID)vi, L"\\", &val, &vallen)) {

			CString				v;
			VS_FIXEDFILEINFO	*fi  = (VS_FIXEDFILEINFO*)val;

			int		version[4];
			version[0] = (fi->dwFileVersionMS >> 16) & 0xffff;
			version[1] = (fi->dwFileVersionMS >>  0) & 0xffff;
			version[2] = (fi->dwFileVersionLS >> 16) & 0xffff;
			version[3] = (fi->dwFileVersionLS >>  0) & 0xffff;

			v.Format(_T("%d.%d.%d.%d"), version[0], version[1], version[2], version[3]);
			info->AddItem(new PropItem(_T("File Version"), v));

			version[0] = (fi->dwProductVersionMS >> 16) & 0xffff;
			version[1] = (fi->dwProductVersionMS >>  0) & 0xffff;
			version[2] = (fi->dwProductVersionLS >> 16) & 0xffff;
			version[3] = (fi->dwProductVersionLS >>  0) & 0xffff;

			v.Format(_T("%d.%d.%d.%d"), version[0], version[1], version[2], version[3]);
			info->AddItem(new PropItem(_T("Product Version"), v));

			// todo:
			CString name;
			GetObjectName(CLSID_VideoRenderer, name);

		}
		
		free(vi);
		return 0;
	}

	int CLSIDToString(CLSID clsid, CString &str)
	{
		LPOLESTR	ostr = NULL;
		StringFromCLSID(clsid, &ostr);
		if (ostr) {
			str = CString(ostr);
			CoTaskMemFree(ostr);
		}
		return 0;
	}

	int GetObjectFile(GUID clsid, CString &filename)
	{
		// we find this in the registry
		CRegKey		reg;
		int			ret = -1;

		CString		clsid_str;
		CLSIDToString(clsid, clsid_str);

		clsid_str = _T("\\CLSID\\") + clsid_str + _T("\\InprocServer32");
		if (reg.Open(HKEY_CLASSES_ROOT, clsid_str, KEY_READ) == ERROR_SUCCESS) {

			TCHAR		val[1024];
			ULONG		len;
			reg.QueryStringValue(_T(""), val, &len);
			filename = val;

			ret = 0;
			reg.Close();
		}

		return ret;
	}

	int GetObjectName(GUID clsid, CString &name)
	{
		// we find this in the registry
		CRegKey		reg;
		int			ret = -1;

		CString		clsid_str;
		CLSIDToString(clsid, clsid_str);

		clsid_str = _T("\\CLSID\\") + clsid_str;
		if (reg.Open(HKEY_CLASSES_ROOT, clsid_str, KEY_READ) == ERROR_SUCCESS) {

			TCHAR		val[1024];
			ULONG		len;
			reg.QueryStringValue(_T(""), val, &len);
			name = val;

			ret = 0;
			reg.Close();
		}

		return ret;
	}

	int GetFilterDetails(CLSID clsid, PropItem *info)
	{
		info->AddItem(new PropItem(_T("CLSID"), clsid));

		CString name = _T("Unknown");
		GetObjectName(clsid, name);
		info->AddItem(new PropItem(_T("Object Name"), name));

		// load binary data
		CString		clsid_str;
		CLSIDToString(clsid, clsid_str);
		clsid_str = _T("\\CLSID\\{083863F1-70DE-11d0-BD40-00A0C911CE86}\\Instance\\") + clsid_str;

		CRegKey		reg;
		if (reg.Open(HKEY_CLASSES_ROOT, clsid_str, KEY_READ) == ERROR_SUCCESS) {
			
			// get binary data
			ULONG		size;
			if (reg.QueryBinaryValue(_T("FilterData"), NULL, &size) == ERROR_SUCCESS) {
				BYTE *buf = (BYTE*)malloc(size+1);
				reg.QueryBinaryValue(_T("FilterData"), buf, &size);

				// parse data
				DSUtil::FilterTemplate		temp;
				int ret = temp.Load((char*)buf, size);
				if (ret == 0) {

					CString		val;
					val.Format(_T("0x%08x"), temp.merit);
					info->AddItem(new PropItem(_T("Merit"), val));
					val.Format(_T("0x%08x"), temp.version);
					info->AddItem(new PropItem(_T("Version"), val));

				}

				free(buf);
			}
	
			reg.Close();
		}

		CString		filename;
		GetObjectFile(clsid, filename);
		PropItem	*fileinfo = new PropItem(_T("File"));

		if (GetFileDetails(filename, fileinfo) < 0) {
			delete fileinfo;
		} else {
			info->AddItem(fileinfo);
		}

		return 0;
	}

	int GetPinDetails(IPin *pin, PropItem *info)
	{
		PropItem	*group;

		group = info->AddItem(new PropItem(_T("Pin")));
		
		PIN_INFO	pi;
		HRESULT		hr;
		pin->QueryPinInfo(&pi);
		if (pi.pFilter) pi.pFilter->Release();

		group->AddItem(new PropItem(_T("Name"), CString(pi.achName)));
		if (pi.dir == PINDIR_INPUT) {
			group->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_INPUT"))));
		} else {
			group->AddItem(new PropItem(_T("Direction"), CString(_T("PINDIR_OUTPUT"))));
		}

		CComPtr<IPin>	con_pin = NULL;
		pin->ConnectedTo(&con_pin);
		if (con_pin == NULL) {
			group->AddItem(new PropItem(_T("IsConnected"), (bool)FALSE));
		} else {
			group->AddItem(new PropItem(_T("IsConnected"), (bool)TRUE));

			//-----------------------------------------------------------------
			// current media type
			//-----------------------------------------------------------------
			AM_MEDIA_TYPE	mt;
			hr = pin->ConnectionMediaType(&mt);
			if (SUCCEEDED(hr)) {
				CMediaType	mmt; mmt = mt;

				PropItem		*curmt = group->AddItem(new PropItem(_T("Current MediaType")));
				GetMediaTypeDetails(&mmt, curmt);

				FreeMediaType(mt);
			}

		}
		con_pin = NULL;

		return 0;
	}

	int GetMediaTypeDetails(CMediaType *pmt, PropItem *mtinfo)
	{
		CString		id_name;

		GraphStudio::NameGuid(pmt->majortype,	id_name);		mtinfo->AddItem(new PropItem(_T("majortype"), id_name));
		GraphStudio::NameGuid(pmt->subtype,		id_name);		mtinfo->AddItem(new PropItem(_T("subtype"), id_name));
		GraphStudio::NameGuid(pmt->formattype,	id_name);		mtinfo->AddItem(new PropItem(_T("formattype"), id_name));

		mtinfo->AddItem(new PropItem(_T("bFixedSizeSamples"), (bool)pmt->bFixedSizeSamples));
		mtinfo->AddItem(new PropItem(_T("bTemporalCompression"), (bool)pmt->bTemporalCompression));
		mtinfo->AddItem(new PropItem(_T("lSampleSize"), (int)pmt->lSampleSize));
		mtinfo->AddItem(new PropItem(_T("cbFormat"), (int)pmt->cbFormat));

		if (pmt->formattype == FORMAT_WaveFormatEx) {
			WAVEFORMATEX	*wfx = (WAVEFORMATEX*)pmt->pbFormat;
			PropItem	*wfxinfo = mtinfo->AddItem(new PropItem(_T("WAVEFORMATEX")));
			GetWaveFormatExDetails(wfx, wfxinfo);
		} else
		if (pmt->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER	*vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			PropItem	*vihinfo = mtinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER")));
			GetVideoInfoDetails(vih, vihinfo);
		} else
		if (pmt->formattype == FORMAT_VideoInfo2) {
			VIDEOINFOHEADER2	*vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
			PropItem	*vihinfo = mtinfo->AddItem(new PropItem(_T("VIDEOINFOHEADER2")));
			GetVideoInfo2Details(vih, vihinfo);
		}

		return 0;
	}

	int GetWaveFormatExDetails(WAVEFORMATEX *wfx, PropItem *wfxinfo)
	{
		wfxinfo->AddItem(new PropItem(_T("wFormatTag"), (int)wfx->wFormatTag));
		wfxinfo->AddItem(new PropItem(_T("nChannels"), (int)wfx->nChannels));
		wfxinfo->AddItem(new PropItem(_T("nSamplesPerSec"), (int)wfx->nSamplesPerSec));
		wfxinfo->AddItem(new PropItem(_T("nAvgBytesPerSec"), (int)wfx->nAvgBytesPerSec));
		wfxinfo->AddItem(new PropItem(_T("nBlockAlign"), (int)wfx->nBlockAlign));
		wfxinfo->AddItem(new PropItem(_T("wBitsPerSample"), (int)wfx->wBitsPerSample));
		wfxinfo->AddItem(new PropItem(_T("cbSize"), (int)wfx->cbSize));
		return 0;
	}

	int GetVideoInfoDetails(VIDEOINFOHEADER *vih, PropItem *vihinfo)
	{
		vihinfo->AddItem(new PropItem(_T("rcSource"), vih->rcSource));
		vihinfo->AddItem(new PropItem(_T("rcTarget"), vih->rcTarget));
		vihinfo->AddItem(new PropItem(_T("dwBitRate"), (int)vih->dwBitRate));
		vihinfo->AddItem(new PropItem(_T("dwBitErrorRate"), (int)vih->dwBitErrorRate));
		vihinfo->AddItem(new PropItem(_T("AvgTimePerFrame"), (__int64)vih->AvgTimePerFrame));

		PropItem	*bihinfo = vihinfo->AddItem(new PropItem(_T("BITMAPINFOHEADER")));
		GetBitmapInfoDetails(&vih->bmiHeader, bihinfo);
		return 0;
	}

	int GetVideoInfo2Details(VIDEOINFOHEADER2 *vih, PropItem *vihinfo)
	{
		vihinfo->AddItem(new PropItem(_T("rcSource"), vih->rcSource));
		vihinfo->AddItem(new PropItem(_T("rcTarget"), vih->rcTarget));
		vihinfo->AddItem(new PropItem(_T("dwBitRate"), (int)vih->dwBitRate));
		vihinfo->AddItem(new PropItem(_T("dwBitErrorRate"), (int)vih->dwBitErrorRate));
		vihinfo->AddItem(new PropItem(_T("AvgTimePerFrame"), (__int64)vih->AvgTimePerFrame));

		CString		v;
		v.Format(_T("0x%08x"), vih->dwInterlaceFlags);		vihinfo->AddItem(new PropItem(_T("dwInterlaceFlags"), v));
		v.Format(_T("0x%08x"), vih->dwCopyProtectFlags);	vihinfo->AddItem(new PropItem(_T("dwCopyProtectFlags"), v));
		v.Format(_T("0x%08x"), vih->dwPictAspectRatioX);	vihinfo->AddItem(new PropItem(_T("dwPictAspectRatioX"), v));
		v.Format(_T("0x%08x"), vih->dwPictAspectRatioY);	vihinfo->AddItem(new PropItem(_T("dwPictAspectRatioY"), v));
		v.Format(_T("0x%08x"), vih->dwControlFlags);		vihinfo->AddItem(new PropItem(_T("dwControlFlags"), v));

		PropItem	*bihinfo = vihinfo->AddItem(new PropItem(_T("BITMAPINFOHEADER")));
		GetBitmapInfoDetails(&vih->bmiHeader, bihinfo);
		return 0;
	}

	int GetBitmapInfoDetails(BITMAPINFOHEADER *bih, PropItem *bihinfo)
	{
		CString		v;

		bihinfo->AddItem(new PropItem(_T("biSize"), (int)bih->biSize));
		bihinfo->AddItem(new PropItem(_T("biWidth"), (int)bih->biWidth));
		bihinfo->AddItem(new PropItem(_T("biHeight"), (int)bih->biHeight));
		bihinfo->AddItem(new PropItem(_T("biPlanes"), (int)bih->biPlanes));
		bihinfo->AddItem(new PropItem(_T("biBitCount"), (int)bih->biBitCount));

		v.Format(_T("0x%08x"), bih->biCompression);		bihinfo->AddItem(new PropItem(_T("biCompression"), v));
		bihinfo->AddItem(new PropItem(_T("biSizeImage"), (int)bih->biSizeImage));

		bihinfo->AddItem(new PropItem(_T("biXPelsPerMeter"), (int)bih->biXPelsPerMeter));
		bihinfo->AddItem(new PropItem(_T("biYPelsPerMeter"), (int)bih->biYPelsPerMeter));
		bihinfo->AddItem(new PropItem(_T("biClrUsed"), (int)bih->biClrUsed));
		bihinfo->AddItem(new PropItem(_T("biClrImportant"), (int)bih->biClrImportant));

		return 0;
	}


};


