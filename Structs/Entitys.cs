using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.Structs
{
    public class Entitys
    {
        public struct FrameRate 
        { 
            public float fps; 
        }

        public struct Camera
        {
            public float X;
            public float Y;
            public float Z;
            public float Yaw;
            public float Pitch;
        }

        public struct Header
        {
            public float ConstCaptureFps;
            public string MapName;
            public int TotalFrames;
            
        }
    }

}
