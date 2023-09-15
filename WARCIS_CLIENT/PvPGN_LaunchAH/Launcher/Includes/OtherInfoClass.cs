using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OtherInfo
{
    public class OtherInfoClass
    {
        public static void ReadOtherInfo(ref UInt32 var1, ref UInt32 var2, ref UInt32 var3, ref UInt32 var4)
        {
            ProcessHelper.InitProcessHelper();
            try
            {
                var1 = ProcessHelper.Value_int1();
                var2 = ProcessHelper.Value_int2();
                var3 = ProcessHelper.Value_int3();
                var4 = ProcessHelper.Value_int4();
            }
            catch
            {
                try
                {
                    var4 = Crc32.Compute(Encoding.UTF8.GetBytes(CpuID.getCpuID()));
                }
                catch
                {

                }
            }
        }
    }
}
