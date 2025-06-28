using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.config
{
    class Offsets
    {
        public static IntPtr FpsCounterAddress { get; } = (IntPtr) 0xC6EE228;
    }
}
