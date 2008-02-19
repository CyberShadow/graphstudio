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
	//	AutoRestartPreset class
	//
	//-------------------------------------------------------------------------

	AutoRestartPreset::AutoRestartPreset()
	{
	}

	AutoRestartPreset::~AutoRestartPreset()
	{
		// todo:
	}

	int AutoRestartPreset::Schedule()
	{
		// try 10 second intervals
		CTime		now  = CTime::GetCurrentTime();
		CTimeSpan	span(1, 0, 0, 0);

		now = now + span;

		int	year  = now.GetYear();
		int month = now.GetMonth();
		int day   = now.GetDay();

		// at 4:00 in the morning next day
		next_restart = CTime(year, month, day, 4, 0, 0);

		return 0;
	}

	//-------------------------------------------------------------------------
	//
	//	AutoRestart class
	//
	//-------------------------------------------------------------------------

	AutoRestart::AutoRestart()
	{
		preset	= new AutoRestartPreset();
		enabled = false;
	}

	AutoRestart::~AutoRestart()
	{
		delete preset;
	}

	void AutoRestart::Enable(bool enable)
	{
		enabled = enable;		
	}

	void AutoRestart::Schedule()
	{
		if (preset) preset->Schedule();
	}

	void AutoRestart::OnTimer()
	{
		if (!enabled || !preset) return ;

		// check the time
		CTime		now = CTime::GetCurrentTime();
		if (now > preset->next_restart) {

			// fire the signal
			if (IsWindow(hwndTarget)) {
				SendMessage(hwndTarget, WM_AUTORESTART, 0, 0);
			}

		}
	}



};

