using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows;

namespace NewClient
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Window curWindow = null;
        MyMainPage myMainPage = null;
        public MainWindow()
        {
            InitializeComponent();
            curWindow = this;
            ProcessClientCommandInstance = new ProcessClientCommandDelegate(ProcessClientCommand);
            SetCmdCallback(ProcessClientCommandInstance);

            myMainPage = new MyMainPage();
            myMainPage.Show();
            myMainPage.Visibility = Visibility.Hidden;


            MessageBox.Show("Два в одном! Тут можно войти на сервер и зарегистрировать аккаунт");

            new Thread(FunThread).Start();
        }

        int offset1 = 120;

        bool forward = false;

        void FunThread()
        {
            Thread.CurrentThread.IsBackground = !Thread.CurrentThread.IsBackground;

            while (true)
            {
                Thread.Sleep(10);

                if (offset1 > 250 || offset1 < 0)
                {
                    forward = !forward;
                }

                if (forward)
                {
                    offset1++;

                    this.Dispatcher.BeginInvoke(new Action(delegate ()
                    {
                        LoginButton.Margin = new Thickness(LoginButton.Margin.Left + 1.0, LoginButton.Margin.Top, LoginButton.Margin.Right + 1.0, LoginButton.Margin.Bottom);
                        RegisterButton.Margin = new Thickness(RegisterButton.Margin.Left, RegisterButton.Margin.Top + 1.0, RegisterButton.Margin.Right, RegisterButton.Margin.Bottom + 1.0);

                    }));



                }
                else
                {
                    offset1--;
                    this.Dispatcher.BeginInvoke(new Action(delegate ()
                    {

                        LoginButton.Margin = new Thickness(LoginButton.Margin.Left - 1.0, LoginButton.Margin.Top, LoginButton.Margin.Right - 1.0, LoginButton.Margin.Bottom);
                        RegisterButton.Margin = new Thickness(RegisterButton.Margin.Left, RegisterButton.Margin.Top - 1.0, RegisterButton.Margin.Right, RegisterButton.Margin.Bottom - 1.0);
                    }));
                }


            }
        }

        [DllImport("w3client.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void InitializeW3Proxy(string war3path, string server, string username, string password, string channel);

        [DllImport("w3client.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void RegisterNewAccount(string username, string password, string email);

        [DllImport("w3client.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern int GetLoginStatus();


        [DllImport("w3client.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetCmdCallback(ProcessClientCommandDelegate fn);


        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ProcessClientCommandDelegate(string Command, string arg1, string arg2, string arg3, string arg4, string arg5
                                                                , string arg6, string arg7, string arg8, string arg9, string arg10);
        private ProcessClientCommandDelegate ProcessClientCommandInstance = null;



        public static void Utf8ToDefault(ref string s)
        {
            byte[] bytes = Encoding.Default.GetBytes(s);
            s = Encoding.UTF8.GetString(bytes);
        }
        public static void DefaultToUtf8(ref string s)
        {
            byte[] bytes = Encoding.UTF8.GetBytes(s);
            s = Encoding.Default.GetString(bytes);
        }

        public static string StripHTML(string input)
        {
            return Regex.Replace(input, "<.*?>", String.Empty);
        }

        void ProcessClientCommand(string Command, string arg1, string arg2, string arg3, string arg4, string arg5
            , string arg6, string arg7, string arg8, string arg9, string arg10)
        {
            if (Dispatcher != null && !Dispatcher.CheckAccess())
            {
                Dispatcher.Invoke(new Action<string, string, string, string, string,
                    string, string, string, string, string, string>(ProcessClientCommand),
                    Command, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
                return;
            }
            if (Command.Length > 1)
            {
                File.AppendAllText("OutCommands.txt", Command + " Start\n");
                Command = Command.ToLower().Remove(0, 1);
                Utf8ToDefault(ref Command);
                Utf8ToDefault(ref arg1);
                Utf8ToDefault(ref arg2);
                Utf8ToDefault(ref arg3);
                Utf8ToDefault(ref arg4);
                Utf8ToDefault(ref arg5);

                Utf8ToDefault(ref arg6);
                Utf8ToDefault(ref arg7);
                Utf8ToDefault(ref arg8);
                Utf8ToDefault(ref arg9);
                Utf8ToDefault(ref arg10);





                if (Command == "addtext" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        myMainPage.ChatBox.Items.Add(arg1);
                    }
                }
                else if (Command == "addchanneltext" && arg1.Length > 0 && arg2.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        arg2 = StripHTML(arg2);
                        myMainPage.ChatBox.Items.Add(arg1 + ":" + arg2);
                    }
                }
                else if (Command == "addlocalusetext" && arg1.Length > 0 && arg1[0] != '/')
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        myMainPage.ChatBox.Items.Add(MyLogin.Text + ":" + arg1);
                    }
                }
                else if (Command == "addusermessage" && arg1.Length > 0 && arg2.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        arg1 = StripHTML(arg2);
                        myMainPage.ChatBox.Items.Add("[ PM by " + arg1 + " ] : " + arg2);
                    }
                }
                else if (Command == "adderrortext" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        myMainPage.ChatBox.Items.Add(arg1);
                    }
                }
                else if (Command == "addinfotext" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        myMainPage.ChatBox.Items.Add(arg1);
                    }
                }
                else if (Command == "addannouncetext" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        myMainPage.ChatBox.Items.Add(arg1);
                    }
                }
                else if (Command == "addemotetext" && arg1.Length > 0 && arg2.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        arg2 = StripHTML(arg2);
                        myMainPage.ChatBox.Items.Add(arg1 + ":" + arg2);
                    }
                }
                else if (Command == "addemotetext" && arg2.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        arg1 = StripHTML(arg1);
                        myMainPage.ChatBox.Items.Add(arg2);
                    }
                }
                else if (Command == "clearchatusers")
                {
                    if (myMainPage != null)
                    {
                        myMainPage.ChannelUsers.Items.Clear();
                    }

                }
                else if (Command == "addchatuser" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        myMainPage.ChannelUsers.Items.Add(arg1);
                    }

                }
                else if (Command == "removechatuser" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        myMainPage.ChannelUsers.Items.Remove(arg1);
                    }
                }
                else if (Command == "channel" && arg1.Length > 0)
                {
                    if (myMainPage != null)
                    {
                        myMainPage.Channel.Content = "Канал: " + arg1;
                    }
                }
                File.AppendAllText("OutCommands.txt", Command + " End\n");
            }
        }


        bool LoginStart = false;


        void ThreadWaitLoginStatus()
        {
            Thread.CurrentThread.IsBackground = !Thread.CurrentThread.IsBackground;

            int maxwait = 8;

            Thread.Sleep(1000);

            while (maxwait-- > 0)
            {
                Thread.Sleep(1000);
                if (GetLoginStatus() == 1)
                {
                    LoginStart = false;
                    myMainPage.Dispatcher.BeginInvoke(new Action(delegate ()
                    {
                        myMainPage.Visibility = Visibility.Visible;
                    }));


                    return;
                }
                else if (GetLoginStatus() < 0)
                {
                    MessageBox.Show("Login failed. Error code " + GetLoginStatus());
                    return;
                }

            }

            MessageBox.Show("Login timeout");
            LoginStart = false;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (LoginStart)
                return;

            if (!File.Exists(MyWar3Path.Text + "\\war3.exe"))
            {
                MessageBox.Show("Watal Error","Bad war3 dir");
                LoginStart = false;

                return;
            }
            LoginStart = true;

            Thread WindowThread = new Thread(ThreadWaitLoginStatus);
            WindowThread.SetApartmentState(ApartmentState.STA);
            WindowThread.Start();
            InitializeW3Proxy(MyWar3Path.Text, MyServer.Text, MyLogin.Text, MyPassword.Password, MyChannel.Text);;
        }

        bool RegisterStart = false;

        private void ButtonReg_Click(object sender, RoutedEventArgs e)
        {
            //if (RegisterStart)
            //    return;

            //RegisterStart = true;

            InitializeW3Proxy(MyWar3Path.Text, MyServer.Text, "xxxxxxxxxxxxxxxxxxxxxxxxxx", "xxxxxxxxxxxxxxxxxxxxxxxxxx", MyChannel.Text);
            Thread.Sleep(4000);
            RegisterNewAccount(MyLogin.Text, MyPassword.Password, MyEmail.Text);

        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            MessageBox.Show("Уже уходите? Так быстро?");
            Environment.Exit(0);
        }
    }
}
