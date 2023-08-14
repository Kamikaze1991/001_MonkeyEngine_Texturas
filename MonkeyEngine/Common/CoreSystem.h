//*********************************************************
//
// Copyright (c) Rick Laboratories. All rights reserved.
// This code is licensed under the MIT License (MIT).
// Autor: Luis Borrero
//
//*********************************************************

#ifndef _CORE_SYSTEM_H_
#define _CORE_SYSTEM_H_
#include "CoreUtil.h"
#include "CoreEngine.h"

/// <summary>
/// Core system windows class
/// </summary>
class CoreSystem {
private:
	static HWND mCoreHwnd;
	static CoreEngine* mCoreEngine;
public:
	static int Run(CoreEngine* CoreEngine, HINSTANCE mHinstance, int cmdShow);
	static HWND GetHwnd();
protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
#endif
