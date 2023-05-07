using StormLibSharp;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

namespace TestConsole
{
    class Program
    {

        static void Main(string[] args)
        {
            Console.WriteLine("Attach a native debugger now and press <enter> to continue.");
            Console.ReadLine();

         
            
            string listFile = null;
            using (MpqArchive archive = new MpqArchive(@"d:\Projects\base-Win.MPQ", FileAccess.Read))
            {
                using (MpqFileStream file = archive.OpenFile("(listfile)"))
                using (StreamReader sr = new StreamReader(file))
                {
                    listFile = sr.ReadToEnd();
                    Console.WriteLine(listFile);
                }

                archive.ExtractFile("(listfile)", @"d:\projects\base-win-listfile.txt");
            }

            using (MpqArchive archive = MpqArchive.CreateNew(@"d:\projects\mynewmpq.mpq", MpqArchiveVersion.Version4))
            {
                archive.AddFileFromDisk(@"D:\projects\base-win-listfile.txt", "base-win-listfile.txt");

                int retval = archive.AddListFile(@"base-win-listfile.txt");
                archive.Compact("base-win-listfile.txt");
                archive.Flush();
            } 

            Console.WriteLine("<enter> to exit.");
            Console.ReadLine();
        }
    }

    internal static class StreamExtensions
    {
        public static byte[] ReadAllBytes(this Stream fs)
        {
            byte[] result = new byte[fs.Length];
            fs.Position = 0;
            int cur = 0;
            while (cur < fs.Length)
            {
                int read = fs.Read(result, cur, result.Length - cur);
                cur += read;
            }

            return result;
        }
    }
}
