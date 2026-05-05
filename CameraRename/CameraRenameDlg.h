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

// CameraRenameDlg.h : header file
//

#pragma once

//-----------------------------------------------------------------------------

struct SCameraInfo
{
	CString m_strName;
	CString m_strOrgName;
	CString m_strDevicePath;
};

// CCameraRenameDlg dialog
class CCameraRenameDlg : public CDialogEx
{
// Construction
public:
	CCameraRenameDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMERARENAME_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	CComboBox m_cbCameras;
	CEdit m_edOrgName;
	CEdit m_edName;
	CButton m_btRestore;
	CButton m_btOK;
	HICON m_hIcon;

	const SCameraInfo& FindSelectCameraInfo();
	std::vector<SCameraInfo>	m_aCameras;

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtRestore();
	afx_msg void OnEnChangeName();
	afx_msg void OnCbnSelchangeCameras();
};
