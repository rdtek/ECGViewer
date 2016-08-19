using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Threading;

namespace ECGViewerWPF
{
    public class EcgAnnotation
    {
        public string Text { get; set; }
        public System.Windows.Point Position { get; set; }

        public EcgAnnotation() { }

        public EcgAnnotation(string text, System.Windows.Point position)
        {
            Text = text; Position = position;
        }
    }

    public class ECGView : Canvas, INotifyPropertyChanged
    {
        //Standard big square is 1 second, 25mm, 1mV. (25mm * 96dpi / mmInOneInch)
        const float DPI = 96.0f;
        const float MM_PER_BIG_SQUARE = 25.0f;
        const float MM_PER_INCH = 25.4f;       //25.4 millimetres = 1 inch.
        const int MS_PER_BIG_SQUARE = 1000;    //1 big square = 1 second = 1000ms.

        int ECG_BIG_SQUARE_PX;
        int Y_ZERO_PX = 250;
        double m_scaleFactor = 1;

        public float SampleResolution = 0.001f;
        public float SampleFrequency = 500f;

        private int m_currentPage = 1;
        private int m_totalPages = 1;

        private System.Windows.Media.Pen m_bigGridPen = MakeMediaPen(240, 128, 128, 1);
        private System.Windows.Media.Pen m_smallGridPen = MakeMediaPen(255, 192, 192, 1);

        private double m_width = 0;
        private double m_height = 0;

        DispatcherTimer m_resizeTimer = new DispatcherTimer {
            Interval = new TimeSpan(0, 0, 0, 0, 100),
            IsEnabled = false
        };

        private double[] m_signalSamples;
        private BitmapImage[] m_signalBitmaps;
        private SignalAnalysis m_analysisResults;
        private EcgAnnotation[] m_annotations;

        public static readonly DependencyProperty TracksPerPageProperty
            = DependencyProperty.Register("TracksPerPage", typeof(int), 
                typeof(NumberStepper), new PropertyMetadata(5));

        public event PropertyChangedEventHandler PropertyChanged;

        static ECGView()
        {
            DefaultStyleKeyProperty.OverrideMetadata(
                typeof(ECGView), new FrameworkPropertyMetadata(typeof(ECGView)));
        }

        public ECGView()
        {
            ECG_BIG_SQUARE_PX = (int)Math.Round(MM_PER_BIG_SQUARE * DPI / MM_PER_INCH);
            TracksPerPage = 10;
            m_resizeTimer.Tick += ResizeTimer_Tick;
            this.Loaded += new RoutedEventHandler(EcgChart_Loaded);
        }

        protected void NotifyPropertyChanged([CallerMemberName] String propertyName = "")
        {
            PropertyChangedEventHandler h = PropertyChanged;
            if (h != null) { h(this, new PropertyChangedEventArgs(propertyName)); }
        }

        private void EcgChart_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                Window.GetWindow(this).SizeChanged
                    += new SizeChangedEventHandler(EcgChart_SizeChanged);
            }
            catch (Exception) { }

            /* Fix parent scrollviewer height */
            UpdateGraphics();
        }

        private void ResizeTimer_Tick(object sender, EventArgs e)
        {
            m_resizeTimer.IsEnabled = false;
            UpdateGraphics();
        }

        private void EcgChart_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            m_resizeTimer.IsEnabled = true;
            m_resizeTimer.Stop();
            m_resizeTimer.Start();
        }

        private double GetParentWidth()
        {
            double width = 0;

            try
            {
                Control parent = this.Parent as Control;
                width = parent.ActualWidth;
                if (parent.GetType() == typeof(ScrollViewer))
                {
                    ScrollViewer sv = (ScrollViewer)parent;
                    width = sv.ViewportWidth;
                }
            }
            catch (Exception ex) { Console.WriteLine(ex.Message); }

            return width;
        }

        private double GetParentHeight()
        {
            double height = 0;
            try { height = (this.Parent as Control).ActualHeight; }
            catch (Exception ex) { Console.WriteLine(ex.Message); }
            return height;
        }

        protected override System.Windows.Size MeasureOverride(System.Windows.Size availableSize)
        {
            System.Windows.Size desiredSize = new System.Windows.Size(m_width, m_height);
            return desiredSize;
        }

        private static System.Windows.Media.Pen MakeMediaPen(byte r, byte g, byte b, double thickness)
        {
            return new System.Windows.Media.Pen(new SolidColorBrush(
                System.Windows.Media.Color.FromRgb(r, g, b)), thickness);
        }

        public double ScaleFactor
        {
            get { return m_scaleFactor; }
            set
            {
                if (value >= 0.1)
                {
                    m_scaleFactor = value;
                    UpdateGraphics();
                }
            }
        }

        public int EcgBigSquarePx { get { return (int)Math.Round(ECG_BIG_SQUARE_PX * ScaleFactor); } }

        public float EcgSmallSquarePx { get { return (float)EcgBigSquarePx / 5; } }

        private double XOffsetPx { get { return EcgBigSquarePx; } }

        public int TrackHeightPx { get { return 4 * EcgBigSquarePx; } }

        public int TrackWidthPx
        {
            get
            {
                int width = (int)(m_width - XOffsetPx * 1.5);
                //Console.WriteLine(this.ActualWidth + " m_width " + m_width + " XOffsetPx " + XOffsetPx + " TrackWidthPx " + width);
                return width >= EcgBigSquarePx ? width : EcgBigSquarePx;
            }
        }

        private int TrackDurationMs
        {
            get { return (int)Math.Round(TrackWidthPx / (float)EcgBigSquarePx * 1000); }
        }

        public int BigSquaresPerTrack
        {
            get
            {
                int numSuares = TrackWidthPx / EcgBigSquarePx;
                return numSuares >= 1 ? numSuares : 1;
            }
        }

        public int CurrentPage
        {
            get { return m_currentPage >= 1 ? m_currentPage : 1; }
            set
            {
                m_currentPage = value >= 1 ? value : 1;
                NotifyPropertyChanged();
            }
        }

        public int TracksPerPage
        {
            get { return (int)GetValue(TracksPerPageProperty); }
            set { SetValue(TracksPerPageProperty, value); }
        }

        public bool ShowBeatTimes { get; set; }

        public int TotalPages
        {
            get { return m_totalPages; }
            private set
            {
                m_totalPages = value;
                if (CurrentPage > m_totalPages)
                {
                    SetPagination(TracksPerPage, m_totalPages);
                }
                NotifyPropertyChanged();
            }
        }

        public void Clear()
        {
            //Clear the signal data and draw the empty grid
            m_signalBitmaps = null;
            m_signalSamples = null;
            m_analysisResults = null;
            m_annotations = null;
            InvalidateVisual();
        }

        private int CalculateTotalPages()
        {
            int pointsPerTrack = (int)Math.Round(BigSquaresPerTrack * SampleFrequency);
            double numPages = 1;
            if (m_signalSamples != null && m_signalSamples.Length >= 1)
            {
                numPages = Math.Ceiling((m_signalSamples.Length / (double)pointsPerTrack) / TracksPerPage);
            }
            return (int)numPages;
        }

        public void SetPagination(int tracksPerPage, int currentPage)
        {
            TracksPerPage = tracksPerPage;
            CurrentPage = currentPage;
            UpdateGraphics();
        }

        private void UpdateGraphics()
        {
            m_width = GetParentWidth();
            if (m_height <= 0) m_height = GetParentHeight();

            /* Get bitmaps before layout update so we can
            calculate the desired height first */
            UpdateDrawingData();

            InvalidateMeasure();
            InvalidateVisual();
            TotalPages = CalculateTotalPages();
        }

        public void SetSignalData(double[] signalData, bool updateGraphics = true)
        {
            Clear();
            m_signalSamples = signalData;
            if (updateGraphics) UpdateGraphics();
        }

        public bool HasSignalData()
        {
            bool hasData = false;
            if (m_signalSamples != null && m_signalSamples.Length >= 1) hasData = true;
            return hasData;
        }

        public void SetAnalysisResults(SignalAnalysis res)
        {
            m_analysisResults = res;
            UpdateGraphics();
        }

        private void UpdateDrawingData()
        {
            if (m_signalSamples != null && m_signalSamples.Length >= 1)
            {
                Bitmap[] bitmaps = GetSignalBitmaps(CurrentPage);
                m_signalBitmaps = Array.ConvertAll(bitmaps, ImageTools.BitmapToImage);
                if (m_analysisResults != null) m_annotations = GetAnnotations(CurrentPage);
            }
        }

        protected override void OnRender(DrawingContext dc)
        {
            m_width = GetParentWidth();

            try
            {
                Window window = Window.GetWindow(this);
                //Set width to ensure grid will be drawn to fill the window when window maximized.
                if (window.WindowState == WindowState.Maximized)
                {
                    m_width = this.ActualWidth;
                }
            }
            catch (Exception) { }

            DrawBackground(dc);
            DrawGrid(dc, EcgSmallSquarePx, m_smallGridPen);
            DrawGrid(dc, EcgBigSquarePx, m_bigGridPen);
            DrawSignals(dc);
            DrawAnnotations(dc, m_annotations);
        }

        private void DrawBackground(DrawingContext dc)
        {
            Rect size = new Rect(0, 0, m_width, m_height);
            dc.DrawRectangle(new SolidColorBrush(Colors.White), null, size);
        }

        private void DrawGrid(DrawingContext dc, float square, System.Windows.Media.Pen pen)
        {
            float x = -square; float y = -square;
            double height = m_height;

            /* Use parent height if the combined height of the signal
            tracks is smaller than the page */
            try { height = (this.Parent as Control).ActualHeight; }
            catch (Exception ex) { Console.WriteLine(ex.Message); }
            m_height = m_height < height ? height : m_height;

            ImageTools.DrawGrid(dc, square, pen, m_width, m_height);
        }

        private void DrawSignals(DrawingContext dc)
        {
            if (m_signalBitmaps == null) return;

            for (int i = 0; i < m_signalBitmaps.Length; i++)
            {

                //Draw the signal bitmap
                BitmapImage bmpImg = m_signalBitmaps[i];
                dc.DrawImage(bmpImg, new Rect(0, i * bmpImg.Height, bmpImg.Width, bmpImg.Height));

                //Draw the start time of the track
                double x = EcgSmallSquarePx;
                double y = i * bmpImg.Height + EcgSmallSquarePx;

                FormattedText timeText = GetTrackStartTime((CurrentPage - 1) * TracksPerPage + i);

                ImageTools.DrawText(dc, timeText, x, y);
            }
        }

        private void DrawAnnotations(DrawingContext dc, EcgAnnotation[] annotations)
        {
            if (annotations == null) return;
            double fontEmSize = 10;

            for (int i = 0; i < annotations.Length; i++)
            {
                EcgAnnotation ann = annotations[i];
                FormattedText text = GetFormattedText(ann.Text);

                double xPosition = ann.Position.X;
                if (i >= 1 && xPosition == annotations[i - 1].Position.X)
                {
                    //Offset X if multiple annotation in same position
                    xPosition += fontEmSize - 2;
                }

                ImageTools.DrawText(dc, text, xPosition - (text.Width / 2), ann.Position.Y);
            }
        }

        private void DrawAnnotations(System.Drawing.Graphics g, EcgAnnotation[] annotations)
        {
            if (annotations == null) return;
            double fontEmSize = 10;

            for (int i = 0; i < annotations.Length; i++)
            {
                EcgAnnotation ann = annotations[i];
                FormattedText text = GetFormattedText(ann.Text);

                double xPosition = ann.Position.X;
                if (i >= 1 && xPosition == annotations[i - 1].Position.X)
                {
                    //Offset X if multiple annotation in same position
                    xPosition += fontEmSize - 2;
                }

                ImageTools.DrawText(g, text, 8.0f, xPosition - (text.Width / 2), ann.Position.Y);
            }
        }

        private FormattedText GetFormattedText(string text)
        {
            double fontEmSize = 10;
            return new FormattedText(text,
                   System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight,
                   new Typeface("Arial"), fontEmSize, System.Windows.Media.Brushes.Black);
        }

        private FormattedText GetTrackStartTime(int trackIndex)
        {
            TimeSpan t = TimeSpan.FromMilliseconds(trackIndex * TrackDurationMs);
            return GetFormattedText(string.Format("{0:00}:{1:00}:{2:00}.{3:0}",
                t.Hours, t.Minutes, t.Seconds, Math.Round(t.Milliseconds / (float)100)));
        }

        private Bitmap DrawSignalBitmap(System.Windows.Point[] signalPoints, int bitmapWidth)
        {
            Bitmap bmp = ImageTools.CreateBitmap(bitmapWidth, TrackHeightPx, DPI);

            if (bmp.Width == bitmapWidth && bmp.Height == TrackHeightPx)
            {
                using (Graphics bg = Graphics.FromImage(bmp))
                {
                    bg.SmoothingMode = SmoothingMode.AntiAlias;
                    bg.FillRectangle(System.Drawing.Brushes.Transparent,
                        new Rectangle(0, 0, bmp.Width, bmp.Height));
                    bg.DrawRectangle(new System.Drawing.Pen(System.Drawing.Brushes.Gray), 0, 0, bmp.Width, bmp.Height);

                    var bmpPoints = new List<System.Drawing.Point>();

                    for (int i = 0; i < signalPoints.Length; i++)
                    {
                        System.Windows.Point wp = signalPoints[i];
                        System.Drawing.Point dp = new System.Drawing.Point((int)wp.X, (int)wp.Y);
                        bmpPoints.Add(dp);
                    }

                    if (bmpPoints.Count >= 1)
                    {
                        bg.DrawLines(new System.Drawing.Pen(System.Drawing.Color.FromArgb(10, 10, 10), 1), bmpPoints.ToArray());
                    }
                }
            }

            return bmp;
        }

        private System.Windows.Point[] GetSignalPoints(double[] signalData, int start = 0, int length = -1)
        {
            var points = new List<System.Windows.Point>();

            if (signalData != null)
            {
                if (length <= 0) length = signalData.Length;
                if (start <= 0) start = 0;
                for (int i = start; i < start + length && i < signalData.Length; i++)
                    points.Add(new System.Windows.Point(i, signalData[i]));
            }

            return points.ToArray();
        }

        private Bitmap[] GetSignalBitmaps(int pageNum)
        {
            var bitmaps = new List<Bitmap>();
            int pointsPerTrack = (int)Math.Round(TrackWidthPx / (float)EcgBigSquarePx * SampleFrequency);

            //Get the point data for the current page
            int pointsPerPage = TracksPerPage * pointsPerTrack;
            int startIndex = (pageNum - 1) * pointsPerPage;
            var pagePoints = GetSignalPoints(m_signalSamples, startIndex, pointsPerPage);

            for (int iTrack = 0; iTrack < TracksPerPage; iTrack++)
            {
                int cursor = iTrack * pointsPerTrack;
                int subset = (cursor + pointsPerTrack > pagePoints.Length)
                    ? pagePoints.Length % pointsPerTrack : pointsPerTrack;

                System.Windows.Point[] linePoints = pagePoints.Skip(cursor).Take(subset).ToArray();

                //Scale the X and Y values
                for (int j = 0, k = cursor; j < linePoints.Length; j++, k++)
                {
                    linePoints[j].X = XOffsetPx + ScaleSignalXToPixels(j);
                    linePoints[j].Y = Y_ZERO_PX * ScaleFactor - ScaleSignalYToPixels(pagePoints[k].Y);
                }

                Bitmap bmp = DrawSignalBitmap(linePoints, (int)m_width);
                bitmaps.Add(bmp);

                //Save height before layout is updated
                m_height = iTrack * bmp.Height + bmp.Height;
            }
            return bitmaps.ToArray();
        }

        private int ScaleSignalXToPixels(int sampleIndex)
        {
            double xPixels = (double)(sampleIndex / SampleFrequency * EcgBigSquarePx);
            return (int)Math.Round(xPixels);
        }

        private int ScaleSignalYToPixels(double sample)
        {
            double yPixels = sample * SampleResolution * EcgBigSquarePx;
            return (int)Math.Round(yPixels);
        }

        private EcgAnnotation[] GetAnnotations(int pageNum)
        {
            var annotations = new List<EcgAnnotation>();
            SignalBeat[] beats = m_analysisResults.GetBeats();

            double timeMsPerPage = ((TrackWidthPx * TracksPerPage) / (double)EcgBigSquarePx) * 1000;
            double pageStartTime = (pageNum - 1) * timeMsPerPage;
            double pageEndTime = pageStartTime + timeMsPerPage;

            for (int i = 0; i < beats.Length; i++)
            {
                SignalBeat beat = beats[i];

                if (beat.Time >= pageStartTime && beat.Time < pageEndTime)
                {

                    int trackNum = 0; double x = 0; double y = 0;

                    x = beat.Time / (float)MS_PER_BIG_SQUARE * EcgBigSquarePx;
                    trackNum = (int)Math.Floor(x / TrackWidthPx);
                    x = XOffsetPx + (x % TrackWidthPx);

                    y = (trackNum % TracksPerPage + 1) * 4 * EcgBigSquarePx;
                    y = y - EcgBigSquarePx + EcgSmallSquarePx;

                    System.Windows.Point point = new System.Windows.Point(x, y);
                    annotations.Add(new EcgAnnotation(beat.Label 
                        + (ShowBeatTimes ? "\n" + beat.Time : ""), point));
                }
            }

            return annotations.ToArray();
        }

        public void SaveReport(string filePath)
        {
            if (m_signalSamples == null || m_signalSamples.Length <= 0) return;

            var pageBitmaps = new List<Bitmap>();

            for (int pageNum = 1; pageNum <= TotalPages; pageNum++)
            {
                var pageBitmap = new Bitmap(1, 1);
                double pageHeight = 0;
                Bitmap[] signalBitmaps = GetSignalBitmaps(pageNum);
                EcgAnnotation[] annotations = GetAnnotations(pageNum);
                System.Drawing.Pen bigGridPen = new System.Drawing.Pen(System.Drawing.Color.FromArgb(240, 128, 128));
                System.Drawing.Pen smallGridPen = new System.Drawing.Pen(System.Drawing.Color.FromArgb(255, 192, 192));

                foreach (var bmp in signalBitmaps) pageHeight += bmp.Height;

                pageBitmap = new Bitmap((int)signalBitmaps[0].Width, (int)pageHeight);

                using (var g = Graphics.FromImage(pageBitmap))
                {

                    g.InterpolationMode = InterpolationMode.HighQualityBicubic;

                    //Draw white background for the whole page
                    var backgroundRect = new System.Drawing.Rectangle(0, 0, pageBitmap.Width, pageBitmap.Height);
                    //g.DrawRectangle(new System.Drawing.Pen(System.Drawing.Color.White), backgroundRect);
                    g.FillRectangle(System.Drawing.Brushes.White, backgroundRect);

                    //Draw grid squares
                    ImageTools.DrawGrid(g, EcgSmallSquarePx, smallGridPen, pageBitmap.Width, pageBitmap.Height);
                    ImageTools.DrawGrid(g, EcgBigSquarePx, bigGridPen, pageBitmap.Width, pageBitmap.Height);

                    //Draw the signal tracks and annotation
                    for (int iTrack = 0; iTrack < signalBitmaps.Length; iTrack++)
                    {
                        Bitmap b = signalBitmaps[iTrack];

                        //Draw signal bitmap
                        g.DrawImage(b, 0, iTrack * b.Height, (int)b.Width, b.Height);

                        //Draw track time text
                        FormattedText timeText = GetTrackStartTime((pageNum - 1) * TracksPerPage + iTrack);
                        ImageTools.DrawText(g, timeText, 8.0f, (int)EcgSmallSquarePx, (int)(iTrack * b.Height + EcgSmallSquarePx));

                        //Draw the beat annotations
                        DrawAnnotations(g, annotations);
                    }
                    g.Save();
                }

                pageBitmaps.Add(pageBitmap);
            }

            ImageTools.SavePdf(filePath, pageBitmaps.ToArray(), "ECG Report");
        }
    }
}
