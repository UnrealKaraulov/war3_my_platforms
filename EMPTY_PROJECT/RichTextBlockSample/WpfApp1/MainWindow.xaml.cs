using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace WpfApp1
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            _htmlData.Text = "<strong>hello</strong> world";
            _htmlPanel.Text = _htmlData.Text;
        }

        private void OnHtmlControl_click(object sender, MouseButtonEventArgs e)
        {

        }

        private void _htmlData_TextChanged(object sender, TextChangedEventArgs e)
        {
            try { _htmlPanel.Text = _htmlData.Text; }
            catch { }
        }
    }
}
