using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.config
{
    class Offsets
    {
        public static IntPtr FpsCounterAddress { get; } =  0xC6EE228; //iw3mp.exe+C6EE228

        public static IntPtr Cam_X { get; } = 0x39B6A4;

        public static IntPtr Cam_Y { get; } = 0x3973F8;

        /// <summary>
        /// Cam_Z
        /// iw3mp.exe+85B6F4
        ///iw3mp.exe+D5D108
        ///iw3mp.exe+D5D1B8
        /// </summary>
        public static IntPtr Cam_Z { get; } = 0x85B6F4;

        public static IntPtr Cam_Yaw { get; } = 0x39B69C;

        public static IntPtr Cam_Pitch { get; } = 0x39B698;

        public static IntPtr debug_show_viewpos { get; } = 0xCC874E0; //iw3mp.exe+CC874E0
        //iw3mp.exe+C7C5E88
        //iw3mp.exe+C7C5E98

        //
        //public static IntPtr debug_show_viewpos { get; } = 0xC7C5E98;

    }
}
