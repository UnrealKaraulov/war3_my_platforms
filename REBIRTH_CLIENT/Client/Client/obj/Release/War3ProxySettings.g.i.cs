﻿#pragma checksum "..\..\War3ProxySettings.xaml" "{8829d00f-11b8-4213-878b-770e8597ac16}" "D782B52BB20C3A822F12CA22B23149EBC0E1017F73BB82A6FD05F28A16E0DEAC"
//------------------------------------------------------------------------------
// <auto-generated>
//     Этот код создан программой.
//     Исполняемая версия:4.0.30319.42000
//
//     Изменения в этом файле могут привести к неправильной работе и будут потеряны в случае
//     повторной генерации кода.
// </auto-generated>
//------------------------------------------------------------------------------

using Client;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Media.TextFormatting;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Shell;


namespace Client {
    
    
    /// <summary>
    /// War3ProxySettings
    /// </summary>
    public partial class War3ProxySettings : System.Windows.Window, System.Windows.Markup.IComponentConnector {
        
        
        #line 20 "..\..\War3ProxySettings.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox War3PathTextBox;
        
        #line default
        #line hidden
        
        
        #line 21 "..\..\War3ProxySettings.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox ServerTextBox;
        
        #line default
        #line hidden
        
        
        #line 25 "..\..\War3ProxySettings.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.CheckBox WindowModeCheckBox;
        
        #line default
        #line hidden
        
        
        #line 26 "..\..\War3ProxySettings.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ComboBox VideoDriver;
        
        #line default
        #line hidden
        
        
        #line 32 "..\..\War3ProxySettings.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox DefaultChannelName;
        
        #line default
        #line hidden
        
        private bool _contentLoaded;
        
        /// <summary>
        /// InitializeComponent
        /// </summary>
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
        public void InitializeComponent() {
            if (_contentLoaded) {
                return;
            }
            _contentLoaded = true;
            System.Uri resourceLocater = new System.Uri("/Client;component/war3proxysettings.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\War3ProxySettings.xaml"
            System.Windows.Application.LoadComponent(this, resourceLocater);
            
            #line default
            #line hidden
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Never)]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1800:DoNotCastUnnecessarily")]
        void System.Windows.Markup.IComponentConnector.Connect(int connectionId, object target) {
            switch (connectionId)
            {
            case 1:
            
            #line 8 "..\..\War3ProxySettings.xaml"
            ((Client.War3ProxySettings)(target)).Loaded += new System.Windows.RoutedEventHandler(this.Window_Loaded);
            
            #line default
            #line hidden
            
            #line 14 "..\..\War3ProxySettings.xaml"
            ((Client.War3ProxySettings)(target)).Closed += new System.EventHandler(this.MetroWindow_Closed);
            
            #line default
            #line hidden
            return;
            case 2:
            this.War3PathTextBox = ((System.Windows.Controls.TextBox)(target));
            
            #line 20 "..\..\War3ProxySettings.xaml"
            this.War3PathTextBox.TextChanged += new System.Windows.Controls.TextChangedEventHandler(this.War3Path_TextChanged);
            
            #line default
            #line hidden
            
            #line 20 "..\..\War3ProxySettings.xaml"
            this.War3PathTextBox.MouseUp += new System.Windows.Input.MouseButtonEventHandler(this.War3PathTextBox_MouseUp);
            
            #line default
            #line hidden
            return;
            case 3:
            this.ServerTextBox = ((System.Windows.Controls.TextBox)(target));
            
            #line 21 "..\..\War3ProxySettings.xaml"
            this.ServerTextBox.TextChanged += new System.Windows.Controls.TextChangedEventHandler(this.Server_TextChanged);
            
            #line default
            #line hidden
            return;
            case 4:
            
            #line 24 "..\..\War3ProxySettings.xaml"
            ((System.Windows.Controls.Button)(target)).Click += new System.Windows.RoutedEventHandler(this.Button_Click);
            
            #line default
            #line hidden
            return;
            case 5:
            this.WindowModeCheckBox = ((System.Windows.Controls.CheckBox)(target));
            
            #line 25 "..\..\War3ProxySettings.xaml"
            this.WindowModeCheckBox.Checked += new System.Windows.RoutedEventHandler(this.WindowModeCheckBox_Checked);
            
            #line default
            #line hidden
            
            #line 25 "..\..\War3ProxySettings.xaml"
            this.WindowModeCheckBox.Unchecked += new System.Windows.RoutedEventHandler(this.WindowModeCheckBox_Checked);
            
            #line default
            #line hidden
            return;
            case 6:
            this.VideoDriver = ((System.Windows.Controls.ComboBox)(target));
            
            #line 26 "..\..\War3ProxySettings.xaml"
            this.VideoDriver.SelectionChanged += new System.Windows.Controls.SelectionChangedEventHandler(this.VideoDriver_SelectionChanged);
            
            #line default
            #line hidden
            return;
            case 7:
            this.DefaultChannelName = ((System.Windows.Controls.TextBox)(target));
            
            #line 32 "..\..\War3ProxySettings.xaml"
            this.DefaultChannelName.TextChanged += new System.Windows.Controls.TextChangedEventHandler(this.DefaultChannelName_TextChanged);
            
            #line default
            #line hidden
            return;
            }
            this._contentLoaded = true;
        }
    }
}
