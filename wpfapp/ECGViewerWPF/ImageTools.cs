using System;
using System.IO;
using System.Windows.Media.Imaging;
using System.Windows.Media;
using System.Drawing;
using System.Windows;
using PdfSharp.Pdf;
using PdfSharp.Drawing;

namespace ECGViewerWPF
{
    public static class ImageTools
    {
        public static BitmapImage BitmapToImage(Bitmap bitmap)
        {
            using (MemoryStream memory = new MemoryStream())
            {
                bitmap.Save(memory, System.Drawing.Imaging.ImageFormat.Png);
                memory.Position = 0;
                BitmapImage bitmapimage = new BitmapImage();
                bitmapimage.BeginInit();
                bitmapimage.StreamSource = memory;
                bitmapimage.CacheOption = BitmapCacheOption.OnLoad;
                bitmapimage.EndInit();
                return bitmapimage;
            }
        }

        public static Bitmap BitmapImage2Bitmap(BitmapImage bitmapImage)
        {
            using (MemoryStream outStream = new MemoryStream())
            {
                BitmapEncoder enc = new BmpBitmapEncoder();
                enc.Frames.Add(BitmapFrame.Create(bitmapImage));
                enc.Save(outStream);
                Bitmap bitmap = new Bitmap(outStream);
                return new Bitmap(bitmap);
            }
        }

        public static Bitmap CreateBitmap(int width, int height, float dpi)
        {
            Bitmap bmp = new Bitmap(1, 1);

            try { bmp = new Bitmap(width, height); }
            catch (ArgumentException ex)
            {
                //This error could occur if the bitmap takes up too much memory.
                Console.WriteLine("Error creating bitmap.\n" + ex.Message);
            }

            bmp.SetResolution(dpi, dpi);
            return bmp;
        }

        public static void DrawGrid(System.Windows.Media.DrawingContext dc, float square,
            System.Windows.Media.Pen pen, double maxWidth, double maxHeight)
        {
            float x = -square; float y = -square;

            while ((x += square) < maxWidth)
                dc.DrawLine(pen, new System.Windows.Point(x, 0),
                    new System.Windows.Point(x, maxHeight));

            while ((y += square) < maxHeight)
                dc.DrawLine(pen, new System.Windows.Point(0, y),
                    new System.Windows.Point(maxWidth, y));
        }

        public static void DrawGrid(System.Drawing.Graphics g, float square,
            System.Drawing.Pen pen, int maxWidth, int maxHeight)
        {
            float x = -square; float y = -square;

            while ((x += square) < maxWidth)
                g.DrawLine(pen, new System.Drawing.Point((int)Math.Round(x), 0),
                    new System.Drawing.Point((int)Math.Round(x), maxHeight));

            while ((y += square) < maxHeight)
                g.DrawLine(pen, new System.Drawing.Point(0, (int)Math.Round(y)),
                    new System.Drawing.Point(maxWidth, (int)Math.Round(y)));
        }

        public static void DrawText(System.Windows.Media.DrawingContext dc, FormattedText text, double x, double y, bool background = true)
        {
            System.Windows.Media.Brush brush = new SolidColorBrush(
                System.Windows.Media.Color.FromArgb(190, 255, 255, 255));
            int pad = 1;

            //Draw semi transparent rectangle behind text
            Rect size = new Rect(x - pad, y - pad, text.Width + 2 * pad, text.Height + 2 * pad);
            dc.DrawRoundedRectangle(brush, new System.Windows.Media.Pen(brush, 0), size, pad, pad);
            dc.DrawText(text, new System.Windows.Point(x, y));
        }

        public static void DrawText(System.Drawing.Graphics g, FormattedText text, float fontEmSize, double x, double y, bool background = true)
        {
            Font font = new Font("Arial", fontEmSize, System.Drawing.FontStyle.Regular);
            int pad = 1;
            SizeF textSize = g.MeasureString(text.Text, font);

            //Draw semi transparent rectangle behind text
            System.Drawing.Brush brush = new SolidBrush(System.Drawing.Color.FromArgb(190, 255, 255, 255));
            Rectangle box = new Rectangle((int)x - pad, (int)y - pad, (int)textSize.Width - 1, (int)textSize.Height - 1);
            if (background) g.FillRectangle(brush, box);

            g.DrawString(text.Text, font, System.Drawing.Brushes.Black, (float)(x - 2), (float)(y - 2));
        }

        public static void SavePdf(string filePath, System.Drawing.Image[] pageImages, string docInfoTitle = "")
        {
            PdfSharp.Pdf.PdfDocument document = new PdfDocument();
            document.Info.Title = docInfoTitle;

            for (int i = 0; i < pageImages.Length; i++)
            {
                PdfPage page = document.AddPage();
                XGraphics gfx = XGraphics.FromPdfPage(page);

                System.Drawing.Image img = pageImages[i];
                float margin = 20;
                double ratio = ImageTools.GetImageResizeRatio(img, page.Width - 2 * margin, page.Height - 2 * margin);

                gfx.DrawImage(XImage.FromGdiPlusImage(pageImages[i]), margin, margin, ratio * img.Width, ratio * img.Height);
            }

            try { document.Save(filePath); }
            catch (Exception) { Console.WriteLine("Error saving PDF file."); }
        }

        public static double GetImageResizeRatio(System.Drawing.Image img, double maxWidth, double maxHeight)
        {
            double ratio = 0;

            // Figure out the ratio and use whichever multiplier is smaller
            double ratioX = maxWidth / img.Width;
            double ratioY = maxHeight / img.Height;
            ratio = ratioX < ratioY ? ratioX : ratioY;

            return ratio;
        }
    }
}
