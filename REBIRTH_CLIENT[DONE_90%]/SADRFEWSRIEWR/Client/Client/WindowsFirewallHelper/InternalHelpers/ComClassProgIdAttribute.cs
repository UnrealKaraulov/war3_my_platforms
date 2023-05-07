﻿using System;
using System.Linq;

namespace WindowsFirewallHelper.InternalHelpers
{
    internal class ComClassProgIdAttribute : Attribute
    {
        public ComClassProgIdAttribute(string classProgId)
        {
            ClassProgId = classProgId;
        }

        public string ClassProgId { get; }

        public static string GetClassProgId<T>()
        {
            return typeof(T)
                .GetCustomAttributes(typeof(ComClassProgIdAttribute), true)
                .OfType<ComClassProgIdAttribute>()
                .FirstOrDefault()?.ClassProgId;
        }
    }
}