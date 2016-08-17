#include "ecg.h"

int maxSamples          = 100000;
int padding_x           = 20;
int sampleFrequency     = 500;
double sampleResolution = 0.1;

static HWND m_hWindow;
static HeartSignal* m_heartSignal;
static int m_currentPageNum = 0;

void InitECG(HWND hWindow) {
    m_hWindow = hWindow;
}

// Name:   GenerateSignal
// Desc:   Generate a simulated ECG signal.
void GenerateSignal(HeartSignal* ptr_signal) {

    int indexSamples = 0;
    ptr_signal->numberOfSamples = 0;
    size_t numBeats = 50;

    //Generate each part of the heartbeat waveform
    for (size_t i = 0; i < numBeats; i++) {

        //1. *** Flatline
        GenerateLine(ptr_signal, 300, 0);

        //2. *** P wave: use Gausian Distribution to generate P-Wave curve
        GenerateGausianCurve(ptr_signal, 0, 2, 200);

        //3. *** PR segment
        GenerateLine(ptr_signal, 50, 0);

        //4. *** QRS complex
        GenerateLine(ptr_signal, 50, -2);  //'Q' part - dip down
        GenerateLine(ptr_signal, 20, 25);  //'R' part - jump up
        GenerateLine(ptr_signal, 22, -25); //'S' part - jump down
        GenerateLine(ptr_signal, 20, 7);   //Come back to base line

        //5. *** ST segment
        GenerateLine(ptr_signal, 50, 0);

        //6. *** T wave: use Gausian Distribution to generate T-Wave curve
        GenerateGausianCurve(ptr_signal, 0, 2, 200);
    }

}

// Name:   GenerateLine
// Desc:   Generates a line with a gradient (rise or fall or flat)
void GenerateLine(HeartSignal* ptr_signal, size_t length, double rise) {

    size_t maxLength = 1000;
    int iCurrent = ptr_signal->numberOfSamples;

    for (size_t i = 0; i < length && i < maxLength; i++) {
        int iPrevious = iCurrent - 1 >= 0 ? iCurrent - 1 : 0;
        double previousYValue = ptr_signal->samples[iPrevious];
        ptr_signal->samples[iCurrent] = previousYValue + rise;
        //log_dbl("Live val: ", ptr_signal->samples[iCurrent]);
        ptr_signal->numberOfSamples++;
        iCurrent++;
    }
}

// Name:   CalculateGausianPDF
// Desc:   Calculates the Probability Density Function value for position
//         on the curve.
double CalculateGausianPDF(double x, double mu, double variance) {
    double oneOverSqrt2VarPi = 1 / sqrt(2 * variance * M_PI);
    double expo = exp(-1 * pow((x - mu), 2) / (2 * pow(variance, 2)));
    return oneOverSqrt2VarPi * expo;
}

// Name:   GenerateGausianCurve
// Desc:   Generates a Gausian Distribution curve
void GenerateGausianCurve(HeartSignal* ptr_signal, double mu, double variance, double scaleFactor) {
    int sampleIndex = ptr_signal->numberOfSamples;
    for (double x = -3; x <= 3; x += 0.1) {
        double xpdf = scaleFactor * CalculateGausianPDF(x, mu, variance);
        ptr_signal->samples[sampleIndex] = xpdf;
        ptr_signal->numberOfSamples++;
        sampleIndex++;
    }
}

void SetECGSignal(HeartSignal* signal) {
    m_heartSignal = signal;
}

// Name:   IncrementPagination
// Desc:   Increases current page number so next time screen is painted 
//         the next page of ECG data will be drawn.
int IncrementPagination() {
    int result = 0;
    if (m_heartSignal == NULL) return 0;
    int totalPages = TotalPages();
    //Check there are pages remaining
    if (m_currentPageNum < totalPages) {
        m_currentPageNum++;
        result = 1;
    }
    return result;
}

// Name:   DecrementPagination
// Desc:   Decreases current page number so next time screen is painted 
//         the previous page of ECG data will be drawn.
int DecrementPagination() {
    int result = 0;
    if (m_heartSignal == NULL) return 0;
    if (m_currentPageNum >= 1) {
        m_currentPageNum--;
        result = 1;
    }
    return result;
}

// Name:   DrawGrid
// Desc:   Draws grid squares indicating time along horizontal and voltage in vertical.
// Param:  hDeviceContext - the Windows device context to draw to.
void DrawGrid(HDC hDeviceContext){
    
    //Setup drawing pens
    HPEN   hBigGridPen;
    HPEN   hSmallGridPen;
	HBRUSH hBgBrush; 
	RECT   rect;

	hBgBrush      = CreateSolidBrush(RGB(255, 255, 255));
    hSmallGridPen = CreatePen(PS_SOLID, 1, RGB(255, 192, 192));
    hBigGridPen   = CreatePen(PS_SOLID, 1, RGB(240, 128, 128));
    
	if (GetClientRect(m_hWindow, &rect)) {
		//Fill background
		FillRect(hDeviceContext, &rect, hBgBrush);

		//Small grid lines
		SelectObject(hDeviceContext, hSmallGridPen);
		DrawGridLines(hDeviceContext, 10);

		//Big grid lines
		SelectObject(hDeviceContext, hBigGridPen);
		DrawGridLines(hDeviceContext, 50);
	}
}
 
// Name:   DrawGridLines
// Desc:   Draws horizontal and vertical lines to form a grid.
// Param:  hDeviceContext - the Windows device context to draw to.
void DrawGridLines(HDC hDeviceContext, int interval){

    POINT points[2];
    RECT rect;

    if(GetWindowRect(m_hWindow, &rect)) {
        
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
// Desc:   Draws rows of signal wave lines on top of ECG grid.
// Param:  hDeviceContext - Windows device context to draw to.
void DrawSignal(HDC hDeviceContext){

    POINT points[2];
    RECT  windowRect;
    RECT  textRect;
    HPEN  hSignalPen    = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    int xPos            = padding_x;
    int bottomPadding   = 0.5 * EcgBigSquarePx();
    int yOffset         = 1.5 * EcgBigSquarePx();
    int iTrackPoint     = 0; 
    int iSample         = 0;
    int pointsOneTrack  = PointsPerTrack(hDeviceContext, m_hWindow);
    int trackNum        = 0;

	SelectObject(hDeviceContext, hSignalPen);
    
    if(GetClientRect(m_hWindow, &windowRect)) {

        //How many signal tracks can fit in the window?
        int windowWidth      = GetWindowWidth(m_hWindow);
        int maxTracksPerPage = MaxTracksPerPage();

        //First time label starting position
        textRect      = windowRect;
        textRect.left = padding_x;
        textRect.top  = 2 * EcgSmallSquarePx();

        //Get the starting sample for the current page
        iSample = pointsOneTrack * maxTracksPerPage * m_currentPageNum;
        
        for (iTrackPoint = 0, iSample = 0; 
            iSample < maxSamples - 1 && iSample < m_heartSignal->numberOfSamples;
            iTrackPoint += 1, iSample += 1) {

            //Draw the track start time label
            if (iTrackPoint == 0) 
                DrawTrackStartTime(hDeviceContext, iSample, textRect);
			
            //Set the coordinates for start and end points of line
            points[0].x = xPos;
            points[0].y = yOffset + (-1) * (m_heartSignal->samples[iSample] * sampleResolution);
            points[1].x = padding_x + ScaleSignalXToPixels(iTrackPoint + 1);
            points[1].y = yOffset + (-1) * (m_heartSignal->samples[iSample + 1] * sampleResolution);
            
            //Draw the line onto the device context
            Polyline(hDeviceContext, points, 2);

            xPos = points[1].x;
            
            //Reached end of track, begin new track
            if (iTrackPoint == (pointsOneTrack - 1)) {
                iTrackPoint = -1;
                xPos = padding_x;
                yOffset += 2 * EcgBigSquarePx();
                textRect.top += 2 * EcgBigSquarePx();
                trackNum++;
            }

            if (trackNum >= maxTracksPerPage) break;
        }
    }
}

// Name:    DrawTrackStartTime
// Desc:    Display the time at the beginning of each track.
// Param:   hDeviceContext - Windows device context to draw to.
// Param:   sampleIndex - the sample index at the start of the track.
// Param:   positionRect - rectangle struct to draw text in position.
void DrawTrackStartTime(HDC hDeviceContext, int sampleIndex, RECT positionRect) {
    
    int trackTimeMs         = sampleIndex *  1000 / sampleFrequency;
    double seconds          = (double)trackTimeMs / 1000;
    double remainderSeconds = seconds - floor(seconds / 60) * 60;
    double minutes          = floor(seconds / 60);
    double remainderMinutes = minutes - floor(minutes / 60) * 60;
    double hours            = floor(minutes / 60);
    
    wchar_t timeLabelBuff[256];
    swprintf(timeLabelBuff, 30, L"Time: %02.0f:%02.0f:%04.1f", hours, remainderMinutes, remainderSeconds);
    DrawText(hDeviceContext, timeLabelBuff, -1, &positionRect, DT_SINGLELINE | DT_NOCLIP);
}

// Name:    ScaleSignalXToPixels
// Desc:    Creates a X coordinate based on grid scale so we can draw the signal point.
// Param:   sampleIndex - the index of the sample in the signal samples array.
// Returns: X pixel coordinate as integer.
int ScaleSignalXToPixels(int sampleIndex) {
    double xPixels = (sampleIndex / (double)sampleFrequency) * EcgBigSquarePx();
    int intXPixels = (int)floor(xPixels);
    return intXPixels;
}

// Name:    PointsPerTrack
// Desc:    Calculates how many points fit in one signal track.
// Param:   hDeviceContext - Windows device context to draw to.
// Returns: Whole number of points for one track
int PointsPerTrack() {
    
    int trackWidth = TrackWidthPx();
    int ecgBigSq = EcgBigSquarePx();
    
    double dblNumPoints = (trackWidth / ecgBigSq) * sampleFrequency; 
    double roundedNumPoints = floor(dblNumPoints);
    
    return (int) roundedNumPoints;
}

// Name:    TrackWidthPx
// Desc:    Calculates width of signal track taking account of padding or margins.
// Returns: Width of track in pixel units as integer.
int TrackWidthPx(){
    
    double trackWidth = -1; 
    RECT windowRect;
    
    if(GetWindowRect(m_hWindow, &windowRect)) {
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;
        trackWidth = windowWidth - 2 * padding_x;
    }

    return (int) trackWidth;
}

// Name:    MaxTracksPerPage
// Desc:    Calculate maximum number of signal tracks that can fit in current window size.
// Returns: Integer number of tracks.
int MaxTracksPerPage() {
    int clientHeight = GetClientHeight(m_hWindow);
    return (clientHeight - PaddingBottom()) / TrackHeight();
}

// Name:    TotalTracks
// Desc:    Calculate total number of signal tracks for all the signal samples.
// Returns: Integer number of tracks.
int TotalTracks() {
    int numPointsPerTrack = PointsPerTrack();
    return ceil(m_heartSignal->numberOfSamples / numPointsPerTrack);
}

int TotalPages() {
    int total = ceil(TotalTracks() / MaxTracksPerPage());
}

// Name:    EcgBigSquarePx
// Desc:    Get the width and height of big ECG grid square representing 1 second.
// Returns: Width and height of big grid square in pixel units.
int EcgBigSquarePx(){
    return 50;
}

// Name:    EcgSmallSquarePx
// Desc:    Get the width and height of small ECG grid square representing 1/5 second.
// Returns: Width and height of big small square in pixel units.
double EcgSmallSquarePx(){
    return EcgBigSquarePx() / 5;
}

int PaddingBottom() {
    return 0.5 * EcgBigSquarePx();
}

int TrackHeight() {
    return 2 * EcgBigSquarePx();
}

// Name:    TrackDurationMs
// Desc:    Get the total time in milliseconds represented by one track.
// Returns: Duration of one track in milliseconds as integer.
int TrackDurationMs(){
    float trackDurationMs = TrackWidthPx(m_hWindow) / (float)EcgBigSquarePx() * 1000;
    return (int)round(trackDurationMs);
}
