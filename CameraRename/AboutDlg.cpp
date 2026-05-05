// CameraRename
// Copyright (C) 2026 Martin Richter (xMRi-Software) - webmaster@m-ri.de
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see
// <https://www.gnu.org/licenses/>.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#include "pch.h"
#include "CameraRename.h"
#include "afxdialogex.h"
#include "AboutDlg.h"
#include "FileVersionInfo.h"

//-----------------------------------------------------------------------------
// CAboutDlg dialog

IMPLEMENT_DYNAMIC(CAboutDlg, CDialogEx)

CAboutDlg::CAboutDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ABOUTBOX, pParent)
{

}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CFileVersionInfo fvi;
	fvi.GetFileVersionInfo();

	CString strAppVersion;
	strAppVersion.Format(_T("%d.%d.%d"), HIWORD(fvi.dwFileVersionMS),LOWORD(fvi.dwFileVersionMS),HIWORD(fvi.dwFileVersionLS));

	CString strVersion, strMask;
	GetDlgItemText(IDC_ST_VERSION,strMask);
	strVersion.FormatMessage(strMask,strAppVersion.GetString());
	SetDlgItemText(IDC_ST_VERSION,strVersion);
	SetDlgItemText(IDC_ST_COPYRIGHT,fvi.GetLegalCopyright());


	return TRUE;  
}


