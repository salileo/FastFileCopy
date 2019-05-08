// FastFileCopyDlg.h : header file
//

#pragma once
#include "afxcmn.h"


// CFastFileCopyDlg dialog
class CFastFileCopyDlg : public CDialog
{
// Construction
public:
	CFastFileCopyDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FASTFILECOPY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:
	CString m_source;
	CString m_dest;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedStart();
	CProgressCtrl m_part1;
	CProgressCtrl m_part2;
	CProgressCtrl m_part3;
	CProgressCtrl m_part4;
	CProgressCtrl m_part5;
};
