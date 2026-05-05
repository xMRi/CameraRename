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

// CameraRenameDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "CameraRename.h"
#include "CameraRenameDlg.h"
#include "AboutDlg.h"

#include "afxdialogex.h"

#include <dshow.h>
#pragma comment(lib, "strmiids.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString GetSystemErrorDescription(HRESULT hr)
{
	LPTSTR lpMessageBuffer=NULL;
	int iReturn = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPTSTR)&lpMessageBuffer,
		0,
		NULL
    );
	if (!iReturn)
	{
		// error text couldn't be determined
		LONG dwLastError = ::GetLastError();
		UNUSED_ALWAYS(dwLastError);
		return _T("");
	}
	// newlines abschneiden
	LPTSTR lpszEnd = lpMessageBuffer+_tcsnbcnt(lpMessageBuffer,-1),
		   lpszWork = lpszEnd;
	while ((lpszWork=_tcsdec(lpMessageBuffer,lpszWork)) && _istascii(*lpszWork) && _istcntrl(*lpszWork))
		;
    CString strOut{ lpMessageBuffer,static_cast<int>(lpszWork ? lpszWork-lpMessageBuffer+1 : lpszEnd-lpMessageBuffer) };
	
	// Free the buffer allocated by the system
	LocalFree(lpMessageBuffer); 
	// Fini
	return strOut;
}

CString BuildRegPath(CString devicePath)
{
    devicePath.MakeLower();
    devicePath.Replace(_T("\\\\?\\usb"),_T("\\##?#USB"));
    devicePath.Replace(_T("\\global"),_T("\\#GLOBAL\\Device Parameters"));
    return _T("SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{65E8773D-8F56-11D0-A3B9-00A0C9223196}")+devicePath;
}

HRESULT EnumerateCameras(std::vector<SCameraInfo> &result)
{   
    CComPtr<ICreateDevEnum> devEnum;
    CComPtr<IEnumMoniker> enumMoniker;

    HRESULT hr = devEnum.CoCreateInstance(CLSID_SystemDeviceEnum);
    if (FAILED(hr))
        return hr;

    hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
    if (FAILED(hr))
        return HRESULT_FROM_WIN32(hr);

    CComPtr<IMoniker> moniker;
    while ((hr = enumMoniker->Next(1, &moniker, nullptr)) == S_OK)
    {
        CComPtr<IPropertyBag> bag;
        if (SUCCEEDED(moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)&bag)))
        {
            SCameraInfo cam;
            CComVariant vName, vPath;

            if (SUCCEEDED(bag->Read(L"FriendlyName", &vName, nullptr)))
                cam.m_strOrgName = cam.m_strName = vName.bstrVal;

            if (SUCCEEDED(bag->Read(L"DevicePath", &vPath, nullptr)))
                cam.m_strDevicePath = vPath.bstrVal;

            result.push_back(cam);
        }
        moniker.Release();
    }

    return FAILED(hr) ? hr : S_OK;
}

HRESULT  RenameCamera(const CString& devicePath, const CString& newName)
{
    CString regPath = BuildRegPath(devicePath);

    HRESULT hr;
    CRegKey key;
	if ((hr=key.Open(HKEY_LOCAL_MACHINE, regPath)) != ERROR_SUCCESS)
        return hr;

	hr = key.SetStringValue(_T("FriendlyName"), newName);
    return hr;
}

//-----------------------------------------------------------------------------
// CCameraRenameDlg dialog

CCameraRenameDlg::CCameraRenameDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAMERARENAME_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

const SCameraInfo& CCameraRenameDlg::FindSelectCameraInfo() 
{
	static SCameraInfo const emptyCameraInfo = SCameraInfo{};
    int n = m_cbCameras.GetCurSel();
    if (n<0)
        return emptyCameraInfo;

    auto idx = m_cbCameras.GetItemData(n);
    if (idx<0 || idx>=m_aCameras.size())
        return emptyCameraInfo;

    return m_aCameras[idx];
}

void CCameraRenameDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CB_CAMERAS, m_cbCameras);
    DDX_Control(pDX, IDC_ED_ORIGINALNAME, m_edOrgName);
    DDX_Control(pDX, IDC_ED_NAME, m_edName);
    DDX_Control(pDX, IDC_BT_RESTORE, m_btRestore);
    DDX_Control(pDX, IDOK, m_btOK);
}

BEGIN_MESSAGE_MAP(CCameraRenameDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BT_RESTORE, &CCameraRenameDlg::OnBnClickedBtRestore)
    ON_EN_CHANGE(IDC_ED_NAME, &CCameraRenameDlg::OnEnChangeName)
    ON_CBN_SELCHANGE(IDC_CB_CAMERAS, &CCameraRenameDlg::OnCbnSelchangeCameras)
END_MESSAGE_MAP()


// CCameraRenameDlg message handlers

BOOL CCameraRenameDlg::OnInitDialog()
{
	__super::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Load all cameras
	HRESULT hr = EnumerateCameras(m_aCameras);
    if (FAILED(hr))
    {
        CString strMask{ MAKEINTRESOURCE(IDP_ENUMERATE_FAILED) };
        CString strMsg;
        strMsg.FormatMessage(strMask, GetSystemErrorDescription(hr));
        AfxMessageBox(strMsg, MB_ICONERROR);
        EndDialog(IDABORT);
        return FALSE;
	}
	
    // Find the original names or save them, if they don't exist
    CRegKey key;
    CString strKey{ _T("Software\\") };
    strKey += theApp.m_pszRegistryKey;
    strKey += _T("\\");
    strKey += theApp.m_pszProfileName;
    LONG lRes = key.Open(HKEY_LOCAL_MACHINE, strKey, KEY_READ | KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
    {
        lRes = key.Create(HKEY_LOCAL_MACHINE, strKey);
        if (lRes!=ERROR_SUCCESS)
        {
            AfxMessageBox(IDP_FAILURE);
            EndDialog(IDABORT);
            return FALSE;
        }
    }

    // Load the combo box
	for (size_t i = 0; i < m_aCameras.size(); ++i)
    {
        // Prüfen ob wir den Originalnamen schon kennen.
        DWORD dwLen = _MAX_PATH;
        CString strOrgName;
		lRes = key.QueryStringValue(m_aCameras[i].m_strDevicePath,CStrBuf(strOrgName,_MAX_PATH),&dwLen);
        if (lRes==ERROR_SUCCESS)
            // Get the original name
            m_aCameras[i].m_strOrgName = strOrgName;
        else
            // Save the original name for the first time.
            key.SetStringValue(m_aCameras[i].m_strDevicePath,m_aCameras[i].m_strOrgName);

		auto n = m_cbCameras.AddString(m_aCameras[i].m_strName);
		if (n >= 0)
            m_cbCameras.SetItemData(n, i);
	}   
	if (m_aCameras.empty())
    {
        AfxMessageBox(IDP_NO_CAMERAS);
        EndDialog(IDABORT);
        return FALSE;
    }
    else
        m_cbCameras.SetCurSel(0);

    OnCbnSelchangeCameras();
	return TRUE;  
}

void CCameraRenameDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		__super::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCameraRenameDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		__super::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCameraRenameDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CCameraRenameDlg::OnOK()
{
    // Check the name if euqal.
    CString strName;
    m_edName.GetWindowText(strName);

    auto const &camera = FindSelectCameraInfo();

    // Ask the user 
	CString strMask{ MAKEINTRESOURCE(IDP_QUERY_RENAME) };
	CString strMsg;
	strMsg.FormatMessage(strMask, camera.m_strName, strName, camera.m_strOrgName);
	if (AfxMessageBox(strMsg, MB_ICONQUESTION | MB_YESNO)!=IDYES)
        return;

	auto hr = RenameCamera(camera.m_strDevicePath, strName);
    if (FAILED(hr))
    {
        strMask = CString{ MAKEINTRESOURCE(IDP_RENAME_FAILED) };
        strMsg.FormatMessage(strMask, GetSystemErrorDescription(hr));
        AfxMessageBox(strMsg, MB_ICONERROR);
		return;
    }

    __super::OnOK();
}

void CCameraRenameDlg::OnCancel()
{
    __super::OnCancel();
}

void CCameraRenameDlg::OnBnClickedBtRestore()
{
    CString strOrgName;
    m_edOrgName.GetWindowText(strOrgName);
    m_edName.SetWindowText(strOrgName);
}

void CCameraRenameDlg::OnEnChangeName()
{
    CString strName;
	m_edName.GetWindowText(strName);
	CString strOrgName;
	m_edOrgName.GetWindowText(strOrgName);

    auto const &camera = FindSelectCameraInfo();

	m_btOK.EnableWindow(!strName.IsEmpty() && strName!=camera.m_strName);
	m_btRestore.EnableWindow(strName!=strOrgName);
}

void CCameraRenameDlg::OnCbnSelchangeCameras()
{
    auto const &camera = FindSelectCameraInfo();

    m_edOrgName.SetWindowText(camera.m_strOrgName);
	m_edName.SetWindowText(camera.m_strName);
}
