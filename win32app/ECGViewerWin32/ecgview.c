#include "ecgview.h"

maxSamples = 10000;
PADDING_X = 20;
sampleFrequency = 500;

void DrawGrid(HDC hdc, HWND hwnd){
    
    //Setup drawing pens
    HPEN   hBigGridPen;
    HPEN   hSmallGridPen;
	HBRUSH hBgBrush; 
	RECT   rect;

	hBgBrush = CreateSolidBrush(RGB(255, 255, 255));
    hSmallGridPen = CreatePen(PS_SOLID, 1, RGB(255, 192, 192));
    hBigGridPen = CreatePen(PS_SOLID, 1, RGB(240, 128, 128));
    
	if (GetClientRect(hwnd, &rect)) {
		//Fill background
		FillRect(hdc, &rect, hBgBrush);

		//Small grid lines
		SelectObject(hdc, hSmallGridPen);
		DrawGridLines(hdc, hwnd, 10);

		//Big grid lines
		SelectObject(hdc, hBigGridPen);
		DrawGridLines(hdc, hwnd, 50);
	}
}

void DrawGridLines(HDC hdc, HWND hwnd, int interval){

    POINT points[2];
    RECT rect;

    if(GetWindowRect(hwnd, &rect)) {
        
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
        
        //Vertical lines
        for(int i = 0; i * interval < windowWidth; i++){
            points[0].x = interval * i;
            points[0].y = 0;
            points[1].x = interval * i;
            points[1].y = windowHeight;
            Polyline(hdc, points, 2);
        }
        
        //Horizontal lines
        for(int i = 0; i * interval < windowHeight; i++){
            points[0].x = 0;
            points[0].y = interval * i;
            points[1].x = windowWidth;
            points[1].y = interval * i;
            Polyline(hdc, points, 2);
        }
    }
}

void DrawSignal(HDC hdc, HWND hwnd){

	HPEN  hSignalPen;
    POINT points[2];
    RECT  rect;

	hSignalPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    int xPos = PADDING_X;
    int yOffset = 100;
    int i = 0; int j = 0;
    int pointsPerTrack = PointsPerTrack(hdc, hwnd);

	//Signal wave line
	SelectObject(hdc, hSignalPen);

    //log_int("pointsPerTrack: ", (int) pointsPerTrack);
    
    if(GetWindowRect(hwnd, &rect)) {

        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
    
        for (i = 0, j = 0; j < maxSamples - 1; i += 1, j += 1) {

            points[0].x = xPos;
            points[0].y = yOffset + (signalBuffer[j] * 0.1);
            points[1].x = PADDING_X + ScaleSignalXToPixels(i + 1);
            points[1].y = yOffset + (signalBuffer[j + 1] * 0.1);
            
            Polyline(hdc, points, 2);
            xPos = points[1].x;
            
            if (i == (pointsPerTrack - 1)) {
                //log_int("j: ", j);
                i = 0;
                xPos = PADDING_X;
                yOffset += 100;
            }
        }
    }
}

int ScaleSignalXToPixels(int sampleIndex) {
    double xPixels = (sampleIndex / (double)sampleFrequency) * EcgBigSquarePx();
    int intXPixels = (int)floor(xPixels);
    return intXPixels;
}

int PointsPerTrack(HDC hdc, HWND hwnd) {
    
    int trackWidth = TrackWidthPx(hdc, hwnd);
    int ecgBigSq = EcgBigSquarePx();
    
    double dblNumPoints = (trackWidth / ecgBigSq) * sampleFrequency; 
    double roundedNumPoints = floor(dblNumPoints);
    
    return (int) roundedNumPoints;
}

int TrackWidthPx(HDC hdc, HWND hwnd){
    
    double trackWidth = -1; 
    RECT windowRect;
    
    if(GetWindowRect(hwnd, &windowRect)) {
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;
        trackWidth = windowWidth - 2 * PADDING_X;
    }

    return (int) trackWidth;
}

int EcgBigSquarePx(){
    return 50;
}
