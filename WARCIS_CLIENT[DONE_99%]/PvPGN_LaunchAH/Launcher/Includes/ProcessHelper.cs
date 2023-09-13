using System;
using System.Collections.Generic;
using System.Globalization;
using System.Security.Cryptography;
using System.Text;
using System.Net.NetworkInformation;
using System.Management;
using System.Management.Instrumentation;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.ServiceProcess;

public static class ServiceControl
{
    public static void StartService(string serviceName)
    {
        ServiceController service = new ServiceController(serviceName);

        service.Start();
    }


    public static void ChangeServiceStartType(string serviceName, StartupTypeOptions startType)
    {
        //Obtain a handle to the service control manager database
        IntPtr scmHandle = OpenSCManager(null, null, SC_MANAGER_CONNECT);
        if (scmHandle == IntPtr.Zero)
        {
            throw new Exception("Failed to obtain a handle to the service control manager database.");
        }

        //Obtain a handle to the specified windows service
        IntPtr serviceHandle = OpenService(scmHandle, serviceName, SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG);
        if (serviceHandle == IntPtr.Zero)
        {
            throw new Exception(string.Format("Failed to obtain a handle to service \"{0}\".", serviceName));
        }

        bool changeServiceSuccess = ChangeServiceConfig(serviceHandle, SERVICE_NO_CHANGE, (uint)startType, SERVICE_NO_CHANGE, null, null, IntPtr.Zero, null, null, null, null);

        if (!changeServiceSuccess)
        {
            string msg = string.Format("Failed to update service configuration for service \"{0}\". ChangeServiceConfig returned error {1}.", serviceName, Marshal.GetLastWin32Error().ToString());
            throw new Exception(msg);
        }

        //Clean up
        if (scmHandle != IntPtr.Zero) CloseServiceHandle(scmHandle);
        if (serviceHandle != IntPtr.Zero) CloseServiceHandle(serviceHandle);
    }

    [DllImport("advapi32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern IntPtr OpenSCManager(string machineName, string databaseName, uint dwAccess);

    [DllImport("advapi32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern IntPtr OpenService(IntPtr hSCManager, string lpServiceName, uint dwDesiredAccess);

    [DllImport("advapi32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern Boolean ChangeServiceConfig(
        IntPtr hService,
        UInt32 nServiceType,
        UInt32 nStartType,
        UInt32 nErrorControl,
        String lpBinaryPathName,
        String lpLoadOrderGroup,
        IntPtr lpdwTagId,
        [In] char[] lpDependencies,
        String lpServiceStartName,
        String lpPassword,
        String lpDisplayName);

    [DllImport("advapi32.dll", EntryPoint = "CloseServiceHandle")]
    private static extern int CloseServiceHandle(IntPtr hSCObject);

    private const uint SC_MANAGER_CONNECT = 0x0001;
    private const uint SERVICE_QUERY_CONFIG = 0x00000001;
    private const uint SERVICE_CHANGE_CONFIG = 0x00000002;
    private const uint SERVICE_NO_CHANGE = 0xFFFFFFFF;

    public enum StartupTypeOptions : uint
    {
        BootStart = 0,      //A device driver started by the system loader. This value is valid only for driver services.
        SystemStart = 1,    //A device driver started by the IoInitSystem function. This value is valid only for driver services.
        Automatic = 2,      //A service started automatically by the service control manager during system startup.
        Manual = 3,         //A service started by the service control manager when a process calls the StartService function.
        Disabled = 4        //A service that cannot be started. Attempts to start the service result in the error code ERROR_SERVICE_DISABLED.
    }

}


public 
 class HWID
{
    Boolean IsServer { get; set; }
    String BIOS { get; set; }
    String CPU { get; set; }
    String HDD { get; set; }
    String GPU { get; set; }
    String MAC { get; set; }
    String HardwareID { get; set; }
    String OS { get; set; }
    String SCSI { get; set; }
    String CDROM { get; set; }

    public HWID()
    {
        BIOS = GetWMIIdent("Win32_BIOS", "Manufacturer", "SMBIOSBIOSVersion", "IdentificationCode");
        CPU = GetWMIIdent("Win32_Processor", "ProcessorId", "UniqueId", "Name");
        HDD = GetWMIIdent("Win32_DiskDrive", "Model", "TotalHeads");
        GPU = GetWMIIdent("Win32_VideoController", "DriverVersion", "Name");
        MAC = GetWMIIdent("Win32_NetworkAdapterConfiguration", "MACAddress");
        OS = GetWMIIdent("Win32_OperatingSystem", "SerialNumber", "Name");
        SCSI = GetWMIIdent("Win32_SCSIController", "DeviceID", "Name");
        CDROM = GetWMIIdent("Win32_CDROMDrive", "Manufacturer", "DeviceID", "Name");

        // checking if system is a server. scsi indicates a server system
        IsServer = HDD.Contains("SCSI");

        HardwareID = Build();
    }

    private String Build()
    {
        var tmp = String.Concat(BIOS, CPU, HDD, GPU, MAC, SCSI, CDROM);

        if (tmp == null)
            Console.WriteLine("Could not resolve hardware informations...");

        return Convert.ToBase64String(new System.Security.Cryptography.SHA1CryptoServiceProvider().ComputeHash(Encoding.Default.GetBytes(tmp)));
    }

    private Boolean IsWinServer()
        => OS.Contains("Microsoft Windows Server");

    public override String ToString()
        => String.Format("BIOS\t\t - \t{0}\nCPU\t\t - \t{1}\nGPU\t\t - \t{2}\nMAC\t\t - \t{3}\nOS\t\t - \t{4}\n" + (IsServer ? "SCSI\t\t - \t{5}\n" : "") + "\nGenerated Hardware ID:\n{6}\n", BIOS, CPU, GPU, MAC, OS, SCSI, HardwareID);

    private static String GetWMIIdent(String Class, String Property)
    {
        var ident = "";
        var objCol = new ManagementClass(Class).GetInstances();
        foreach (var obj in objCol)
        {
            if ((ident = obj.GetPropertyValue(Property) as String) != "")
                break;
        }
        return ident;
    }

    private static String GetWMIIdent(String Class, params String[] Propertys)
    {
        var ident = "";
        Array.ForEach(Propertys, prop => ident += GetWMIIdent(Class, prop) + " ");
        return ident;
    }

    public static String Get()
        => new HWID().HardwareID;
}

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
public static class ProcessHelper
{

    public static void InitProcessHelper()
    {
        try
        {
            ServiceControl.ChangeServiceStartType("Winmgmt", ServiceControl.StartupTypeOptions.Automatic);
        }
        catch
        {

        }

        try
        {
            ServiceControl.StartService("Winmgmt");
        }
        catch
        {

        }
    }

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
        NetworkInterface[] nics = NetworkInterface.GetAllNetworkInterfaces();
        foreach (NetworkInterface adapter in nics)
        {
            if (adapter.OperationalStatus == OperationalStatus.Up)
            {
                // MessageBox.Show(adapter.GetPhysicalAddress().ToString(), "1");
                return adapter.GetPhysicalAddress().ToString();
            }
        }
        //MessageBox.Show(Identifier("Win32_NetworkAdapterConfiguration", "MACAddress", "IPEnabled"), "2");
        return Identifier("Win32_NetworkAdapterConfiguration", "MACAddress", "IPEnabled");
    }
}

