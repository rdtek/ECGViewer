using Microsoft.Win32;
using System.IO;
using System.Windows;
using System.Collections.Generic;

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
            fd.Filter = "Signal Files (*.EDF, *.TXT, *.CSV) | *.TXT; *.EDF; *.CSV";
            fd.Title = "Open RAW data file.";
            fd.Multiselect = false;

            if (fd.ShowDialog() == true)
            {
                var signalSamples = ReadSignalFile(fd.FileName);
                ECG.SetSignalData(signalSamples);
            }
        }

        private float[] ReadSignalFile(string fileName)
        {
            var signalSamples = new List<float>();

            if (File.Exists(fileName))
            {
                string ext = Path.GetExtension(fileName).Replace(".", "");

                if (ext.ToUpper() == "CSV" || ext.ToUpper() == "TXT") {
                    using (var reader = new StreamReader(fileName)) {
                        string line;
                        while ((line = reader.ReadLine()) != null) {
                            float sample;
                            if (float.TryParse(line, out sample))
                                signalSamples.Add(sample);
                        }
                    }
                }

                if (ext.ToUpper() == "EDF")
                {
                    //TODO: use COM object to read EDF file
                }
            }

            return signalSamples.ToArray();
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
