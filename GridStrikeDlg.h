// GridStrikeDlg.h : header file
//

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <thread>
#include <atlimage.h>

struct PathResult
{
    CString algorithm;
    int manhattan;
    int steps;
};

extern std::map<CString, std::vector<PathResult>> algoResults;
#define WM_SIMULATION_DONE (WM_USER + 101)

// CGridStrikeDlg dialog
class CGridStrikeDlg : public CDialogEx
{
public:
    CGridStrikeDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_GRIDSTRIKE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    // State and Rendering Context
    bool bCompleteChart = false;
    HICON m_hIcon;
    HGLRC m_hRC, m_chartRC;
    HDC m_hDC, m_chartDC;
    CWnd* m_pGLWnd = nullptr, * m_pChartWnd = nullptr;
    CStatic m_glControl;

    // UI Elements
    CListBox m_algoList;
    CEdit m_statusBox;

    // Map and Path Data
    std::vector<std::vector<int>> m_mapData;
    std::pair<int, int> startPos;
    std::pair<int, int> endPos;
    std::vector<std::pair<int, int>> path;

    // Simulation
    int m_runCount = 1;
    std::atomic<bool> m_cancelSim = false;
    std::thread m_simThread;
    bool m_runningSim = false;
    bool m_readyToSimulate = true;

    // Icons
    CImage m_startIcon;
    CImage m_endIcon;

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnEnableRun(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSimulationDone(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    DECLARE_MESSAGE_MAP()

    void OnDestroy();
    void AppendStatus(const CString& msg);
    void ClearStatus();

    // Logic
    bool LoadMapFromFile(const std::string& filename);
    void FindStartAndEnd();
    void RunSimulations();
    std::pair<int, int> GenerateRandomWalkable();
    std::vector<std::pair<int, int>> AStarPathfind();
    std::vector<std::pair<int, int>> GreedyBestFirst();

    // Drawing
    bool InitChartOpenGL();
    void DrawMap();
    void DrawGridMap(bool showPath);
    void DrawChart();
    void DrawAxisTicksAndLabels(int maxDistance, int maxSteps);
    void DrawLegendBox(CDC* pDC, int width, int height, const std::vector<CString>& algoNames);
    void DrawLegendBoxGDI();
    void DrawCoordinateFrame(int maxDistance, int maxSteps);
    int RoundDown(int value);
    int ComputeStep(int maxVal, int divisions);

public:
    afx_msg void OnBnClickedCompute();
};
