
// downloadDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CdownloadDlg �Ի���
class CdownloadDlg : public CDialogEx
{
// ����
public:
	CdownloadDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_DOWNLOAD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit url_edit;
	CEdit path_edit;
	CEdit name_edit;
};
