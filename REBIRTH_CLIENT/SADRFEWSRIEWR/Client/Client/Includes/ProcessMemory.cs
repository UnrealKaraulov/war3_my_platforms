// Decompiled with JetBrains decompiler
// Type: ProcessMemory
// Assembly: GL_Launcher, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: E55833E0-5717-4B5A-8E37-E6D118A47194
// Assembly location: C:\Users\Администратор\Downloads\GrandLadder\GL_Launcher.exe

using System;
using System.Collections;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.IO;

public class ProcessMemory
{
   
    [Flags]
    public enum ProcessAccessFlags : uint
    {
        All = 0x001F0FFF,
        Terminate = 0x00000001,
        CreateThread = 0x00000002,
        VirtualMemoryOperation = 0x00000008,
        VirtualMemoryRead = 0x00000010,
        VirtualMemoryWrite = 0x00000020,
        DuplicateHandle = 0x00000040,
        CreateProcess = 0x000000080,
        SetQuota = 0x00000100,
        SetInformation = 0x00000200,
        QueryInformation = 0x00000400,
        QueryLimitedInformation = 0x00001000,
        Synchronize = 0x00100000
    }

    protected int BaseAddress;
    protected Process MyProcess;
    protected ProcessModule myProcessModule;
    protected IntPtr processHandle;
    protected string ProcessName;
    protected int processID;

    public ProcessMemory(int processID, string processname)
    {
        this.processID = processID;
        this.ProcessName = processname;
    }


    public bool CheckProcess()
    {
        for (int i = 0; i < Process.GetProcessesByName(this.ProcessName).Length; i++)
        {
            if (Process.GetProcessesByName(this.ProcessName)[i].Id == this.processID)
                return true;
        }

        return false;
    }

    [DllImport("kernel32.dll")]
    public static extern bool CloseHandle(int hObject);

    public string CutString(string mystring)
    {
        char[] chArray = mystring.ToCharArray();
        string str = "";
        for (int index = 0; index < mystring.Length; ++index)
        {
            if ((int)chArray[index] == 32 && (int)chArray[index + 1] == 32 || (int)chArray[index] == 0)
                return str;
            str += chArray[index].ToString();
        }
        return mystring.TrimEnd('0');
    }

    public int DllImageAddress(string dllname)
    {
        foreach (ProcessModule processModule in MyProcess.Modules)
        {
            if (dllname.ToLower() == processModule.ModuleName.ToLower())
                return processModule.BaseAddress.ToInt32();
            /*  if ( Path.GetFileName( dllname ).ToLower( ) == Path.GetFileName( processModule.ModuleName ).ToLower( ) )
                  return processModule.BaseAddress.ToInt32( );
              if ( Path.GetFileNameWithoutExtension( dllname ).ToLower( ) == Path.GetFileNameWithoutExtension( processModule.ModuleName ).ToLower( ) )
                  return processModule.BaseAddress.ToInt32( );*/
        }
        return -1;
    }

    [DllImport("user32.dll", EntryPoint = "FindWindow", SetLastError = true)]
    public static extern int FindWindowByCaption(int ZeroOnly, string lpWindowName);

    public int ImageAddress()
    {
        this.BaseAddress = 0;
        this.myProcessModule = this.MyProcess.MainModule;
        this.BaseAddress = (int)this.myProcessModule.BaseAddress;
        return this.BaseAddress;
    }

    public int ImageAddress(int pOffset)
    {
        this.BaseAddress = 0;
        this.myProcessModule = this.MyProcess.MainModule;
        this.BaseAddress = (int)this.myProcessModule.BaseAddress;
        return pOffset + this.BaseAddress;
    }

    public string MyProcessName()
    {
        return this.ProcessName;
    }

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr OpenProcess(
      ProcessAccessFlags processAccess,
      bool bInheritHandle,
      int processId
 );
    IntPtr OpenProcess(Process proc, ProcessAccessFlags flags)
    {
        return OpenProcess(flags, false, proc.Id);
    }
    public int Pointer(bool AddToImageAddress, int pOffset)
    {
        if (AddToImageAddress)
            return this.ReadInt(this.ImageAddress(pOffset));
        return this.ReadInt(pOffset);
    }

    public int Pointer(string Module, params int[] Offsets)
    {
        int num1 = this.DllImageAddress(Module);
        foreach (int num2 in Offsets)
            num1 = this.ReadInt(num1 + num2);
        return num1;
    }

    public int Pointer(string Module, int pOffset)
    {
        return this.ReadInt(this.DllImageAddress(Module) + pOffset);
    }

    public int Pointer(bool AddToImageAddress, int pOffset, int pOffset2)
    {
        if (AddToImageAddress)
            return this.ReadInt(this.ImageAddress() + pOffset) + pOffset2;
        return this.ReadInt(pOffset) + pOffset2;
    }

    public byte ReadByte(int pOffset)
    {
        byte[] buffer = new byte[1];
        ProcessMemory.ReadProcessMemory(this.processHandle, pOffset, buffer, 1, 0);
        return buffer[0];
    }

    public byte ReadByte(bool AddToImageAddress, int pOffset)
    {
        byte[] buffer = new byte[1];
        ProcessMemory.ReadProcessMemory(this.processHandle, AddToImageAddress ? this.ImageAddress(pOffset) : pOffset, buffer, 1, 0);
        return buffer[0];
    }

    public byte ReadByte(string Module, int pOffset)
    {
        byte[] buffer = new byte[1];
        ProcessMemory.ReadProcessMemory(this.processHandle, this.DllImageAddress(Module) + pOffset, buffer, 1, 0);
        return buffer[0];
    }

    public float ReadFloat(int pOffset)
    {
        return BitConverter.ToSingle(this.ReadMem(pOffset, 4), 0);
    }

    public float ReadFloat(bool AddToImageAddress, int pOffset)
    {
        return BitConverter.ToSingle(this.ReadMem(pOffset, 4, AddToImageAddress), 0);
    }

    public float ReadFloat(string Module, int pOffset)
    {
        return BitConverter.ToSingle(this.ReadMem(this.DllImageAddress(Module) + pOffset, 4), 0);
    }

    public int ReadInt(int pOffset)
    {
        return BitConverter.ToInt32(this.ReadMem(pOffset, 4), 0);
    }

    public int ReadInt(bool AddToImageAddress, int pOffset)
    {
        return BitConverter.ToInt32(this.ReadMem(pOffset, 4, AddToImageAddress), 0);
    }

    public int ReadInt(string Module, int pOffset)
    {
        return BitConverter.ToInt32(this.ReadMem(this.DllImageAddress(Module) + pOffset, 4), 0);
    }

    public byte[] ReadMem(int pOffset, int pSize)
    {
        byte[] buffer = new byte[pSize];
        ProcessMemory.ReadProcessMemory(this.processHandle, pOffset, buffer, pSize, 0);
        return buffer;
    }

    public byte[] ReadMem(int pOffset, int pSize, bool AddToImageAddress)
    {
        byte[] buffer = new byte[pSize];
        ProcessMemory.ReadProcessMemory(this.processHandle, AddToImageAddress ? this.ImageAddress(pOffset) : pOffset, buffer, pSize, 0);
        return buffer;
    }

    [DllImport("kernel32.dll")]
    public static extern bool ReadProcessMemory(IntPtr hProcess, int BaseAddress, byte[] buffer, int size, int NumberOfBytesRead);

    public short ReadShort(int pOffset)
    {
        return BitConverter.ToInt16(this.ReadMem(pOffset, 2), 0);
    }

    public short ReadShort(bool AddToImageAddress, int pOffset)
    {
        return BitConverter.ToInt16(this.ReadMem(pOffset, 2, AddToImageAddress), 0);
    }

    public short ReadShort(string Module, int pOffset)
    {
        return BitConverter.ToInt16(this.ReadMem(this.DllImageAddress(Module) + pOffset, 2), 0);
    }

    public string ReadStringAscii(int pOffset, int pSize)
    {
        return this.CutString(Encoding.ASCII.GetString(this.ReadMem(pOffset, pSize)));
    }

    public string ReadStringAscii(bool AddToImageAddress, int pOffset, int pSize)
    {
        return this.CutString(Encoding.ASCII.GetString(this.ReadMem(pOffset, pSize, AddToImageAddress)));
    }

    public string ReadStringAscii(string Module, int pOffset, int pSize)
    {
        return this.CutString(Encoding.ASCII.GetString(this.ReadMem(this.DllImageAddress(Module) + pOffset, pSize)));
    }

    public string ReadStringWarcraft(int pOffset, int pSize)
    {
        return this.CutString(Encoding.Default.GetString(this.ReadMem(pOffset, pSize)));
    }

    public string ReadStringWarcraft(bool AddToImageAddress, int pOffset, int pSize)
    {
        return this.CutString(Encoding.Default.GetString(this.ReadMem(pOffset, pSize, AddToImageAddress)));
    }

    public string ReadStringWarcraft(string Module, int pOffset, int pSize)
    {
        return this.CutString(Encoding.Default.GetString(this.ReadMem(this.DllImageAddress(Module) + pOffset, pSize)));
    }

    public string ReadStringUnicode(int pOffset, int pSize)
    {
        return this.CutString(Encoding.Unicode.GetString(this.ReadMem(pOffset, pSize)));
    }

    public string ReadStringUnicode(bool AddToImageAddress, int pOffset, int pSize)
    {
        return this.CutString(Encoding.Unicode.GetString(this.ReadMem(pOffset, pSize, AddToImageAddress)));
    }

    public string ReadStringUnicode(string Module, int pOffset, int pSize)
    {
        return this.CutString(Encoding.Unicode.GetString(this.ReadMem(this.DllImageAddress(Module) + pOffset, pSize)));
    }

    public uint ReadUInt(int pOffset)
    {
        return BitConverter.ToUInt32(this.ReadMem(pOffset, 4), 0);
    }

    public uint ReadUInt(bool AddToImageAddress, int pOffset)
    {
        return BitConverter.ToUInt32(this.ReadMem(pOffset, 4, AddToImageAddress), 0);
    }

    public uint ReadUInt(string Module, int pOffset)
    {
        return BitConverter.ToUInt32(this.ReadMem(this.DllImageAddress(Module) + pOffset, 4), 0);
    }

    public bool StartProcess()
    {
        if (this.ProcessName != "")
        {
            this.MyProcess = Process.GetProcessById(this.processID);
            if (this.MyProcess.Id == 0)
            {
              //  int num = (int)MessageBox.Show(this.ProcessName + " - не найден ID процесса. Возможно процесс не запущен или не доступен!", "Process Not Found ERROR PROC ID", MessageBoxButtons.OK, MessageBoxIcon.Hand);
                return false;
            }
            this.processHandle = this.OpenProcess(this.MyProcess, ProcessAccessFlags.CreateThread | ProcessAccessFlags.VirtualMemoryRead | ProcessAccessFlags.VirtualMemoryWrite | ProcessAccessFlags.VirtualMemoryOperation);
            if (this.processHandle != IntPtr.Zero)
                return true;
           // int num1 = (int)MessageBox.Show(this.ProcessName + " - нет прав или антивирус блокирует доступ к процессу.", "Process Not Found ERROR ACCESS PROC", MessageBoxButtons.OK, MessageBoxIcon.Hand);
            return false;
        }
      //  int num2 = (int)MessageBox.Show("Define process name first!");
        return false;
    }

    [DllImport("kernel32.dll")]
    public static extern bool VirtualProtectEx(IntPtr hProcess, int lpAddress, int dwSize, uint flNewProtect, out uint lpflOldProtect);


    public uint ProtectOffset(int pOffset, int size, uint flNewProtect)
    {
        uint oldprot = 0;
        VirtualProtectEx(processHandle, pOffset, size, flNewProtect, out oldprot);
        return oldprot;
    }

    public void WriteByte(int pOffset, byte pBytes)
    {
        this.WriteMem(pOffset, new byte[1]
    {
      pBytes
    });
    }

    public void WriteByte(bool AddToImageAddress, int pOffset, byte pBytes)
    {
        this.WriteMem(pOffset, new byte[1]
    {
      pBytes
    }, (AddToImageAddress ? 1 : 0) != 0);
    }

    public void WriteByte(string Module, int pOffset, byte pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, new byte[1]
    {
      pBytes
    });
    }

    public void WriteDouble(int pOffset, double pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteDouble(bool AddToImageAddress, int pOffset, double pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes), AddToImageAddress);
    }

    public void WriteDouble(string Module, int pOffset, double pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteFloat(int pOffset, float pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteFloat(bool AddToImageAddress, int pOffset, float pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes), AddToImageAddress);
    }

    public void WriteFloat(string Module, int pOffset, float pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteInt(int pOffset, int pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteInt(bool AddToImageAddress, int pOffset, int pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes), AddToImageAddress);
    }

    public void WriteInt(string Module, int pOffset, int pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteMem(int pOffset, byte[] pBytes)
    {
        ProcessMemory.WriteProcessMemory(this.processHandle, pOffset, pBytes, pBytes.Length, 0);
    }

    public void WriteMem(int pOffset, byte[] pBytes, bool AddToImageAddress)
    {
        ProcessMemory.WriteProcessMemory(this.processHandle, AddToImageAddress ? this.ImageAddress(pOffset) : pOffset, pBytes, pBytes.Length, 0);
    }

    [DllImport("kernel32.dll")]
    public static extern bool WriteProcessMemory(IntPtr hProcess, int lpBaseAddress, byte[] buffer, int size, int lpNumberOfBytesWritten);

    public void WriteShort(int pOffset, short pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteShort(bool AddToImageAddress, int pOffset, short pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes), AddToImageAddress);
    }

    public void WriteShort(string Module, int pOffset, short pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteStringAscii(int pOffset, string pBytes)
    {
        this.WriteMem(pOffset, Encoding.ASCII.GetBytes(pBytes + "\0"));
    }

    public void WriteStringAscii(bool AddToImageAddress, int pOffset, string pBytes)
    {
        this.WriteMem(pOffset, Encoding.ASCII.GetBytes(pBytes + "\0"), AddToImageAddress);
    }

    public void WriteStringAscii(string Module, int pOffset, string pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, Encoding.ASCII.GetBytes(pBytes + "\0"));
    }

    public void WriteStringWarcraft(int pOffset, string pBytes)
    {
        this.WriteMem(pOffset, Encoding.Default.GetBytes(pBytes + "\0"));
    }

    public void WriteStringWarcraft(bool AddToImageAddress, int pOffset, string pBytes)
    {
        this.WriteMem(pOffset, Encoding.Default.GetBytes(pBytes + "\0"), AddToImageAddress);
    }

    public void WriteStringWarcraft(string Module, int pOffset, string pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, Encoding.Default.GetBytes(pBytes + "\0"));
    }

    public void WriteStringUnicode(int pOffset, string pBytes)
    {
        this.WriteMem(pOffset, Encoding.Unicode.GetBytes(pBytes + "\0"));
    }

    public void WriteStringUnicode(bool AddToImageAddress, int pOffset, string pBytes)
    {
        this.WriteMem(pOffset, Encoding.Unicode.GetBytes(pBytes + "\0"), AddToImageAddress);
    }

    public void WriteStringUnicode(string Module, int pOffset, string pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, Encoding.Unicode.GetBytes(pBytes + "\0"));
    }

    public void WriteUInt(int pOffset, uint pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes));
    }

    public void WriteUInt(bool AddToImageAddress, int pOffset, uint pBytes)
    {
        this.WriteMem(pOffset, BitConverter.GetBytes(pBytes), AddToImageAddress);
    }

    public void WriteUInt(string Module, int pOffset, uint pBytes)
    {
        this.WriteMem(this.DllImageAddress(Module) + pOffset, BitConverter.GetBytes(pBytes));
    }


}
