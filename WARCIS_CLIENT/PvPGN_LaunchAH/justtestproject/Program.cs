using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace justtestproject
{
    class Program
    {
        static void Main(string[] args)
        {
            for (int i = 0; i < short.MaxValue; i++)
            {
                File.AppendAllText("deffile.txt", "\nA00" + i.ToString());
                File.AppendAllText("funcfile.txt", "A00" + i.ToString() + "(){}\n");

            }
        }
    }
}
