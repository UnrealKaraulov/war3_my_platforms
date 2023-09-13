using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;


namespace FastDiffGenerateForCode
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Enter old file path:");
            string File1 = Console.ReadLine().Replace("\"","");

            Console.WriteLine("Enter new file path:");
            string File2 = Console.ReadLine().Replace("\"", "");

            Console.WriteLine("Enter out file path:");
            string FileOut = Console.ReadLine().Replace("\"", "");

            try
            {
                Console.WriteLine("Read files...");
                byte[] File1Data = File.ReadAllBytes(File1);
                byte[] File2Data = File.ReadAllBytes(File2);
                List<string> FileOutData = new List<string>();

                int File1DataOffset = 0;
                int File2DataOffset = 0;
                Console.WriteLine("Read diff bytes...");
                while (File1DataOffset < File1Data.Length && File2DataOffset < File2Data.Length)
                {
                    if (File1Data[File1DataOffset] != File2Data[File2DataOffset])
                    {
                        FileOutData.Add("data[" + File1DataOffset + "] = 0x" + File2Data[File2DataOffset].ToString("x2") + ";");
                    }

                    File1DataOffset++;
                    File2DataOffset++;
                }
                Console.WriteLine("Read diff size...");
                while (File2DataOffset < File2Data.Length)
                {
                    FileOutData.Add("data.Add(0x" + File2Data[File2DataOffset].ToString("x2") + ");");

                    File2DataOffset++;
                }
                Console.WriteLine("Write diff bytes...");
                File.WriteAllLines(FileOut, FileOutData.ToArray());
                Console.WriteLine("End..");
            }
            catch
            {
                Console.WriteLine("Error.");

            }


            Console.ReadKey();
        }
    }
}
