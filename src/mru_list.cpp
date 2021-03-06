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
	//	MRUList class
	//
	//-------------------------------------------------------------------------

	MRUList::MRUList()
	{
		list.RemoveAll();
		max_count = 8;
	}

	MRUList::~MRUList()
	{
		list.RemoveAll();
	}

	void MRUList::Save()
	{
		CString		count_name = _T("MRU_count");
		int count = list.GetCount();
		if (count > max_count) count = max_count;

		AfxGetApp()->WriteProfileInt(_T("MRU"), count_name, count);

		for (int i=0; i<count; i++) {
			CString		key;
			key.Format(_T("MRU_%d"), i);
			AfxGetApp()->WriteProfileString(_T("MRU"), key, list[i]);
		}
	}

	void MRUList::Load()
	{
		CString		count_name = _T("MRU_count");
		int count = AfxGetApp()->GetProfileInt(_T("MRU"), count_name, 0);
		if (count > max_count) count = max_count;
		list.RemoveAll();

		for (int i=0; i<count; i++) {
			CString		item;
			CString		key;
			key.Format(_T("MRU_%d"), i);
			item = AfxGetApp()->GetProfileString(_T("MRU"), key, _T(""));
			if (item != _T("")) {
				list.Add(item);
			}
		}
	}

	void MRUList::NotifyEntry(CString filename)
	{
		for (int i=0; i<list.GetCount(); i++) {
			if (list[i] == filename) {
				list.RemoveAt(i);
				break;
			}
		}
		list.InsertAt(0, filename);
		Save();
	}

	void MRUList::Clear()
	{
		list.RemoveAll();
		Save();
	}

	void MRUList::GenerateMenu(CMenu *menu)
	{
		// we insert our items just before the last item (Exit)
		int	cn = menu->GetMenuItemCount();
		while (cn > 21) {
			menu->DeleteMenu(cn-2, MF_BYPOSITION);
			cn--;
		}
		cn = menu->GetMenuItemCount();
		
		int count = min(max_count, list.GetCount());
		int i;
		for (i=0; i<count; i++) {
			CString	t;
			t.Format(_T("&%i %s"), (i+1), list[i]);
			menu->InsertMenu(cn-1 + i, MF_BYPOSITION | MF_STRING, ID_LIST_MRU_FILE0 + i, t);
		}
	
		cn += count;
		menu->InsertMenu(cn-1, MF_BYPOSITION | MF_STRING, ID_LIST_MRU_CLEAR, _T("Clear Recent List"));
		menu->InsertMenu(cn, MF_BYPOSITION | MF_SEPARATOR);
	}


};

