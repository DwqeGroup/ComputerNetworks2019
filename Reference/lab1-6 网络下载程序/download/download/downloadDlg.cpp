
// downloadDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "download.h"
#include "downloadDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CdownloadDlg �Ի���
extern int download(char* url, char* path, char* name);



CdownloadDlg::CdownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdownloadDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, url_edit);
	DDX_Control(pDX, IDC_EDIT2, path_edit);
	DDX_Control(pDX, IDC_EDIT3, name_edit);
}

BEGIN_MESSAGE_MAP(CdownloadDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CdownloadDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CdownloadDlg ��Ϣ�������

BOOL CdownloadDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CdownloadDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CdownloadDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CdownloadDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	char url[130] = {0};
	char path[260] = {0};
	char name[260] = {0};
	CString text;

	url_edit.GetWindowTextA(text);
	strcpy(url,(LPSTR)(LPCTSTR)text);

	path_edit.GetWindowTextA(text);
	strcpy(path,(LPSTR)(LPCTSTR)text);

	name_edit.GetWindowTextA(text);
	strcpy(name,(LPSTR)(LPCTSTR)text);

	download(url, path, name);
}
