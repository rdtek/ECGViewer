using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ECGViewerWPF
{

    public partial class NumberStepper : UserControl, INotifyPropertyChanged
    {
        public static readonly DependencyProperty NumValueProperty
            = DependencyProperty.Register("NumValue", typeof(double),
                typeof(NumberStepper), new PropertyMetadata((double)1));

        public static readonly DependencyProperty DecimalPlacesProperty
            = DependencyProperty.Register("DecimalPlaces", typeof(int),
                typeof(NumberStepper), new PropertyMetadata(0));

        public static readonly DependencyProperty MinValueProperty
            = DependencyProperty.Register("MinValue", typeof(double),
                typeof(NumberStepper), new PropertyMetadata((double)0));

        public static readonly DependencyProperty MaxValueProperty
            = DependencyProperty.Register("MaxValue", typeof(double),
                typeof(NumberStepper), new PropertyMetadata((double)1000));

        public static readonly DependencyProperty IncrementProperty
            = DependencyProperty.Register("Increment", typeof(double),
                typeof(NumberStepper), new PropertyMetadata((double)1));

        public delegate void ValueChanged(double oldValue, double newValue);
        public ValueChanged OnValueChanged { get; set; }

        public event PropertyChangedEventHandler PropertyChanged;
        private void RaisePropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public NumberStepper() { InitializeComponent(); }

        public double MinValue
        {
            get { return (double)GetValue(MinValueProperty); }
            set { SetValue(MinValueProperty, value); }
        }

        public double MaxValue
        {
            get { return (double)GetValue(MaxValueProperty); }
            set
            {
                if (NumValue >= MaxValue) NumValue = MaxValue;
                SetValue(MaxValueProperty, value);
            }
        }

        public double Increment
        {
            get { return (double)GetValue(IncrementProperty); }
            set { SetValue(IncrementProperty, value); }
        }

        public int DecimalPlaces
        {
            get { return (int)GetValue(DecimalPlacesProperty); }
            set { SetValue(DecimalPlacesProperty, value); }
        }

        public double NumValue
        {
            get { return (double)GetValue(NumValueProperty); }

            set
            {
                double oldValue = (double)GetValue(NumValueProperty);
                SetValue(NumValueProperty, value);
                RaisePropertyChanged("NumValue");
                if (OnValueChanged != null) OnValueChanged(oldValue, value);
            }
        }

        private void CmdUp_Click(object sender, RoutedEventArgs e)
        {
            if (NumValue + Increment <= MaxValue) { NumValue += Increment; FormatText(); }
        }

        private void CmdDown_Click(object sender, RoutedEventArgs e)
        {
            if (NumValue - Increment >= MinValue) { NumValue -= Increment; FormatText(); }
        }

        private void FormatText()
        {
            string format = "0.";
            for (int i = 0; i < DecimalPlaces; i++) format += "0";
            TxtNum.Text = string.Format("{0:" + format + "}", NumValue);
        }

        private void TxtNum_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!TxtNum.IsFocused) FormatText();
        }

        private void TxtNum_KeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            e.Handled = !IsNumberKey(e.Key) && !IsActionKey(e.Key);
            if (e.Key == Key.Enter) NumValue = double.Parse(TxtNum.Text);
        }

        private void TxtNum_LostFocus(object sender, RoutedEventArgs e)
        {
            try
            {
                double num = double.Parse(TxtNum.Text);
                if (num > MaxValue) NumValue = MaxValue;
                if (num < MinValue) NumValue = MinValue;
            }
            catch (Exception) { }
        }

        private bool IsNumberKey(Key inKey)
        {
            if (inKey == Key.OemPeriod) return true;

            if (inKey < Key.D0 || inKey > Key.D9)
            {
                if (inKey < Key.NumPad0 || inKey > Key.NumPad9) return false;
            }
            return true;
        }

        private bool IsActionKey(Key inKey)
        {
            return inKey == Key.Delete
                || inKey == Key.Back
                || inKey == Key.Tab
                || inKey == Key.Return
                || Keyboard.Modifiers.HasFlag(ModifierKeys.Alt);
        }
    }
}
