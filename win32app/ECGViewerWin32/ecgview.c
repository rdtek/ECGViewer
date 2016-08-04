#include "ecgview.h"

int maxSamples = 100000;
int padding_x = 20;
int sampleFrequency = 500;

// Name:   DrawGrid
// Desc:   Draws grid squares indicating time along horizontal and voltage in vertical
// Param:  hDeviceContext - the Windows device context to draw to.
void DrawGrid(HDC hDeviceContext, HWND hWindow){
    
    //Setup drawing pens
    HPEN   hBigGridPen;
    HPEN   hSmallGridPen;
	HBRUSH hBgBrush; 
	RECT   rect;

	hBgBrush      = CreateSolidBrush(RGB(255, 255, 255));
    hSmallGridPen = CreatePen(PS_SOLID, 1, RGB(255, 192, 192));
    hBigGridPen   = CreatePen(PS_SOLID, 1, RGB(240, 128, 128));
    
	if (GetClientRect(hWindow, &rect)) {
		//Fill background
		FillRect(hDeviceContext, &rect, hBgBrush);

		//Small grid lines
		SelectObject(hDeviceContext, hSmallGridPen);
		DrawGridLines(hDeviceContext, hWindow, 10);

		//Big grid lines
		SelectObject(hDeviceContext, hBigGridPen);
		DrawGridLines(hDeviceContext, hWindow, 50);
	}
}
 
// Name:   DrawGridLines
// Desc:   Draws horizontal and vertical lines to form a grid
// Param:  hDeviceContext - the Windows device context to draw to.
void DrawGridLines(HDC hDeviceContext, HWND hwnd, int interval){

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
            Polyline(hDeviceContext, points, 2);
        }
        
        //Horizontal lines
        for(int i = 0; i * interval < windowHeight; i++){
            points[0].x = 0;
            points[0].y = interval * i;
            points[1].x = windowWidth;
            points[1].y = interval * i;
            Polyline(hDeviceContext, points, 2);
        }
    }
}

// Name:   DrawSignal
// Desc:   Draws rows of signal wave lines on top of ECG grid
// Param:  hDeviceContext - Windows device context to draw to.
// Param:  hWindow - handle to window containing device context.
void DrawSignal(HDC hDeviceContext, HWND hWindow){

	HPEN  hSignalPen;
    POINT points[2];
    RECT  windowRect;
    RECT  textRect;

	hSignalPen          = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    int xPos            = padding_x;
    int bottomPadding   = 0.5 * EcgBigSquarePx();
    int yOffset         = 1.5 * EcgBigSquarePx();
    int iTrackPoint     = 0; int iSample = 0;
    int pointsPerTrack  = PointsPerTrack(hDeviceContext, hWindow);
    int trackIndex      = 0;

	SelectObject(hDeviceContext, hSignalPen);
    
    if(GetClientRect(hWindow, &windowRect)) {
        
        //How many signal tracks can fit in the window?
        int width = windowRect.left - windowRect.right;
        int windowHeight = windowRect.bottom - windowRect.top;
        int trackHeight = 2 * EcgBigSquarePx();
        int maxTracksInWindow = (windowHeight - bottomPadding) / trackHeight;

        //First time label starting position
        textRect = windowRect;
        textRect.left = padding_x;
        textRect.top = 2 * EcgSmallSquarePx();

        for (iTrackPoint = 0, iSample = 0; iSample < maxSamples - 1; iTrackPoint += 1, iSample += 1) {

            if (iTrackPoint == 0) 
                DrawTrackStartTime(hDeviceContext, hWindow, trackIndex, textRect);
			
            //Set the coordinates for start and end points of line
            points[0].x = xPos;
            points[0].y = yOffset + (signalBuffer[iSample] * 0.1);
            points[1].x = padding_x + ScaleSignalXToPixels(iTrackPoint + 1);
            points[1].y = yOffset + (signalBuffer[iSample + 1] * 0.1);
            
            //Draw the line onto the device context
            Polyline(hDeviceContext, points, 2);

            xPos = points[1].x;
            
            //Reached eng of track, begin new track
            if (iTrackPoint == (pointsPerTrack - 1)) {
                iTrackPoint = -1;
                xPos = padding_x;
                yOffset += 2 * EcgBigSquarePx();
                textRect.top += 2 * EcgBigSquarePx();
                trackIndex++;
            }

            if (trackIndex >= maxTracksInWindow) break;
        }
    }
}

// Name:    DrawTrackStartTime
// Desc:    Display the time at the beginning of each track
// Param:   hDeviceContext - Windows device context to draw to.
// Param:   hWindow - handle to window containing device context.
// Param:   trackIndex - the index of the signal track 0, 1, 2, 3...
// Param:   positionRect - rectangle struct to draw text in position
void DrawTrackStartTime(HDC hDeviceContext, HWND hWindow, int trackIndex, RECT positionRect) {
    int trackTimeMs = trackIndex * TrackDurationMs(hWindow);
    double seconds = (double)trackTimeMs / 1000;
    double minutes = seconds / 60;
    double hours = minutes / 60;
    wchar_t timeLabelBuff[256];
    swprintf(timeLabelBuff, 30, L"Time: %.0f:%.0f:%.1f", hours, minutes, seconds);
    DrawText(hDeviceContext, timeLabelBuff, -1, &positionRect, DT_SINGLELINE | DT_NOCLIP);
}

// Name:    ScaleSignalXToPixels
// Desc:    Creates a X coordinate based on grid scale so we can draw the signal point
// Param:   sampleIndex - the index of the sample in the signal samples array
// Returns: X pixel coordinate as integer
int ScaleSignalXToPixels(int sampleIndex) {
    double xPixels = (sampleIndex / (double)sampleFrequency) * EcgBigSquarePx();
    int intXPixels = (int)floor(xPixels);
    return intXPixels;
}

// Name:    PointsPerTrack
// Desc:    Calculates how many points fit in one signal track
// Param:   hDeviceContext - Windows device context to draw to.
// Param:   hWindow - handle to window containing device context.
// Returns: Whole number of points for one track
int PointsPerTrack(HDC hDeviceContext, HWND hWindow) {
    
    int trackWidth = TrackWidthPx(hWindow);
    int ecgBigSq = EcgBigSquarePx();
    
    double dblNumPoints = (trackWidth / ecgBigSq) * sampleFrequency; 
    double roundedNumPoints = floor(dblNumPoints);
    
    return (int) roundedNumPoints;
}

// Name:    TrackWidthPx
// Desc:    Calculates width of signal track taking account of padding or margins
// Param:   hWindow - handle to window containing device context.
// Returns: Width of track in pixel units as integer
int TrackWidthPx(HWND hWindow){
    
    double trackWidth = -1; 
    RECT windowRect;
    
    if(GetWindowRect(hWindow, &windowRect)) {
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;
        trackWidth = windowWidth - 2 * padding_x;
    }

    return (int) trackWidth;
}

// Name:    EcgBigSquarePx
// Desc:    Get the width and height of big ECG grid square representing 1 second
// Returns: Width and height of big grid square in pixel units
int EcgBigSquarePx(){
    return 50;
}

// Name:    EcgSmallSquarePx
// Desc:    Get the width and height of small ECG grid square representing 1/5 second
// Returns: Width and height of big small square in pixel units
double EcgSmallSquarePx(){
    return EcgBigSquarePx() / 5;
}

// Name:    TrackDurationMs
// Desc:    Get the total time in milliseconds represented by one track
// Returns: Duration of one track in milliseconds as integer
int TrackDurationMs(HWND hWindow){
    float trackDurationMs = TrackWidthPx(hWindow) / (float)EcgBigSquarePx() * 1000;
    return (int)round(trackDurationMs);
}
