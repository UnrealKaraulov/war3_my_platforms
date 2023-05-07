using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace NewClient
{
    /// <summary>
    /// Логика взаимодействия для CreateGameWindow.xaml
    /// </summary>
    public partial class CreateGameWindow : Window
    {
        public CreateGameWindow()
        {
            InitializeComponent();
        }

        [DllImport("w3client.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SendServerChatMessage(string text);


        private void Button_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
