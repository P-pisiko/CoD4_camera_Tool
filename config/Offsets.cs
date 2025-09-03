using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.config
{
    /// <summary>
    /// Appearintly when you lookup for an DVAR using cheat engine it looks like you find the "static" adress of that 
    /// variable but as soons as a new profile loaded all the "static" are gone, the underlying problems looks like the user config file
    /// Sinse the most of the settings are loaded from the config_mp.cfg file order of the DVAR's effects the DVAR table.
    /// </summary>
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

        public static IntPtr CurrentMap { get; } = 0xA36218; //iw3mp.exe+A36218 code_post_gfx_mp static 

        public static IntPtr CurrentMap2 { get; } = 0xBFF270; //iw3mp.exe+BFF270 static
        //
        //public static IntPtr debug_show_viewpos { get; } = 0xC7C5E98;

    }
}
