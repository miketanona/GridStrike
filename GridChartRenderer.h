// GridChartRenderer.h : header file
//

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <thread>
#include <atlimage.h>

class GridChartRenderer
{
public:
    //void DrawGridMap(CDC* pDC, HDC hDC, HGLRC hRC, CWnd* pWnd, const std::vector<std::vector<int>>& mapData,
    //                 const std::vector<std::pair<int, int>>& path,
    //                 const std::pair<int, int>& startPos,
    //                 const std::pair<int, int>& endPos,
    //                 bool bDrawPath);

    void DrawGridMap(HDC hDC, HGLRC hRC, CWnd* pGLWnd,
        const std::vector<std::vector<int>>& mapData,
        const std::vector<std::pair<int, int>>& path,
        const std::pair<int, int>& startPos,
        const std::pair<int, int>& endPos,
        bool bDrawPath);

    void DrawChart(HDC hDC, HGLRC hRC, CWnd* pWnd, const std::map<CString, std::vector<PathResult>>& algoResults);

    void DrawCoordinateFrame(CWnd* pWnd, int width, int height, int maxDistance, int maxSteps);

    //void DrawLegendBoxGDI(CWnd* pChartWnd, const std::map<CString, std::vector<PathResult>>& algoResults);

    void DrawLegendBoxGDI(CWnd* pWnd, int width, int height, const std::map<CString, std::vector<PathResult>>& algoResults);

};
