using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.config
{
    class Offsets
    {
        public static IntPtr FpsCounterAddress { get; } =  0xC6EE228;

        public static IntPtr Cam_X { get; } = 0x00797648;

        public static IntPtr Cam_Y { get; } = 0x0079764C;

        public static IntPtr Cam_Z_Player { get; } = 0x00797650;

        public static IntPtr Cam_Z { get; } = 0x85B6F4;
    }
}
