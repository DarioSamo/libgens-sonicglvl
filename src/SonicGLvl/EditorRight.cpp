//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
//
//    This file is part of SonicGLvl, a community-created free level editor 
//    for the PC version of Sonic Generations.
//
//    SonicGLvl is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    SonicGLvl is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//    
//
//    Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.
//=========================================================================

#include "EditorApplication.h"
#include "ObjectSet.h"

void EditorApplication::initializeCurrentLayerGUI() {
	SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	
	for (int i = 0; i < set_mapping.size(); i++)
	{
		SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)set_mapping[i]->getName().c_str());
	}

	SendDlgItemMessage(hRightDlg, IDC_RIGHT_CURRENT_LAYER, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

INT_PTR CALLBACK RightBarCallback(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg)
	{
	case WM_INITDIALOG:
		return true;

	case WM_ACTIVATE:
		return false;

	case WM_DESTROY:
		return true;

	case WM_CLOSE:
		return false;

	case WM_NOTIFY:
	{
		switch (LOWORD(wParam))
		{
		
		}
		break;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		
		}
		break;
	}
	}

	return false;
}