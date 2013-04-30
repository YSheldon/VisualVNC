#include "StdAfx.h"

#include "VNDriverLoaderEx.h"
#include "bindview.h"

#include "CommonIO.h"
#include <WinIoCtl.h>

VNDriverLoaderEx::VNDriverLoaderEx()
{
	m_bInstalled = FALSE;
	initDataDir();
	m_strDriverLinkName = _T("\\\\.\\VNCPassThru");
	m_hDevice = NULL;
}

VNDriverLoaderEx::~VNDriverLoaderEx()
{
	if (m_bInstalled)
	{
		uninstallDriver_Passthru();
	}
	closeDeviceLink();
}

VNDriverLoaderEx::setJustUnload()
{
	m_bInstalled = TRUE;
}

BOOL VNDriverLoaderEx::FolderExist(CString strPath)
{
	WIN32_FIND_DATA wfd;
	BOOL rValue = FALSE;
	HANDLE hFind = FindFirstFile(strPath, &wfd);
	if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = TRUE;
	}
	FindClose(hFind);
	return rValue;
}

BOOL VNDriverLoaderEx::FileExist(CString strPathName)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(strPathName, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	else
	{
		FindClose(hFind);
		return true;
	}
	return false;
}

void VNDriverLoaderEx::initDataDir()
{
	CString cstrDir;
	TCHAR cBuffer[260];
	GetCurrentDirectory(MAX_PATH, cBuffer);
	CString cstrPath(cBuffer);
	if (FolderExist(cstrPath + _T("\\data")))
	{
		cstrDir = cstrPath;
	}
	else
	{
		int nCount = cstrPath.ReverseFind(_T('\\'));
		cstrPath.Delete(nCount, cstrPath.GetLength() - nCount);

		if (FolderExist(cstrPath + _T("\\data")))
		{
			cstrDir = cstrPath;
		}
		else
		{
			MyMessageBox_Error(_T("initDataDir"));
			return;
		}
	}
	cstrDir += _T("\\data\\");
	m_strDataDirectory = cstrDir;
}

LRESULT VNDriverLoaderEx::MyMessageBox(CString strText, CString strCaption, UINT iType)
{
	BCGP_MSGBOXPARAMS params;
	
	params.lpszCaption = strCaption;
	params.lpszText = strText;
	params.bDrawButtonsBanner = TRUE;
	params.bUseNativeControls = FALSE;
	
	params.dwStyle = iType;
	params.bIgnoreStandardButtons = FALSE;
	params.bShowCheckBox = FALSE;
	
	params.dwStyle |= MB_ICONEXCLAMATION;
	
	int result = BCGPMessageBoxIndirect(&params);
	return result;
}

LRESULT VNDriverLoaderEx::MyMessageBox_YESNO(CString strText, CString strCaption)
{
	BCGP_MSGBOXPARAMS params;
	
	params.lpszCaption = strCaption;
	params.lpszText = strText;
	params.bDrawButtonsBanner = TRUE;
	params.bUseNativeControls = FALSE;
	
	params.dwStyle = MB_YESNO;
	params.bIgnoreStandardButtons = FALSE;
	params.bShowCheckBox = FALSE;
	
	params.dwStyle |= MB_ICONEXCLAMATION;
	
	int result = BCGPMessageBoxIndirect(&params);
	return result;
}

CString VNDriverLoaderEx::getServiceInfFilePath_Passthru()
{
	CString strPathName = m_strDataDirectory + _T("netsf.inf");
	return strPathName;
}

CString VNDriverLoaderEx::getServiceInfFilePath_PassthruMP()
{
	CString strPathName = m_strDataDirectory + _T("netsf_m.inf");
	return strPathName;
}

DWORD VNDriverLoaderEx::getServiceInfFilePath_Passthru(LPTSTR lpFilename, DWORD nSize)
{
	// Get Path to This Module
	DWORD nResult;
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];

	nResult = GetModuleFileName(NULL, lpFilename, nSize);

	if (nResult == 0)
	{
		return 0;
	}

	_tsplitpath(lpFilename, szDrive, szDir, NULL, NULL);

	_tmakepath(lpFilename, szDrive, szDir, _T("netsf"), _T(".inf"));

	return (DWORD)_tcslen(lpFilename);
}

DWORD VNDriverLoaderEx::getServiceInfFilePath_PassthruMP(LPTSTR lpFilename, DWORD nSize)
{
	// Get Path to This Module
	DWORD nResult;
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];

	nResult = GetModuleFileName(NULL, lpFilename, nSize);

	if (nResult == 0)
	{
		return 0;
	}

	_tsplitpath(lpFilename, szDrive, szDir, NULL, NULL);

	_tmakepath(lpFilename, szDrive, szDir, _T("netsf_m"), _T(".inf"));

	return (DWORD)_tcslen(lpFilename);
}



DWORD VNDriverLoaderEx::installDriver_Passthru()
{
	if (m_bInstalled)
	{
		MyMessageBox_Error(_T("The driver has been installed."), _T("Run Info"));
		return 0;
	}

	DWORD nResult;

	//_tprintf( _T("Installing %s...\n"), NDISPROT_FRIENDLY_NAME );

	nResult = MyMessageBox(_T("��Ҫ��װ Passthru ����������"), _T("Passthru�����������"), MB_OKCANCEL | MB_ICONINFORMATION);

	if (nResult != IDOK)
	{
		return 0;
	}

	// Get Path to Service INF File
	// ----------------------------
	// The INF file is assumed to be in the same folder as this application...
	TCHAR szFileFullPath[_MAX_PATH];


	//-----------------------------------------------------------------------
	//��һ����һ��ϵͳ��ʹ���������װPassthruʱ������ְ�װʧ�ܵ����������
	//��װ�ɹ�����passthru miniport�Ĳ���û�а�װ��ȥ,��windows Ŀ¼�� ��setupapi.log�ļ��п��Կ���
	//��װʧ�ܼ�¼���������Ҳ����ļ����ڡ��豸��������ѡ����ʾ�����豸��
	//Ҳ������ ���������� ���濴�� passthru miniport����ֶ���װ��'������������"->"��װ"->������ѡ��
	//Ӳ����netsf.inf���а�װ�ɹ������ó���װ�Ϳ����ˡ�
	//�������ϲ���һ�£��������Ӧ������Ϊ Passthru���������Ҫ����inf�ļ�����netsf_m.inf����û�б����Ƶ�
	//ϵͳ��inf Ŀ¼��c:\windows\inf��ȥ����Ȼ netsf.inf ������[CopyInf   = netsf_m.inf]��Ҫ����netsf_m.inf
	//��������� ����������The \"CopyINF\" directive, by design, is only observed if the original INF is 
	//not yet in the INF directory. So to work around the problem, you have to 
	//update your installation app (snetcfg) to also copy the Net class (miniport) 
	//inf using SetupCopyOEMInf when it comes to installing IM drivers. Make sure 
	//you specify a fully qualified path of the INF in the SetupCopyOEMInf 
	//arguments. 
	//��
	//
	//�����������Ľ�������Լ��� netsf_m.inf����ļ��ŵ�c:\windows\infĿ¼ȥ�������ͨ���� netsf.inf�������
	//copyfile���copy Passthru.sysһ�����һ��copy   netsf_m.inf�����һ�ַ�������������������ӵ���
	//SetupCopyOEMInfW������netsf_m.inf�Ĵ���
	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szDirWithDrive[_MAX_DRIVE + _MAX_DIR];
	CString strMPInfPathName = getServiceInfFilePath_PassthruMP();
	if (!FileExist(strMPInfPathName))
	{
		//_tprintf( _T("Unable to get INF file path.\n") );
		MyMessageBox(_T("��ȡINF�ļ�·��ʧ�ܣ�"), _T("��װ���������ʾ"), MB_OK);
		return 0;
	}
	//
	// Get the path where the INF file is.
	//
	_tcscpy(szFileFullPath, strMPInfPathName);

	_tsplitpath(szFileFullPath, szDrive, szDir, NULL, NULL);

	_tcscpy(szDirWithDrive, szDrive);
	_tcscat(szDirWithDrive, szDir);
	if (!SetupCopyOEMInf(szFileFullPath, szDirWithDrive, // Other files are in the
			 // same dir. as primary INF
			 SPOST_PATH,	 // First param is path to INF
			 0, 			 // Default copy style
			 NULL,  		 // Name of the INF after
			 // it's copied to %windir%\inf
			 0, 			 // Max buf. size for the above
			 NULL,  		 // Required size if non-null
			 NULL)  		 // Optionally get the filename
		// part of Inf name after it is copied.
	   )
	{
		MyMessageBox(_T("���� PasstruMP ��inf��װ�ļ���ϵͳĿ¼ʧ�ܣ�"), _T("��װ���������ʾ"), MB_OK);
	}
	//------------------------------------------------------------------------


	CString strInfPathName = getServiceInfFilePath_Passthru();

	if (!FileExist(strInfPathName))
	{
		//     _tprintf( _T("Unable to get INF file path.\n") );
		MyMessageBox(_T("��ȡINF�ļ�·��ʧ�ܣ�"), _T("��װ���������ʾ"), MB_OK);
		return 0;
	}

	//_tprintf( _T("INF Path: %s\n"), szFileFullPath );

	HRESULT hr = S_OK;

	//_tprintf( _T("PnpID: %s\n"),   _T( "ms_passthru"));
	_tcscpy(szFileFullPath, strInfPathName);

	_bstr_t bstrFileFullPath = szFileFullPath;
	_bstr_t bstrPnpID = "ms_passthru";
	hr = InstallSpecifiedComponent(bstrFileFullPath,  				  //������װ��inf�ļ�·�� , �ʵ��޸İ�
		 bstrPnpID, 				// NDISPROT_SERVICE_PNP_DEVICE_ID,    //���ҲҪ�ʵ��޸ĵ�
		 &GUID_DEVCLASS_NETSERVICE  		  //NDIS Protocal ����
	);

	if (hr != S_OK)
	{
		/*ErrMsg( hr, L"InstallSpecifiedComponent\n" );*/
		MyMessageBox(_T("��װ����ʧ�ܣ�"), _T("��װ���������ʾ"), MB_OK);
	}
	else
	{
		m_bInstalled = TRUE;
		MessageBox(NULL, _T("��װ�����ɹ���"), _T("��װ������ʾ"), MB_OK);
	}

	return 0;
}

DWORD VNDriverLoaderEx::uninstallDriver_Passthru()
{
	if (!m_bInstalled)
	{
		MyMessageBox_Error(_T("The driver hasn't been installed."), _T("Run Info"));
		return 0;
	}
	//_tprintf( _T("Uninstalling %s...\n"), NDISPROT_FRIENDLY_NAME );

	int nResult = MyMessageBox(_T("��Ҫж��Passthru����������"), _T("Passthru�����������"), MB_OKCANCEL | MB_ICONINFORMATION);

	if (nResult != IDOK)
	{
		return 0;
	}

	INetCfg* pnc;
	INetCfgComponent* pncc;
	INetCfgClass* pncClass;
	INetCfgClassSetup* pncClassSetup;
	LPWSTR lpszApp;
	GUID guidClass;
	OBO_TOKEN obo;
	HRESULT hr;

	hr = HrGetINetCfg(TRUE, APP_NAME, &pnc, &lpszApp);

	if (hr == S_OK)
	{
		//
		// Get a reference to the network component to uninstall.
		//
		hr = pnc->FindComponent(L"ms_passthru", &pncc);

		if (hr == S_OK)
		{
			//
			// Get the class GUID.
			//
			hr = pncc->GetClassGuid(&guidClass);

			if (hr == S_OK)
			{
				//
				// Get a reference to component's class.
				//

				hr = pnc->QueryNetCfgClass(&guidClass, IID_INetCfgClass, (PVOID *)&pncClass);
				if (hr == S_OK)
				{
					//
					// Get the setup interface.
					//

					hr = pncClass->QueryInterface(IID_INetCfgClassSetup, (LPVOID *)&pncClassSetup);

					if (hr == S_OK)
					{
						//
						// Uninstall the component.
						//

						ZeroMemory(&obo, sizeof(OBO_TOKEN));

						obo.Type = OBO_USER;

						hr = pncClassSetup->DeInstall(pncc, &obo, NULL);
						if ((hr == S_OK) || (hr == NETCFG_S_REBOOT))
						{
							hr = pnc->Apply();

							if ((hr != S_OK) && (hr != NETCFG_S_REBOOT))
							{
								//ErrMsg( hr,
								//     L"Couldn't apply the changes after"
								//     L" uninstalling %s.",
								//     _T ("ms_passthru" ));
								MyMessageBox(_T("ж������֮���޷��ɹ�Ӧ�ã�"), _T("��װ���������ʾ"), MB_OK);
							}
							else
							{
								m_bInstalled = FALSE;
								MyMessageBox(_T("�ɹ�ж��������"), _T("��װ������ʾ"), MB_OK);
							}
						}
						else
						{
							//ErrMsg( hr,
							//     L"Failed to uninstall %s.",
							//     _T("ms_passthru" ));
							MyMessageBox(_T("ж���������ʧ�ܣ�"), _T("��װ���������ʾ"), MB_OK);
						}

						ReleaseRef(pncClassSetup);
					}
					else
					{
						//ErrMsg( hr,
						//     L"Couldn't get an interface to setup class." );
						MyMessageBox(_T("�޷��õ���װ��ӿڣ�"), _T("��װ���������ʾ"), MB_OK);
					}

					ReleaseRef(pncClass);
				}
				else
				{
					//ErrMsg( hr,
					//     L"Couldn't get a pointer to class interface "
					//     L"of %s.",
					//     _T ("ms_passthru") );
					MessageBox(NULL, _T("�޷��õ���װ��ӿڣ�"), _T("��װ���������ʾ"), MB_OK);
				}
			}
			else
			{
				//ErrMsg( hr,
				//     L"Couldn't get the class guid of %s.",
				//     _T("ms_passthru") );
				MyMessageBox(_T("�޷��õ���װ��ӿڵ� GUID ��"), _T("��װ���������ʾ"), MB_OK);
			}

			ReleaseRef(pncc);
		}
		else
		{
			//ErrMsg( hr,
			//     L"Couldn't get an interface pointer to %s.",
			//     _T ("ms_passthru") );

			//MyMessageBox(_T("�޷��õ�һ���ӿ�ָ�룡"), _T("��װ���������ʾ"), MB_OK);
			MyMessageBox(_T("���������ڻ��Ѵ���ж��״̬��"), _T("��װ���������ʾ"), MB_OK);
		}

		HrReleaseINetCfg(pnc, TRUE);
	}
	else
	{
		if ((hr == NETCFG_E_NO_WRITE_LOCK) && lpszApp)
		{
			//   ErrMsg( hr,
			//  	  L"%s currently holds the lock, try later.",
			//  	 lpszApp );
			MyMessageBox(_T("�����������⣬�Ժ����ԣ�"), _T("��װ���������ʾ"), MB_OK);
			CoTaskMemFree(lpszApp);
		}
		else
		{
			// ErrMsg( hr, L"Couldn't get the notify object interface." );
			MyMessageBox(_T("�޷��õ�֪ͨ����ӿڣ�"), _T("��װ���������ʾ"), MB_OK);
		}
	}

	return 0;
}

BOOL VNDriverLoaderEx::openDeviceLink()
{
	if (m_hDevice)
	{
		return TRUE;
		/*
		if (queryAlive())
		{
			return TRUE;
		}
		else
		{
			m_hDevice = NULL;
		}
		*/
	}

	// �򿪵����������������豸�ľ��
	m_hDevice = ::CreateFile(m_strDriverLinkName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hDevice == INVALID_HANDLE_VALUE)
	{
		m_hDevice = NULL;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL VNDriverLoaderEx::closeDeviceLink()
{
	if (m_hDevice)
	{
		BOOL bRet = ::CloseHandle(m_hDevice);
		if (bRet)
		{
			m_hDevice = NULL;
			return TRUE;
		}
		else
		{
			MyMessageBox_Error(_T("closeDeviceLink"));
			return FALSE;
		}
	}
	else
	{
		return TRUE;
	}
}

BOOL VNDriverLoaderEx::writeAndReadFromNdis(ULONG nCTLCode, UCHAR* pInData, int nInDataLen, UCHAR* pOutData, int nOutDataLen,
											int &nRetDataLen)
{
	if (!openDeviceLink())
	{
		//MyMessageBox_Error(_T("writeAndReadFromNdis"));
		return FALSE;
	}

	ULONG ulRetDataLen;
	BOOL bRet = ::DeviceIoControl(m_hDevice,
		nCTLCode,					// ���ܺ�
		pInData,					// ���뻺�壬Ҫ���ݵ���Ϣ��Ԥ�����
		nInDataLen,					// ���뻺�峤��
		pOutData,					// û���������
		nOutDataLen,				// �������ĳ���Ϊ0
		&ulRetDataLen,				// ���صĳ���	
		NULL);
	nRetDataLen = ulRetDataLen;

	if (bRet == 0)
	{
		MyMessageBox_Error(_T("writeAndReadFromNdis"));
		closeDeviceLink();
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL VNDriverLoaderEx::writeToNdis(ULONG nCTLCode, UCHAR* pData, int nDataLen, int &nRetDataLen)
{
	if (!openDeviceLink())
	{
		//MyMessageBox_Error(_T("writeToNdis"));
		return FALSE;
	}
	
	ULONG ulRetDataLen;
	BOOL bRet = ::DeviceIoControl(m_hDevice,
		nCTLCode,					// ���ܺ�
		NULL,						// 
		0,							// 
		pData,						// 
		nDataLen,					// 
		&ulRetDataLen,				// ���صĳ���	
		NULL);
	nRetDataLen = ulRetDataLen;
	
	if (bRet == 0)
	{
		MyMessageBox_Error(_T("writeToNdis"));
		closeDeviceLink();
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL VNDriverLoaderEx::readFromNdis(ULONG nCTLCode, UCHAR* pData, int nDataLen, int &nRetDataLen)
{
	if (!openDeviceLink())
	{
		//MyMessageBox_Error(_T("readFromNdis"));
		return FALSE;
	}

	ULONG ulRetDataLen;
	BOOL bRet = ::DeviceIoControl(m_hDevice,
		nCTLCode,					// ���ܺ�
		NULL,						// 
		0,							// 
		pData,						// 
		nDataLen,					// 
		&ulRetDataLen,				// ���صĳ���	
		NULL);
	nRetDataLen = ulRetDataLen;

	if (bRet == 0)
	{
		MyMessageBox_Error(_T("readFromNdis"));
		closeDeviceLink();
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL VNDriverLoaderEx::queryAlive()
{
	UCHAR pInputBuffer[100];
	int iRetBufLen = 0;
	readFromNdis(IOCTL_VNCIO_SIMPLE_READ, pInputBuffer, 100, iRetBufLen);
	if (iRetBufLen > 0)
	{
		CString strHelloWord = pInputBuffer; //"hello from vncpassthru"
		if (strHelloWord == _T("hello from vncpassthru"))
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}