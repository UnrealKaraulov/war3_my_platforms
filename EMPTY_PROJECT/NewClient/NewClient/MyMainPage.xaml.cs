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
    /// Логика взаимодействия для MyMainPage.xaml
    /// </summary>
    public partial class MyMainPage : Window
    {
        public MyMainPage()
        {
            InitializeComponent();
        }

        [DllImport("w3client.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SendServerChatMessage(string text);


        private void TextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                SendServerChatMessage(Message.Text);
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (MessageBox.Show("Вы действительно хотите отправить сообщение с помощью этой кнопки?", "Внимание!!!", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                SendServerChatMessage(Message.Text);
            }
        }

        private void Button2_Click(object sender, RoutedEventArgs e)
        {
            if (MessageBox.Show("Вы действительно хотите создать игру с помощью этой кнопки?", "Внимание!!!", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                CreateGameWindow createGameWindow = new CreateGameWindow();
                createGameWindow.ShowDialog();
            }
        }
    }
}
