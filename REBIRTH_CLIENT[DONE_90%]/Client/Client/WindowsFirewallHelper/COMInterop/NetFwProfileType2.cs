﻿// ReSharper disable InconsistentNaming

namespace WindowsFirewallHelper.COMInterop
{
    internal enum NetFwProfileType2
    {
        Domain = 0x00000001,
        Private = 0x00000002,
        Public = 0x00000004,
        All = 0x7FFFFFFF
    }
}