// Type: SharpMagic.Patternscanner
// Assembly: SharpMagic, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null
// MVID: E23B772C-417F-419D-85FF-9CD44497601D
// Assembly location: D:\projects\ReadWriteMemoryLibrary\SharpMagic.dll

using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace SharpMagic
{
  public class Patternscanner
  {
    private byte[] m_vDumpedRegion;
    private Process m_vProcess;
    private IntPtr m_vAddress;
    private int m_vSize;

    public Process Process
    {
      get
      {
        return m_vProcess;
      }
      set
      {
        m_vProcess = value;
      }
    }

    public IntPtr Address
    {
      get
      {
        return m_vAddress;
      }
      set
      {
        m_vAddress = value;
      }
    }

    public int Size
    {
      get
      {
        return m_vSize;
      }
      set
      {
        m_vSize = value;
      }
    }
    /// <summary>
    /// Сканирует память
    /// </summary>
    public Patternscanner()
    {
      m_vProcess = (Process) null;
      m_vAddress = IntPtr.Zero;
      m_vSize = 0;
      m_vDumpedRegion = (byte[]) null;
    }
    /// <summary>
    /// Сканирует память
    /// </summary>
    /// <param name="proc"> Процесс </param>
    /// <param name="addr"> Адрес </param>
    /// <param name="size"> Размер </param>
    public Patternscanner(Process proc, IntPtr addr, int size)
    {
      m_vProcess = proc;
      m_vAddress = addr;
      m_vSize = size;
    }

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, [Out] byte[] lpBuffer, int dwSize, out int lpNumberOfBytesRead);

    /// <summary>
    ///  Создает дамп памяти
    /// </summary>
    private bool DumpMemory()
    {
      try
      {
        if (m_vProcess == null || m_vProcess.HasExited || (m_vAddress == IntPtr.Zero || m_vSize == 0))
          return false;
        m_vDumpedRegion = new byte[m_vSize];
        int lpNumberOfBytesRead = 0;
        return Patternscanner.ReadProcessMemory(m_vProcess.Handle, m_vAddress, m_vDumpedRegion, m_vSize, out lpNumberOfBytesRead) && lpNumberOfBytesRead == m_vSize;
      }
      catch (Exception)
      {
        return false;
      }
    }
    /// <summary>
    /// Сравнить Смещение с маской
    /// </summary>
    /// <param name="btPattern"> Байты </param>
    /// <param name="nOffset"> Смещение </param>
    /// <param name="strMask"> Строка (маска)</param>
    private bool MaskCheck(int nOffset, byte[] btPattern, string strMask)
    {
      for (int index = 0; index < btPattern.Length; ++index)
      {
        if ((int) strMask[index] != 63 && ((int) strMask[index] == 120 && (int) btPattern[index] != (int) m_vDumpedRegion[nOffset + index]))
          return false;
      }
      return true;
    }
    /// <summary>
    /// Поиск совпадений
    /// </summary>
    /// <param name="btPattern"> Байты </param>
    /// <param name="nOffset"> Смещение </param>
    /// <param name="strMask"> Строка (маска)</param>
    public IntPtr FindPattern(byte[] btPattern, string strMask, int nOffset)
    {
      try
      {
        if ((m_vDumpedRegion == null || m_vDumpedRegion.Length == 0) && !DumpMemory() || strMask.Length != btPattern.Length)
          return IntPtr.Zero;
        for (int nOffset1 = 0; nOffset1 < m_vDumpedRegion.Length; ++nOffset1)
        {
          if (MaskCheck(nOffset1, btPattern, strMask))
            return new IntPtr((int) m_vAddress + (nOffset1 + nOffset));
        }
        return IntPtr.Zero;
      }
      catch (Exception)
      {
        return IntPtr.Zero;
      }
    }
    /// <summary>
    /// Удалить дамп памяти
    /// </summary>
    public void ResetRegion()
    {
      m_vDumpedRegion = (byte[]) null;
    }
  }
}
