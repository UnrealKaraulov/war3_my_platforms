using System;
using System.Collections.Generic;
using System.Globalization;
using System.Security.Cryptography;
using System.Text;

public sealed class Crc32 : HashAlgorithm
{
    public const UInt32 DefaultPolynomial = 0xedb88320u;
    public const UInt32 DefaultSeed = 0xffffffffu;

    static UInt32[] defaultTable;

    readonly UInt32 seed;
    readonly UInt32[] table;
    UInt32 hash;

    public Crc32()
        : this(DefaultPolynomial, DefaultSeed)
    {
    }

    public Crc32(UInt32 polynomial, UInt32 seed)
    {
        table = InitializeTable(polynomial);
        this.seed = hash = seed;
    }

    public override void Initialize()
    {
        hash = seed;
    }

    protected override void HashCore(byte[] array, int ibStart, int cbSize)
    {
        hash = CalculateHash(table, hash, array, ibStart, cbSize);
    }

    protected override byte[] HashFinal()
    {
        var hashBuffer = UInt32ToBigEndianBytes(~hash);
        HashValue = hashBuffer;
        return hashBuffer;
    }

    public override int HashSize { get { return 32; } }

    public static UInt32 Compute(byte[] buffer)
    {
        return Compute(DefaultSeed, buffer);
    }

    public static UInt32 Compute(UInt32 seed, byte[] buffer)
    {
        return Compute(DefaultPolynomial, seed, buffer);
    }

    public static UInt32 Compute(UInt32 polynomial, UInt32 seed, byte[] buffer)
    {
        return ~CalculateHash(InitializeTable(polynomial), seed, buffer, 0, buffer.Length);
    }

    static UInt32[] InitializeTable(UInt32 polynomial)
    {
        if (polynomial == DefaultPolynomial && defaultTable != null)
            return defaultTable;

        var createTable = new UInt32[256];
        for (var i = 0; i < 256; i++)
        {
            var entry = (UInt32)i;
            for (var j = 0; j < 8; j++)
                if ((entry & 1) == 1)
                    entry = (entry >> 1) ^ polynomial;
                else
                    entry = entry >> 1;
            createTable[i] = entry;
        }

        if (polynomial == DefaultPolynomial)
            defaultTable = createTable;

        return createTable;
    }

    static UInt32 CalculateHash(UInt32[] table, UInt32 seed, IList<byte> buffer, int start, int size)
    {
        var hash = seed;
        for (var i = start; i < start + size; i++)
            hash = (hash >> 8) ^ table[buffer[i] ^ hash & 0xff];
        return hash;
    }

    static byte[] UInt32ToBigEndianBytes(UInt32 uint32)
    {
        var result = BitConverter.GetBytes(uint32);

        if (BitConverter.IsLittleEndian)
            Array.Reverse(result);

        return result;
    }
}
public class ProcessHelper
{

    public static string ID()
    {
        return Value();
    }
    public static string Reverse(string s)
    {
        char[] charArray = s.ToCharArray();
        Array.Reverse(charArray);
        return new string(charArray);
    }

    private static string _fingerPrint = string.Empty;
    private static string Value()
    {
        //You don't need to generate the HWID again if it has already been generated. This is better for performance
        //Also, your HWID generally doesn't change when your computer is turned on but it can happen.
        //It's up to you if you want to keep generating a HWID or not if the function is called.
        if (string.IsNullOrEmpty(_fingerPrint))
        {
            _fingerPrint = GetHash(CpuId() + "\nBIOS >> " + DiskId() + "\nMAC >> " + GetHash(CpuId()));
        }
        return _fingerPrint;
    }

    public static UInt32 Value_int1()
    {
        return Crc32.Compute(Encoding.UTF8.GetBytes(CpuId()));
    }
    public static UInt32 Value_int2()
    {
        return Crc32.Compute(Encoding.UTF8.GetBytes(BiosId()));
    }
    public static UInt32 Value_int3()
    {
        return Crc32.Compute(Encoding.UTF8.GetBytes(BaseId()));
    }
    public static UInt32 Value_int4()
    {
        return Crc32.Compute(Encoding.UTF8.GetBytes(MacId()));
    }

    public static string GetHash(string s)
    {
        //Initialize a new MD5 Crypto Service Provider in order to generate a hash
        MD5 sec = new MD5CryptoServiceProvider();
        //Grab the bytes of the variable 's'
        byte[] bt = Encoding.ASCII.GetBytes(s);
        //Grab the Hexadecimal value of the MD5 hash
        return GetHexString(sec.ComputeHash(bt));
    }

    private static string GetHexString(IList<byte> bt)
    {
        string s = string.Empty;
        for (int i = 0; i < bt.Count; i++)
        {
            byte b = bt[i];
            int n = b;
            int n1 = n & 15;
            int n2 = (n >> 4) & 15;
            if (n2 > 9)
                s += ((char)(n2 - 10 + 'A')).ToString(CultureInfo.InvariantCulture);
            else
                s += n2.ToString(CultureInfo.InvariantCulture);
            if (n1 > 9)
                s += ((char)(n1 - 10 + 'A')).ToString(CultureInfo.InvariantCulture);
            else
                s += n1.ToString(CultureInfo.InvariantCulture);
            if ((i + 1) != bt.Count && (i + 1) % 2 == 0) s += "-";
        }
        return s;
    }

    //Return a hardware identifier
    private static string Identifier(string wmiClass, string wmiProperty, string wmiMustBeTrue)
    {
        string result = "";
        System.Management.ManagementClass mc = new System.Management.ManagementClass(wmiClass);
        System.Management.ManagementObjectCollection moc = mc.GetInstances();
        foreach (System.Management.ManagementBaseObject mo in moc)
        {
            if (mo[wmiMustBeTrue].ToString() != "True") continue;
            //Only get the first one
            if (result != "") continue;
            try
            {
                result = mo[wmiProperty].ToString();
                break;
            }
            catch
            {
            }
        }
        return result;
    }
    //Return a hardware identifier
    private static string Identifier(string wmiClass, string wmiProperty)
    {
        string result = "";
        System.Management.ManagementClass mc = new System.Management.ManagementClass(wmiClass);
        System.Management.ManagementObjectCollection moc = mc.GetInstances();
        foreach (System.Management.ManagementBaseObject mo in moc)
        {
            //Only get the first one
            if (result != "") continue;
            try
            {
                result = mo[wmiProperty].ToString();
                break;
            }
            catch
            {
            }
        }
        return result;
    }
    private static string CpuId()
    {
        //Uses first CPU identifier available in order of preference
        //Don't get all identifiers, as it is very time consuming
        string retVal = Identifier("Win32_Processor", "UniqueId");
        if (retVal != "") return retVal;
        retVal = Identifier("Win32_Processor", "ProcessorId");
        if (retVal != "") return retVal;
        retVal = Identifier("Win32_Processor", "Name");
        if (retVal == "") //If no Name, use Manufacturer
        {
            retVal = Identifier("Win32_Processor", "Manufacturer");
        }
        //Add clock speed for extra security
        retVal += Identifier("Win32_Processor", "MaxClockSpeed");
        return retVal;
    }
    //BIOS Identifier
    private static string BiosId()
    {
        return Identifier("Win32_BIOS", "Manufacturer") + Identifier("Win32_BIOS", "IdentificationCode") + Identifier("Win32_BIOS", "SerialNumber");
    }
    //Main physical hard drive ID
    private static string DiskId()
    {
        return "_";
    }
    //Motherboard ID
    private static string BaseId()
    {
        return Identifier("Win32_BaseBoard", "Model") + Identifier("Win32_BaseBoard", "Manufacturer") + Identifier("Win32_BaseBoard", "Name") + Identifier("Win32_BaseBoard", "SerialNumber");
    }
    //Primary video controller ID
    private static string VideoId()
    {
        return Identifier("Win32_VideoController", "DriverVersion") + Identifier("Win32_VideoController", "Name");
    }
    //First enabled network card ID
    private static string MacId()
    {
        return Identifier("Win32_NetworkAdapterConfiguration", "MACAddress", "IPEnabled");
    }
}

