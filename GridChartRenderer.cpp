
// GridStrikeDlg.cpp : implementation file
//

#pragma warning(disable : 3177)  


#include "pch.h"
#include "framework.h"
#include "GridStrike.h"
#include "GridStrikeDlg.h"
#include "GridChartRenderer.h"
#include "afxdialogex.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <queue>
#include <unordered_map>

#include <gl/GL.h>
#include <algorithm>
#include <cmath>

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


#include "GridChartRenderer.h"
#include <gl/GL.h>

void GridChartRenderer::DrawGridMap(HDC hDC, HGLRC hRC, CWnd* pGLWnd,
    const std::vector<std::vector<int>>& mapData,
    const std::vector<std::pair<int, int>>& path,
    const std::pair<int, int>& startPos,
    const std::pair<int, int>& endPos,
    bool bDrawPath)
{
    if (!wglMakeCurrent(hDC, hRC)) return;

    CRect rect;
    pGLWnd->GetClientRect(&rect);
    int width = rect.Width();
    int height = rect.Height();
    float tileSize = std::min(width / 32.0f, height / 32.0f);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 32, 32, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int y = 0; y < 32; ++y)
    {
        for (int x = 0; x < 32; ++x)
        {
            int val = mapData[y][x];

            switch (val)
            {
            case -1: case 0: case 8:
                glColor3f(0.95f, 0.95f, 0.95f); break;
            case 3:
                glColor3f(0.0f, 0.6f, 0.0f); break;
            default:
                glColor3f(0.5f, 0.5f, 0.5f); break;
            }

            glBegin(GL_QUADS);
            glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y));
            glVertex2f(static_cast<GLfloat>(x + 1), static_cast<GLfloat>(y));
            glVertex2f(static_cast<GLfloat>(x + 1), static_cast<GLfloat>(y + 1));
            glVertex2f(static_cast<GLfloat>(x), static_cast<GLfloat>(y + 1));
            glEnd();

            if (val == 3 || val == 0 || val == 8)
            {
                if (val == 3)
                    glColor3f(0.0f, 0.3f, 0.0f);
                else if (val == 0)
                    glColor3f(0.5f, 0.3f, 0.1f);
                else if (val == 8)
                    glColor3f(1.0f, 0.0f, 0.0f);

                glBegin(GL_QUADS);
                glVertex2f(static_cast<GLfloat>(x) + 0.3f, static_cast<GLfloat>(y) + 0.3f);
                glVertex2f(static_cast<GLfloat>(x) + 0.7f, static_cast<GLfloat>(y) + 0.3f);
                glVertex2f(static_cast<GLfloat>(x) + 0.7f, static_cast<GLfloat>(y) + 0.7f);
                glVertex2f(static_cast<GLfloat>(x) + 0.3f, static_cast<GLfloat>(y) + 0.7f);
                glEnd();
            }
        }
    }

    glColor3f(0.6f, 0.6f, 0.6f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 32; ++i)
    {
        glVertex2f(static_cast<GLfloat>(i), 0.0f);
        glVertex2f(static_cast<GLfloat>(i), 32.0f);
        glVertex2f(0.0f, static_cast<GLfloat>(i));
        glVertex2f(32.0f, static_cast<GLfloat>(i));
    }
    glEnd();

    if (bDrawPath)
    {
        for (const auto& [x, y] : path)
        {
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(static_cast<GLfloat>(x) + 0.1f, static_cast<GLfloat>(y) + 0.1f);
            glVertex2f(static_cast<GLfloat>(x) + 0.9f, static_cast<GLfloat>(y) + 0.1f);
            glVertex2f(static_cast<GLfloat>(x) + 0.9f, static_cast<GLfloat>(y) + 0.9f);
            glVertex2f(static_cast<GLfloat>(x) + 0.1f, static_cast<GLfloat>(y) + 0.9f);
            glEnd();
        }

        // End marker
        auto [ex, ey] = endPos;
        glColor3f(0.95f, 0.95f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(static_cast<GLfloat>(ex), static_cast<GLfloat>(ey));
        glVertex2f(static_cast<GLfloat>(ex + 1), static_cast<GLfloat>(ey));
        glVertex2f(static_cast<GLfloat>(ex + 1), static_cast<GLfloat>(ey + 1));
        glVertex2f(static_cast<GLfloat>(ex), static_cast<GLfloat>(ey + 1));
        glEnd();

        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(static_cast<GLfloat>(ex) + 0.3f, static_cast<GLfloat>(ey) + 0.3f);
        glVertex2f(static_cast<GLfloat>(ex) + 0.7f, static_cast<GLfloat>(ey) + 0.3f);
        glVertex2f(static_cast<GLfloat>(ex) + 0.7f, static_cast<GLfloat>(ey) + 0.7f);
        glVertex2f(static_cast<GLfloat>(ex) + 0.3f, static_cast<GLfloat>(ey) + 0.7f);
        glEnd();

        // Start marker
        auto [sx, sy] = startPos;
        glColor3f(0.95f, 0.95f, 0.95f);
        glBegin(GL_QUADS);
        glVertex2f(static_cast<GLfloat>(sx), static_cast<GLfloat>(sy));
        glVertex2f(static_cast<GLfloat>(sx + 1), static_cast<GLfloat>(sy));
        glVertex2f(static_cast<GLfloat>(sx + 1), static_cast<GLfloat>(sy + 1));
        glVertex2f(static_cast<GLfloat>(sx), static_cast<GLfloat>(sy + 1));
        glEnd();

        glColor3f(0.5f, 0.3f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(static_cast<GLfloat>(sx) + 0.3f, static_cast<GLfloat>(sy) + 0.3f);
        glVertex2f(static_cast<GLfloat>(sx) + 0.7f, static_cast<GLfloat>(sy) + 0.3f);
        glVertex2f(static_cast<GLfloat>(sx) + 0.7f, static_cast<GLfloat>(sy) + 0.7f);
        glVertex2f(static_cast<GLfloat>(sx) + 0.3f, static_cast<GLfloat>(sy) + 0.7f);
        glEnd();
    }

    SwapBuffers(hDC);

    //int maxDistance = 0;
    //int maxSteps = 0;
    //for (const auto& [name, results] : algoResults)
    //{
    //    for (const auto& r : results)
    //    {
    //        maxDistance = std::max(maxDistance, r.manhattan);
    //        maxSteps = std::max(maxSteps, r.steps);
    //    }
    //}

    //DrawCoordinateFrame(pGLWnd, width, height, maxDistance, maxSteps);

    //DrawLegendBoxGDI(pGLWnd, width, height, algoResults);
}

void GridChartRenderer::DrawLegendBoxGDI(CWnd* pWnd, int width, int height, const std::map<CString, std::vector<PathResult>>& algoResults)
{
    if (!pWnd) return;

    CClientDC dc(pWnd);
    const int boxX = width - 150;
    const int boxY = 30;
    const int boxW = 120;
    const int lineH = 20;

    // Draw dark background rectangle
    dc.FillSolidRect(boxX, boxY, boxW, static_cast<int>(algoResults.size()) * lineH + 10, RGB(30, 30, 30));
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(255, 255, 255));

    int index = 0;
    for (const auto& [name, _] : algoResults)
    {
        // Alternate colors depending on index
        COLORREF color = (index == 0) ? RGB(255, 0, 0) : RGB(0, 100, 255);

        // Swatch box
        dc.FillSolidRect(boxX + 5, boxY + 5 + index * lineH, 12, 12, color);

        // Label
        dc.TextOutW(boxX + 22, boxY + 3 + index * lineH, name);
        ++index;
    }
}




//void GridChartRenderer::DrawGridMap(bool bDrawPath)
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


void GridChartRenderer::DrawCoordinateFrame(CWnd* pWnd, int width, int height, int maxDistance, int maxSteps)
{
    if (!pWnd) return;

    CDC* pDC = pWnd->GetDC();
    if (!pDC) return;

    const int leftMargin = 50;
    const int rightMargin = 20;
    const int topMargin = 20;
    const int bottomMargin = 40;

    float plotWidth = static_cast<float>(width - leftMargin - rightMargin);
    float plotHeight = static_cast<float>(height - topMargin - bottomMargin);

    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(RGB(230, 230, 230));

    int yStep = std::max(1, maxSteps / 5);
    for (int val = 0; val <= maxSteps; val += yStep)
    {
        float norm = static_cast<float>(val) / std::max(1, maxSteps);
        int y = static_cast<int>(height - bottomMargin - norm * plotHeight);
        CString label; label.Format(_T("%d"), val);
        pDC->TextOut(15, y - 6, label);
    }

    int xStep = std::max(1, maxDistance / 5);
    for (int val = 0; val <= maxDistance; val += xStep)
    {
        float norm = static_cast<float>(val) / std::max(1, maxDistance);
        int x = static_cast<int>(leftMargin + norm * plotWidth);
        CString label; label.Format(_T("%d"), val);
        pDC->TextOut(x - 10, height - bottomMargin + 8, label);
    }

    // Axis labels
    pDC->TextOut(static_cast<int>(leftMargin + plotWidth / 2 - 40), height - 20, _T("Manhattan Distance"));
    pDC->TextOut(5, static_cast<int>(topMargin + plotHeight / 2 - 10), _T("Steps"));

    pWnd->ReleaseDC(pDC);
}


void GridChartRenderer::DrawChart(HDC hDC, HGLRC hRC, CWnd* pChartWnd, const std::map<CString, std::vector<PathResult>>& algoResults)
{
	if (!wglMakeCurrent(hDC, hRC)) return;

	CRect rect;
	pChartWnd->GetClientRect(&rect);
	int width = rect.Width();
	int height = rect.Height();

	const int leftMargin = 50;
	const int rightMargin = 20;
	const int topMargin = 20;
	const int bottomMargin = 50;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, static_cast<GLdouble>(width), static_cast<GLdouble>(height), 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	GLfloat xAxisY = static_cast<GLfloat>(height - bottomMargin);
	GLfloat yAxisX = static_cast<GLfloat>(leftMargin);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2f(static_cast<GLfloat>(leftMargin), xAxisY);
	glVertex2f(static_cast<GLfloat>(width - rightMargin), xAxisY);
	glVertex2f(yAxisX, static_cast<GLfloat>(height - bottomMargin));
	glVertex2f(yAxisX, static_cast<GLfloat>(topMargin));
	glEnd();

	// Compute bounds
	int maxDistance = 1;
	int maxSteps = 1;
	for (const auto& [_, results] : algoResults)
	{
		for (const auto& r : results)
		{
			maxDistance = std::max(maxDistance, r.manhattan);
			maxSteps = std::max(maxSteps, r.steps);
		}
	}

	// Plot each algorithm
	int colorIndex = 0;
	for (const auto& [_, results] : algoResults)
	{
		if (colorIndex == 0)
			glColor3f(1.0f, 0.0f, 0.0f);  // red
		else
			glColor3f(0.2f, 0.2f, 1.0f);  // blue

		glPointSize(6.0f);
		glBegin(GL_POINTS);

		for (const auto& pt : results)
		{
			float plotWidth = static_cast<float>(width - leftMargin - rightMargin);
			float plotHeight = static_cast<float>(height - topMargin - bottomMargin);
			float padding = 0.02f;

			float normManhattan = static_cast<float>(pt.manhattan) / (1.2f * std::max(1, maxDistance));
			float normSteps = static_cast<float>(pt.steps) / (1.2f * std::max(1, maxSteps));

			normManhattan = std::clamp(normManhattan, padding, 1.0f - padding);
			normSteps = std::clamp(normSteps, padding, 1.0f - padding);

			float x = static_cast<float>(leftMargin) + normManhattan * plotWidth;
			float y = static_cast<float>(height - bottomMargin) - normSteps * plotHeight;

			glVertex2f(x, y);
		}

		glEnd();
		colorIndex++;
	}

	SwapBuffers(hDC);

    //int c = 0;
    //int maxSteps = 0;
    //for (const auto& [name, results] : algoResults)
    //{
    //    for (const auto& r : results)
    //    {
    //        maxDistance = std::max(maxDistance, r.manhattan);
    //        maxSteps = std::max(maxSteps, r.steps);
    //    }
    //}

    DrawCoordinateFrame(pChartWnd, width, height, maxDistance, maxSteps);

    DrawLegendBoxGDI(pChartWnd, width, height, algoResults);
}




//void GridChartRenderer::DrawChart(HDC hDC, HGLRC hRC, CWnd* pChartWnd, const std::map<CString, std::vector<PathResult>>& algoResults);
//{
//	if (!wglMakeCurrent(hDC, hRC)) return;
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
