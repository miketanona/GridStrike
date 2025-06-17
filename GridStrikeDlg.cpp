
// GridStrikeDlg.cpp : implementation file
//

#pragma warning(disable : 3177)  


#include "pch.h"
#include "framework.h"
#include "GridStrike.h"
#include "GridStrikeDlg.h"
#include "afxdialogex.h"
#include "GridChartRenderer.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <queue>
#include <unordered_map>

#include <random>
#include <iostream>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <algorithm>

//#include "thirdparty/json/json.hpp"  // adjust path as needed
#include "thirdparty/json/json.hpp"
#include <fstream>
#include <vector>

using json = nlohmann::json;

// Global or member-level 2D map array
std::vector<std::vector<int>> mapData; // or make this a member variable if you prefer

std::map<CString, std::vector<PathResult>> algoResults;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CGridStrikeDlg dialog

CGridStrikeDlg::CGridStrikeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GRIDSTRIKE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}



std::vector<std::pair<int, int>> CGridStrikeDlg::AStarPathfind()
{
	using P = std::pair<int, int>;
	auto [sx, sy] = startPos;
	auto [ex, ey] = endPos;

	P cameFrom[32 * 32];
	int costSoFar[32 * 32];
	std::fill_n(cameFrom, 32 * 32, P(-1, -1));
	std::fill_n(costSoFar, 32 * 32, -1);

	auto hash = [](int x, int y) { return y * 32 + x; };
	auto heuristic = [&](int x, int y)
		{
			return abs(ex - x) + abs(ey - y);
		};

	std::priority_queue<std::tuple<int, int, int>, std::vector<std::tuple<int, int, int>>, std::greater<>> frontier;
	frontier.emplace(0, sx, sy);
	cameFrom[hash(sx, sy)] = { -1, -1 };
	costSoFar[hash(sx, sy)] = 0;

	const int dx[] = { 1, -1, 0, 0 };
	const int dy[] = { 0, 0, 1, -1 };

	while (!frontier.empty())
	{
		auto [_, x, y] = frontier.top(); frontier.pop();
		if (x == ex && y == ey) break;

		for (int d = 0; d < 4; ++d)
		{
			int nx = x + dx[d], ny = y + dy[d];
			if (nx < 0 || ny < 0 || nx >= 32 || ny >= 32) continue;
			if (m_mapData[ny][nx] == 3) continue;

			int newCost = costSoFar[hash(x, y)] + 1;
			int nid = hash(nx, ny);
			if (costSoFar[nid] == -1 || newCost < costSoFar[nid])
			{
				costSoFar[nid] = newCost;
				int priority = newCost + heuristic(nx, ny);
				frontier.emplace(priority, nx, ny);
				cameFrom[nid] = { x, y };
			}
		}
	}

	// Check if end was reached
	if (cameFrom[hash(ex, ey)] == P(-1, -1)) return {};

	std::vector<P> returnPath;
	P current = { ex, ey };
	while (current != P(-1, -1))
	{
		returnPath.push_back(current);
		current = cameFrom[hash(current.first, current.second)];
	}
	std::reverse(returnPath.begin(), returnPath.end());
	return returnPath;
}



std::vector<std::pair<int, int>> CGridStrikeDlg::GreedyBestFirst()
{
	using P = std::pair<int, int>;
	int sx = startPos.first, sy = startPos.second;
	int ex = endPos.first, ey = endPos.second;

	// Hash function: unique ID for each grid cell
	auto hash = [](int x, int y) { return y * 32 + x; };
	auto heuristic = [&](int x, int y)
		{
			return abs(ex - x) + abs(ey - y);  // Manhattan distance
		};

	// Fixed-size storage to avoid unordered_map overhead
	P cameFrom[32 * 32];
	std::fill_n(cameFrom, 32 * 32, P(-1, -1));
	bool visited[32 * 32] = { false };

	// Min-heap priority queue: (heuristic, x, y)
	std::priority_queue<
		std::tuple<int, int, int>,
		std::vector<std::tuple<int, int, int>>,
		std::greater<>
	> frontier;

	frontier.emplace(heuristic(sx, sy), sx, sy);
	cameFrom[hash(sx, sy)] = P(-1, -1);
	visited[hash(sx, sy)] = true;

	const int dx[] = { 1, -1, 0, 0 };
	const int dy[] = { 0, 0, 1, -1 };

	while (!frontier.empty())
	{
		auto [_, x, y] = frontier.top(); frontier.pop();
		if (x == ex && y == ey) break;

		for (int d = 0; d < 4; ++d)
		{
			int nx = x + dx[d], ny = y + dy[d];
			if (nx < 0 || ny < 0 || nx >= 32 || ny >= 32) continue;
			if (m_mapData[ny][nx] == 3) continue;

			int nid = hash(nx, ny);
			if (!visited[nid])
			{
				visited[nid] = true;
				frontier.emplace(heuristic(nx, ny), nx, ny);
				cameFrom[nid] = { x, y };
			}
		}
	}

	// Check if we reached the goal
	int eid = hash(ex, ey);
	if (cameFrom[eid] == P(-1, -1)) return {}; // No path found

	std::vector<P> pathReturn;
	P cur = { ex, ey };
	while (cur != P(-1, -1))
	{
		pathReturn.push_back(cur);
		cur = cameFrom[hash(cur.first, cur.second)];
	}

	std::reverse(pathReturn.begin(), pathReturn.end());
	return pathReturn;
}


bool CGridStrikeDlg::LoadMapFromFile(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) return false;

	json j;
	file >> j;

	auto data = j["layers"][0]["data"];
	m_mapData.resize(32, std::vector<int>(32));
	for (int i = 0; i < 32; ++i)
		for (int jx = 0; jx < 32; ++jx)
			m_mapData[i][jx] = data[i * 32 + jx];

	return true;
}


void CGridStrikeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_OPENGL_VIEW, m_glControl);
	DDX_Control(pDX, IDC_LB_ALGORITHM, m_algoList);
	DDX_Text(pDX, IDC_TB_COUNT, m_runCount);
	DDV_MinMaxInt(pDX, m_runCount, 1, 10000); // optional: validation
	DDX_Control(pDX, IDC_TX_OUTPUT, m_statusBox);
}


BEGIN_MESSAGE_MAP(CGridStrikeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_APP + 1, &CGridStrikeDlg::OnEnableRun)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_COMPUTE, &CGridStrikeDlg::OnBnClickedCompute)
	ON_MESSAGE(WM_SIMULATION_DONE, &CGridStrikeDlg::OnSimulationDone)
END_MESSAGE_MAP()


// CGridStrikeDlg message handlers


BOOL CGridStrikeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//GetDlgItem(IDC_COMPUTE)->EnableWindow(FALSE);

	HWND hwnd = GetSafeHwnd();
	if (::IsWindow(hwnd))
	{
		CString msg;
		msg.Format(_T("Dialog HWND is valid: 0x%p\n"), hwnd);
		OutputDebugString(msg);
	}
	else
	{
		OutputDebugString(_T("❗ Dialog HWND is NOT valid!\n"));
	}

	m_readyToSimulate = false;


	m_algoList.AddString(_T("AStar"));
	m_algoList.AddString(_T("Greedy"));
	//m_algoList.AddString(_T("Dijkstra")); // future

	m_runCount = 1;
	UpdateData(FALSE);  // pushes m_runCount → edit box

	if (!InitChartOpenGL())
	{
		AfxMessageBox(_T("Failed to initialize OpenGL for chart control!"));
	}

	GetDlgItem(IDC_OPENGL_CHART)->ShowWindow(SW_HIDE);

	// Get OpenGL target control
	m_pGLWnd = GetDlgItem(IDC_OPENGL_VIEW);
	if (!m_pGLWnd || !::IsWindow(m_pGLWnd->GetSafeHwnd()))
	{
		AfxMessageBox(_T("OpenGL control (IDC_OPENGL_VIEW) not found!"));
		return FALSE;
	}

	// Get device context
	m_hDC = ::GetDC(m_pGLWnd->GetSafeHwnd());
	if (!m_hDC)
	{
		AfxMessageBox(_T("Failed to get DC"));
		return FALSE;
	}

	// Set pixel format
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		24, 8, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	int iFormat = ChoosePixelFormat(m_hDC, &pfd);
	if (iFormat == 0)
	{
		AfxMessageBox(_T("Failed to choose pixel format"));
		return FALSE;
	}

	if (!SetPixelFormat(m_hDC, iFormat, &pfd))
	{
		AfxMessageBox(_T("Failed to set pixel format"));
		return FALSE;
	}

	// Create and activate OpenGL context
	m_hRC = wglCreateContext(m_hDC);
	if (!m_hRC)
	{
		AfxMessageBox(_T("Failed to create OpenGL context"));
		return FALSE;
	}

	if (!wglMakeCurrent(m_hDC, m_hRC))
	{
		AfxMessageBox(_T("Failed to activate OpenGL context"));
		return FALSE;
	}

	if (!LoadMapFromFile("map.json"))
	{
		AfxMessageBox(_T("Failed to load map.json"));
	}

	DrawGridMap(false);

	UINT_PTR timerId = SetTimer(1, 2000, nullptr);  // 300 ms

	if (timerId == 0)
	{
		OutputDebugString(_T("SetTimer failed!\n"));
	}
	else
	{
		CString msg;
		msg.Format(_T("SetTimer succeeded with ID = %u\n"), timerId);
		OutputDebugString(msg);
	};

	m_readyToSimulate = true;

	return TRUE;
}


void CGridStrikeDlg::OnTimer(UINT_PTR nIDEvent)
{
	OutputDebugString(L"OnTimer succeeded\n");

	if (nIDEvent == 1)
	{
		GetDlgItem(IDC_COMPUTE)->EnableWindow(TRUE);
		KillTimer(1);
		OutputDebugString(L"KillTimer succeeded\n");
	}
	CDialogEx::OnTimer(nIDEvent);
}


LRESULT CGridStrikeDlg::OnEnableRun(WPARAM, LPARAM)
{
	GetDlgItem(IDC_COMPUTE)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMPUTE)->Invalidate();    // Marks it for redraw
	GetDlgItem(IDC_COMPUTE)->UpdateWindow();  // Forces immediate repaint
	OutputDebugString(L"Run button enabled after delay\n");
	return 0;
}


bool CGridStrikeDlg::InitChartOpenGL()
{
	m_pChartWnd = GetDlgItem(IDC_OPENGL_CHART);

	if (!m_pChartWnd || !::IsWindow(m_pChartWnd->GetSafeHwnd()))
		return false;

	m_chartDC = ::GetDC(m_pChartWnd->GetSafeHwnd());

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		24, 8, 0,
		PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	int iPixelFormat = ChoosePixelFormat(m_chartDC, &pfd);
	if (iPixelFormat == 0) return false;

	if (!SetPixelFormat(m_chartDC, iPixelFormat, &pfd))
		return false;

	m_chartRC = wglCreateContext(m_chartDC);
	if (!m_chartRC) return false;

	if (!wglMakeCurrent(m_chartDC, m_chartRC))
		return false;

	// Optional: Set OpenGL state
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // black background

	return true;
}





void CGridStrikeDlg::DrawGridMap(bool bDrawPath)
{
	GridChartRenderer renderer;
	renderer.DrawGridMap(m_hDC, m_hRC, m_pGLWnd, m_mapData, path, startPos, endPos, bDrawPath);
}



void CGridStrikeDlg::DrawChart()
{
	GridChartRenderer renderer;
	renderer.DrawChart(m_chartDC, m_chartRC, m_pChartWnd, algoResults);
}




int CGridStrikeDlg::ComputeStep(int maxVal, int divisions)
{
	int rawStep = maxVal / divisions;
	int logVal = static_cast<int>(std::log10(static_cast<double>(rawStep)));
	int magnitude = static_cast<int>(std::pow(10.0, logVal));
	int residual = rawStep / magnitude;

	int niceStep = 0;
	if (residual <= 1)
		niceStep = 1 * magnitude;
	else if (residual <= 2)
		niceStep = 2 * magnitude;
	else if (residual <= 5)
		niceStep = 5 * magnitude;
	else
		niceStep = 10 * magnitude;

	return niceStep;
}


void CGridStrikeDlg::DrawAxisTicksAndLabels(int maxDistance, int maxSteps)
{
	if (!m_pChartWnd) return;

	CClientDC dc(m_pChartWnd);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(255, 255, 255)); // white text

	CRect rect;
	m_pChartWnd->GetClientRect(&rect);
	int width = rect.Width();
	int height = rect.Height();

	const int left = 40;
	const int bottom = height - 40;
	const float tileWidth = static_cast<float>(width - 50);
	const float tileHeight = static_cast<float>(height - 50);

	const int xStepVal = RoundDown(maxDistance);
	const int yStepVal = RoundDown(maxSteps);

	const int numX = maxDistance / xStepVal;
	const int numY = maxSteps / yStepVal;

	// X Axis
	for (int i = 0; i <= numX; ++i)
	{
		int val = i * xStepVal;
		float norm = static_cast<float>(val) / (1.2f * std::max(1, maxDistance));
		int x = static_cast<int>(left + norm * tileWidth);

		dc.MoveTo(x, bottom - 5);
		dc.LineTo(x, bottom + 5);

		CString label;
		label.Format(_T("%d"), val);
		dc.TextOutW(x - 10, bottom + 8, label);
	}

	// Y Axis
	for (int i = 0; i <= numY; ++i)
	{
		int val = i * yStepVal;
		float norm = static_cast<float>(val) / (1.2f * std::max(1, maxSteps));
		int y = static_cast<int>(bottom - norm * tileHeight);

		dc.MoveTo(left - 5, y);
		dc.LineTo(left + 5, y);

		CString label;
		label.Format(_T("%d"), val);
		dc.TextOutW(5, y - 8, label);
	}
}


int CGridStrikeDlg::RoundDown(int value)
{
	if (value <= 10) return 2;
	if (value <= 20) return 5;
	if (value <= 50) return 10;
	if (value <= 100) return 20;
	if (value <= 200) return 40;
	if (value <= 500) return 100;
	return 200;
}



void CGridStrikeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



void CGridStrikeDlg::OnPaint()
{
	if (IsIconic())
	{
		CDialogEx::OnPaint(); // draw icon
	}
	else
	{
		CDialogEx::OnPaint();
		DrawGridMap(false);  // always repaint OpenGL content
	}
}


void CGridStrikeDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// release grid
	if (m_hRC)
	{
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(m_hRC);
	}

	if (m_hDC && m_pGLWnd)
		::ReleaseDC(m_pGLWnd->GetSafeHwnd(), m_hDC);

	// chart
	if (wglGetCurrentContext() == m_chartRC)
		wglMakeCurrent(nullptr, nullptr);

	if (m_chartRC)
	{
		wglDeleteContext(m_chartRC);
		m_chartRC = nullptr;
	}

	if (m_pChartWnd && m_chartDC)
	{
		::ReleaseDC(m_pChartWnd->GetSafeHwnd(), m_chartDC);
		m_chartDC = nullptr;
	}
}



// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGridStrikeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGridStrikeDlg::OnBnClickedCompute()
{
	ClearStatus();

	if (!m_readyToSimulate)
	{
		AppendStatus(_T("App not ready yet!"));
		return;
	}

	if (m_runningSim)
	{
		// Stop running simulation
		m_cancelSim = true;
		if (m_simThread.joinable())
		{
			m_simThread.join();
		}
		m_runningSim = false;
		SetDlgItemText(IDC_COMPUTE, _T("Run"));
		AppendStatus(_T("Simulation stopped."));
		return;
	}

	// Start new simulation
	UpdateData(TRUE); // Get m_runCount from textbox

	m_cancelSim = false;
	m_runningSim = true;

	m_simThread = std::thread([this]()
		{
			RunSimulations();
		});

}

LRESULT CGridStrikeDlg::OnSimulationDone(WPARAM, LPARAM)
{
	OutputDebugString(L"OnSimulationDone...\n");

	if (m_simThread.joinable())
	{
		m_simThread.join();  // 🔒 Ensure thread is fully done writing
	}

	m_runningSim = false;
	SetDlgItemText(IDC_COMPUTE, _T("Run"));

	if (m_runCount == 1 && m_algoList.GetSelCount() == 1)
	{
		GetDlgItem(IDC_OPENGL_CHART)->ShowWindow(SW_HIDE);

		DrawGridMap(true);

		// We'll assume the path for this simulation was stored in a global or accessible member: `lastPath`
		AppendStatus(_T("Path taken:"));
		CString line;
		for (const auto& pos : path)
		{
			CString msg;
			msg.Format(_T("(%d, %d) "), pos.first, pos.second);
			line += msg;
		}
		AppendStatus(line);
	}
	else
	{
		GetDlgItem(IDC_OPENGL_CHART)->ShowWindow(SW_SHOW);
		DrawChart();       // Draw performance chart
		path.clear();
		m_pChartWnd->Invalidate();
		m_pChartWnd->UpdateWindow();
	}

	return 0;
}



void CGridStrikeDlg::RunSimulations()
{
	CString momessage;

	algoResults.clear();

	CArray<int, int> selected;
	selected.SetSize(m_algoList.GetSelCount());
	m_algoList.GetSelItems(m_algoList.GetSelCount(), selected.GetData());

	for (int i = 0; i < m_runCount && !m_cancelSim; ++i)
	{

		std::vector<std::pair<int, int>> m_lastPath;

		if (selected.GetCount() == 1 && m_runCount == 1)
		{
			FindStartAndEnd();
		}
		else
		{
			// Random valid start/end
			auto start = GenerateRandomWalkable();
			auto end = GenerateRandomWalkable();

			while (start == end)
				end = GenerateRandomWalkable();

			startPos = start;
			endPos = end;
		}


		for (int j = 0; j < selected.GetCount(); ++j)
		{
			CString algoName;
			m_algoList.GetText(selected[j], algoName);

			if (algoName == _T("AStar"))
			{
				m_lastPath = AStarPathfind();
			}
			else if (algoName == _T("Greedy"))
			{
				m_lastPath = GreedyBestFirst();
			}

			int steps = static_cast<int>(m_lastPath.size());
			int manhattan = abs(startPos.first - endPos.first) + abs(startPos.second - endPos.second);

			if (selected.GetCount() == 1 && m_runCount == 1)
			{
				// update global path for drawing
				path = m_lastPath;
			}

			algoResults[algoName].push_back({ algoName, manhattan, steps });
		}
	}

	OutputDebugString(L"PostMessage(WM_SIMULATION_DONE...\n");
	PostMessage(WM_SIMULATION_DONE, 0, 0); // notify main thread
}



std::pair<int, int> CGridStrikeDlg::GenerateRandomWalkable()
{
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(0, 31);

	for (int attempts = 0; attempts < 1000; ++attempts)
	{
		int x = dist(rng);
		int y = dist(rng);
		if (m_mapData[y][x] == -1)
		{
			return { x, y };
		}
	}

	return { 0, 0 }; // fallback if none found
}



void CGridStrikeDlg::FindStartAndEnd()
{

	BeginWaitCursor();         //  Show wait

	for (int y = 0; y < 32; ++y)
	{
		for (int x = 0; x < 32; ++x)
		{
			if (m_mapData[y][x] == 0) startPos = { x, y };
			if (m_mapData[y][x] == 8) endPos = { x, y };
		}
	}

	EndWaitCursor();           // back to normal
}


void CGridStrikeDlg::AppendStatus(const CString& msg)
{
	CString current;
	m_statusBox.GetWindowText(current);
	current += msg + _T("\r\n");
	m_statusBox.SetWindowText(current);
	m_statusBox.LineScroll(m_statusBox.GetLineCount()); // optional: auto-scroll to bottom
	m_statusBox.RedrawWindow();                         // force update if needed
}

void CGridStrikeDlg::ClearStatus()
{
	CString current;;
	m_statusBox.SetWindowText(current);
	m_statusBox.LineScroll(0); // optional: auto-scroll to bottom
	m_statusBox.RedrawWindow();                         // force update if needed
}



//void CGridStrikeDlg::DrawGridMap(bool bDrawPath)
//{
//	if (!wglMakeCurrent(m_hDC, m_hRC)) return;
//
//	OutputDebugString(L"DrawGridMap...\n");
//
//	CRect rect;
//	m_pGLWnd->GetClientRect(&rect);
//	int width = rect.Width();
//	int height = rect.Height();
//
//	float tileSize = std::min(width / 32.0f, height / 32.0f);
//
//	glViewport(0, 0, width, height);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(0, 32, 32, 0, -1, 1);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	for (int y = 0; y < 32; ++y)
//	{
//		for (int x = 0; x < 32; ++x)
//		{
//			int val = m_mapData[y][x];
//
//			float left = x * tileSize;
//			float top = y * tileSize;
//			float right = left + tileSize;
//			float bottom = top + tileSize;
//
//			switch (val)
//			{
//			case -1: case 0: case 8:
//				glColor3f(0.95f, 0.95f, 0.95f); break;
//			case 3:
//				glColor3f(0.0f, 0.6f, 0.0f); break;
//			default:
//				glColor3f(0.5f, 0.5f, 0.5f); break;
//			}
//
//			glBegin(GL_QUADS);
//			glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y));
//			glVertex2f(static_cast<GLfloat>(x + 1), static_cast<GLfloat>(y));
//			glVertex2f(static_cast<GLfloat>(x + 1), static_cast<GLfloat>(y + 1));
//			glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y + 1));
//			glEnd();
//
//			if (val == 3 || val == 0 || val == 8)
//			{
//				if (val == 3)
//					glColor3f(0.0f, 0.3f, 0.0f);
//				else if (val == 0)
//					glColor3f(0.5f, 0.3f, 0.1f);
//				else if (val == 8)
//					glColor3f(1.0f, 0.0f, 0.0f);
//
//				glBegin(GL_QUADS);
//				glVertex2f(static_cast<GLfloat>(x) + 0.3f, static_cast<GLfloat>(y) + 0.3f);
//				glVertex2f(static_cast<GLfloat>(x) + 0.7f, static_cast<GLfloat>(y) + 0.3f);
//				glVertex2f(static_cast<GLfloat>(x) + 0.7f, static_cast<GLfloat>(y) + 0.7f);
//				glVertex2f(static_cast<GLfloat>(x) + 0.3f, static_cast<GLfloat>(y) + 0.7f);
//				glEnd();
//			}
//		}
//	}
//
//	glColor3f(0.6f, 0.6f, 0.6f);
//	glBegin(GL_LINES);
//	for (int i = 0; i <= 32; ++i)
//	{
//		glVertex2f(static_cast<GLfloat>(i), 0.0f);
//		glVertex2f(static_cast<GLfloat>(i), 32.0f);
//		glVertex2f(0.0f, static_cast<GLfloat>(i));
//		glVertex2f(32.0f, static_cast<GLfloat>(i));
//	}
//	glEnd();
//
//	if (bDrawPath)
//	{
//		for (const auto& [x, y] : path)
//		{
//			glColor3f(1.0f, 0.0f, 0.0f);
//			glBegin(GL_QUADS);
//			glVertex2f(static_cast<GLfloat>(x) + 0.1f, static_cast<GLfloat>(y) + 0.1f);
//			glVertex2f(static_cast<GLfloat>(x) + 0.9f, static_cast<GLfloat>(y) + 0.1f);
//			glVertex2f(static_cast<GLfloat>(x) + 0.9f, static_cast<GLfloat>(y) + 0.9f);
//			glVertex2f(static_cast<GLfloat>(x) + 0.1f, static_cast<GLfloat>(y) + 0.9f);
//			glEnd();
//		}
//
//		auto [ex, ey] = endPos;
//		glColor3f(0.95f, 0.95f, 0.95f);
//		glBegin(GL_QUADS);
//		glVertex2f(static_cast<GLfloat>(ex), static_cast<GLfloat>(ey));
//		glVertex2f(static_cast<GLfloat>(ex + 1), static_cast<GLfloat>(ey));
//		glVertex2f(static_cast<GLfloat>(ex + 1), static_cast<GLfloat>(ey + 1));
//		glVertex2f(static_cast<GLfloat>(ex), static_cast<GLfloat>(ey + 1));
//		glEnd();
//
//		glColor3f(1.0f, 0.0f, 0.0f);
//		glBegin(GL_QUADS);
//		glVertex2f(static_cast<GLfloat>(ex) + 0.3f, static_cast<GLfloat>(ey) + 0.3f);
//		glVertex2f(static_cast<GLfloat>(ex) + 0.7f, static_cast<GLfloat>(ey) + 0.3f);
//		glVertex2f(static_cast<GLfloat>(ex) + 0.7f, static_cast<GLfloat>(ey) + 0.7f);
//		glVertex2f(static_cast<GLfloat>(ex) + 0.3f, static_cast<GLfloat>(ey) + 0.7f);
//		glEnd();
//
//		auto [sx, sy] = startPos;
//		glColor3f(0.95f, 0.95f, 0.95f);
//		glBegin(GL_QUADS);
//		glVertex2f(static_cast<GLfloat>(sx), static_cast<GLfloat>(sy));
//		glVertex2f(static_cast<GLfloat>(sx + 1), static_cast<GLfloat>(sy));
//		glVertex2f(static_cast<GLfloat>(sx + 1), static_cast<GLfloat>(sy + 1));
//		glVertex2f(static_cast<GLfloat>(sx), static_cast<GLfloat>(sy + 1));
//		glEnd();
//
//		glColor3f(0.5f, 0.3f, 0.1f);
//		glBegin(GL_QUADS);
//		glVertex2f(static_cast<GLfloat>(sx) + 0.3f, static_cast<GLfloat>(sy) + 0.3f);
//		glVertex2f(static_cast<GLfloat>(sx) + 0.7f, static_cast<GLfloat>(sy) + 0.3f);
//		glVertex2f(static_cast<GLfloat>(sx) + 0.7f, static_cast<GLfloat>(sy) + 0.7f);
//		glVertex2f(static_cast<GLfloat>(sx) + 0.3f, static_cast<GLfloat>(sy) + 0.7f);
//		glEnd();
//	}
//
//	SwapBuffers(m_hDC);
//}
// 
// 
//void CGridStrikeDlg::DrawChart()
//{
//	if (!wglMakeCurrent(m_chartDC, m_chartRC)) return;
//
//	CRect rect;
//	m_pChartWnd->GetClientRect(&rect);
//	int width = rect.Width();
//	int height = rect.Height();
//
//	const int leftMargin = 50;
//	const int rightMargin = 20;
//	const int topMargin = 20;
//	const int bottomMargin = 50;
//
//	glViewport(0, 0, width, height);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(0.0, static_cast<GLdouble>(width), static_cast<GLdouble>(height), 0.0, -1.0, 1.0);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//	glClearColor(0, 0, 0, 1);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	GLfloat xAxisY = static_cast<GLfloat>(height - bottomMargin);
//	GLfloat yAxisX = static_cast<GLfloat>(leftMargin);
//
//	glColor3f(1.0f, 1.0f, 1.0f);
//	glBegin(GL_LINES);
//	glVertex2f(static_cast<GLfloat>(leftMargin), xAxisY);
//	glVertex2f(static_cast<GLfloat>(width - rightMargin), xAxisY);
//	glVertex2f(yAxisX, static_cast<GLfloat>(height - bottomMargin));
//	glVertex2f(yAxisX, static_cast<GLfloat>(topMargin));
//	glEnd();
//
//	int maxDistance = 1;
//	int maxSteps = 1;
//	for (const auto& [name, results] : algoResults)
//	{
//		for (const auto& r : results)
//		{
//			maxDistance = std::max(maxDistance, r.manhattan);
//			maxSteps = std::max(maxSteps, r.steps);
//		}
//	}
//
//	int colorIndex = 0;
//	for (const auto& [name, results] : algoResults)
//	{
//		if (colorIndex == 0)
//			glColor3f(1.0f, 0.0f, 0.0f);
//		else
//			glColor3f(0.2f, 0.2f, 1.0f);
//
//		glPointSize(6.0f);
//		glBegin(GL_POINTS);
//
//		for (const auto& pt : results)
//		{
//			float plotWidth = static_cast<float>(width - leftMargin - rightMargin);
//			float plotHeight = static_cast<float>(height - topMargin - bottomMargin);
//			float padding = 0.02f;
//
//			float normManhattan = static_cast<float>(pt.manhattan) / (1.2f * std::max(maxDistance, 1));
//			float normSteps = static_cast<float>(pt.steps) / (1.2f * std::max(maxSteps, 1));
//
//			normManhattan = std::clamp(normManhattan, padding, 1.0f - padding);
//			normSteps = std::clamp(normSteps, padding, 1.0f - padding);
//
//			float x = static_cast<float>(leftMargin) + normManhattan * plotWidth;
//			float y = static_cast<float>(height - bottomMargin) - normSteps * plotHeight;
//
//			glVertex2f(x, y);
//		}
//		glEnd();
//
//		colorIndex++;
//	}
//
//	std::vector<CString> algoNames;
//	for (const auto& [name, _] : algoResults)
//		algoNames.push_back(name);
//
//	SwapBuffers(m_chartDC);
//	DrawCoordinateFrame(maxDistance, maxSteps);
//	DrawLegendBoxGDI();
//}

//void CGridStrikeDlg::DrawCoordinateFrame(int maxDistance, int maxSteps)
//{
//	if (!m_pChartWnd) return;
//
//	CDC* pDC = m_pChartWnd->GetDC();
//	if (!pDC) return;
//
//	CRect rect;
//	m_pChartWnd->GetClientRect(&rect);
//	int width = rect.Width();
//	int height = rect.Height();
//
//	const int leftMargin = 50;
//	const int rightMargin = 20;
//	const int topMargin = 20;
//	const int bottomMargin = 40;
//
//	float plotWidth = static_cast<float>(width - leftMargin - rightMargin);
//	float plotHeight = static_cast<float>(height - topMargin - bottomMargin);
//
//	pDC->SetBkMode(TRANSPARENT);
//	pDC->SetTextColor(RGB(230, 230, 230));
//
//	int yStep = ComputeStep(maxSteps, 5);
//	for (int val = 0; val <= maxSteps; val += yStep)
//	{
//		float norm = static_cast<float>(val) / std::max(1, maxSteps);
//		int y = static_cast<int>(height - bottomMargin - norm * plotHeight);
//		CString label; label.Format(_T("%d"), val);
//		pDC->TextOut(leftMargin - 35, y - 6, label);
//	}
//
//	int xStep = ComputeStep(maxDistance, 5);
//	for (int val = 0; val <= maxDistance; val += xStep)
//	{
//		float norm = static_cast<float>(val) / std::max(1, maxDistance);
//		int x = static_cast<int>(leftMargin + norm * plotWidth);
//		CString label; label.Format(_T("%d"), val);
//		pDC->TextOut(x - 10, height - bottomMargin, label);
//	}
//
//	pDC->TextOut(static_cast<int>(leftMargin + plotWidth / 2 - 40), height - 20, _T("Manhattan Distance"));
//	pDC->TextOut(5, static_cast<int>(topMargin + plotHeight / 2 - 10), _T("Steps"));
//
//	m_pChartWnd->ReleaseDC(pDC);
//}
//
//
//void CGridStrikeDlg::DrawLegendBoxGDI()
//{
//	if (!m_pChartWnd) return;
//
//	CClientDC dc(m_pChartWnd);  // MFC device context for chart window
//
//	// Set transparent background mode and text color
//	dc.SetBkMode(TRANSPARENT);
//	dc.SetTextColor(RGB(255, 255, 255));  // white text
//
//	// Draw legend background rectangle (optional)
//	CBrush brush(RGB(50, 50, 50));  // dark grey background
//	CRect legendRect(50, 20, 200, 80);  // box dimensions
//	dc.FillRect(&legendRect, &brush);
//	dc.DrawEdge(&legendRect, EDGE_RAISED, BF_RECT);
//
//	// Draw legend text
//	int x = 60, y = 30;
//	dc.TextOutW(x, y, _T("Red: AStar"));
//	dc.TextOutW(x, y + 20, _T("Blue: Greedy"));
//}
//
//
//
//
//void CGridStrikeDlg::DrawLegendBox(CDC* pDC, int width, int height, const std::vector<CString>& algoNames)
//{
//	const int boxX = 10;
//	const int boxY = 10;
//	const int boxW = 130;
//	const int lineH = 20;
//
//	// Draw background rectangle
//	glColor3f(0.1f, 0.1f, 0.1f);  // dark gray background
//	glBegin(GL_QUADS);
//	glVertex2f(static_cast<GLfloat>(boxX), static_cast<GLfloat>(boxY));
//	glVertex2f(static_cast<GLfloat>(boxX + boxW), static_cast<GLfloat>(boxY));
//	glVertex2f(static_cast<GLfloat>(boxX + boxW), static_cast<GLfloat>(boxY + static_cast<int>(algoNames.size()) * lineH + 10));
//	glVertex2f(static_cast<GLfloat>(boxX), static_cast<GLfloat>(boxY + static_cast<int>(algoNames.size()) * lineH + 10));
//	glEnd();
//
//	// Text color setup (whiteish)
//	COLORREF oldColor = pDC->SetTextColor(RGB(230, 230, 230));
//	int oldBkMode = pDC->SetBkMode(TRANSPARENT);
//
//	// Draw color swatches and text
//	for (int i = 0; i < static_cast<int>(algoNames.size()); ++i)
//	{
//		// Set color swatch
//		float r = (i == 0) ? 1.0f : 0.2f;
//		float g = (i == 1) ? 0.2f : 0.2f;
//		float b = (i == 1) ? 1.0f : 0.2f;
//		glColor3f(r, g, b);
//
//		GLfloat top = static_cast<GLfloat>(boxY + 5 + i * lineH);
//		GLfloat bottom = static_cast<GLfloat>(boxY + 18 + i * lineH);
//
//		glBegin(GL_QUADS);
//		glVertex2f(static_cast<GLfloat>(boxX + 5), top);
//		glVertex2f(static_cast<GLfloat>(boxX + 20), top);
//		glVertex2f(static_cast<GLfloat>(boxX + 20), bottom);
//		glVertex2f(static_cast<GLfloat>(boxX + 5), bottom);
//		glEnd();
//
//		pDC->TextOut(boxX + 25, boxY + 5 + i * lineH, algoNames[i]);
//	}
//
//	pDC->SetTextColor(oldColor);
//	pDC->SetBkMode(oldBkMode);
//}
