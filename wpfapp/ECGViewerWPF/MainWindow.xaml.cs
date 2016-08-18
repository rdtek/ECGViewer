using Microsoft.Win32;
using System.IO;
using System.Windows;

namespace ECGViewerWPF
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void MenuItem_FileOpen_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog fd = new OpenFileDialog();
            fd.Filter = "Signal Files(*.EDF, *.CSV) | *.EDF; *.CSV";
            fd.Title = "Open RAW data file.";
            string fileName = "";

            if (fd.ShowDialog() == true)
            {
                fileName = fd.FileName;

                if (File.Exists(fileName)) {
                    
                    string ext = Path.GetExtension(fileName);

                    //TODO: read the signal data from the file
                    if (ext.ToUpper() == "CSV")
                    {

                    }

                    if (ext.ToUpper() == "EDF")
                    {

                    }
                }
            }
        }

        private void MenuItem_FileClose_Click(object sender, RoutedEventArgs e)
        {

        }

        private void MenuItem_FileSave_Click(object sender, RoutedEventArgs e)
        {

        }

        private void MenuItem_FileExit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }
    }
}
