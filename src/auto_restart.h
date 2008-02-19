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

#define WM_AUTORESTART		(WM_USER + 1002)

	//-------------------------------------------------------------------------
	//
	//	AutoRestartPreset class
	//
	//-------------------------------------------------------------------------
	class AutoRestartPreset
	{
	public:
		
		CString		name;
		CTime		last_start;			// time when the graph was last started
		CTime		next_restart;		// time when the graph is to be restarted

	public:
		AutoRestartPreset();
		virtual ~AutoRestartPreset();

		// let's schedule the next restart
		virtual int Schedule();
	};

	//-------------------------------------------------------------------------
	//
	//	AutoRestart class
	//
	//-------------------------------------------------------------------------

	class AutoRestart
	{
	public:

		AutoRestartPreset		*preset;		// current active preset
		bool					enabled;		// is the restart enabled?
		HWND					hwndTarget;		// post message to this window

	public:
		AutoRestart();
		virtual ~AutoRestart();

		// I/O
		void Enable(bool enable);
		void OnTimer();
		void Schedule();
	};


};

