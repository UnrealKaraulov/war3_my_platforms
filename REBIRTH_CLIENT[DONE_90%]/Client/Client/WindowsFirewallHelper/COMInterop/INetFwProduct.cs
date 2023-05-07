﻿using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using WindowsFirewallHelper.InternalHelpers;

#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member
namespace WindowsFirewallHelper.COMInterop
{
    [Guid("71881699-18F4-458B-B892-3FFCE5E07F75")]
    [ComImport]
    [ComClassProgId("HNetCfg.FwProduct")]
    public interface INetFwProduct
    {
        [DispId(1)]
        object RuleCategories
        {
            [DispId(1)]
            [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
            [return: MarshalAs(UnmanagedType.Struct)]
            get;
            [DispId(1)]
            [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
            [param: MarshalAs(UnmanagedType.Struct)]
            [param: In]
            set;
        }

        [DispId(2)]
        string DisplayName
        {
            [DispId(2)]
            [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
            [return: MarshalAs(UnmanagedType.BStr)]
            get;
            [DispId(2)]
            [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
            [param: MarshalAs(UnmanagedType.BStr)]
            [param: In]
            set;
        }

        [DispId(3)]
        string PathToSignedProductExe
        {
            [DispId(3)]
            [MethodImpl(MethodImplOptions.InternalCall, MethodCodeType = MethodCodeType.Runtime)]
            [return: MarshalAs(UnmanagedType.BStr)]
            get;
        }
    }
}
#pragma warning restore CS1591 // Missing XML comment for publicly visible type or member