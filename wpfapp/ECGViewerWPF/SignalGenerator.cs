using System;
using System.Collections.Generic;

namespace ECGViewerWPF
{
    class SignalGenerator
    {
        // Name:   GenerateSignal
        // Desc:   Generate a simulated ECG signal.
        public static float[] GenerateSignal()
        {
            var signalSamples = new List<float>();
            uint numBeats = 50;

            //Generate each part of the heartbeat waveform P-Q-R-S
            for (uint i = 0; i < numBeats; i++)
            {
                GenerateLine         (ref signalSamples, 300, 0);    // Flatline
                GenerateGausianCurve (ref signalSamples, 0, 2, 200); // P-Wave
                GenerateLine         (ref signalSamples, 50, 0);     // PR segment
                GenerateLine         (ref signalSamples, 50, -2);    // 'Q' - dip down
                GenerateLine         (ref signalSamples, 20, 25);    // 'R' - jump up
                GenerateLine         (ref signalSamples, 22, -25);   // 'S' - jump down
                GenerateLine         (ref signalSamples, 20, 7);     // Back up to baseline
                GenerateLine         (ref signalSamples, 50, 0);     // ST segment
                GenerateGausianCurve (ref signalSamples, 0, 2, 200); // T-Wave
            }

            return signalSamples.ToArray();
        }

        // Name:   GenerateLine
        // Desc:   Generates a line with a gradient (rise or fall or flat)
        private static void GenerateLine(ref List<float> signalSamples, uint length, float rise)
        {
            uint maxLength = 1000;

            for (uint i = 0; i < length && i < maxLength; i++)
            {
                float previousYValue = signalSamples.Count >= 1 
                    ? signalSamples[signalSamples.Count - 1] : 0;

                signalSamples.Add( previousYValue + rise );
            }
        }

        // Name:   CalculateGausianPDF
        // Desc:   Calculates the Probability Density Function value for position
        //         on the curve.
        private static float CalculateGausianPDF(float x, float mu, float variance)
        {
            float oneOverSqrt2VarPi = 1 / (float)Math.Sqrt(2 * variance * Math.PI);
            float expo = (float)Math.Exp(-1 * Math.Pow((x - mu), 2) / (2 * Math.Pow(variance, 2)));
            return oneOverSqrt2VarPi * expo;
        }

        // Name:   GenerateGausianCurve
        // Desc:   Generates a Gausian Distribution curve
        private static void GenerateGausianCurve(ref List<float> signalSamples, float mu, float variance, float scaleFactor)
        {
            int sampleIndex = signalSamples.Count;
            for (float x = -3; x <= 3; x += 0.1f)
            {
                float xpdf = scaleFactor * CalculateGausianPDF(x, mu, variance);
                signalSamples.Add(xpdf);
            }
        }
    }
}
