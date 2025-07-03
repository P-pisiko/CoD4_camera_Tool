using CoD4_dm1.config;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using static CoD4_dm1.Structs.Entitys;

namespace CoD4_dm1
{
    public class Record
    {
        private readonly Memory _memory;
        private readonly IntPtr _baseaddress;
        private readonly IntPtr _processHandle;
        private readonly List<Structs.Entitys.FrameRate> _fpsList;
        private readonly List<Structs.Entitys.Camera> _camFramesList;

        public Record(IntPtr baseAddress, IntPtr processHandle) 
        {
            _memory = new Memory();
            _baseaddress = baseAddress;
            _processHandle = processHandle;
            _fpsList = new List<Structs.Entitys.FrameRate>();
            _camFramesList = new List<Structs.Entitys.Camera>();
        }

        public List<Structs.Entitys.Camera> StartRecording()
        {
            byte debug_show_viewpos;
            do
            {
                debug_show_viewpos = _memory.ReadMemory<byte>(_processHandle, _baseaddress + Offsets.debug_show_viewpos);
                Thread.Sleep(250);

            } while (debug_show_viewpos == 0);

            while (true)
            {
                if(debug_show_viewpos == 0)
                {
                    /*Console.WriteLine($"{"Frm",3} | {"X",8} {"Y",8} {"Z",8} │  {"Yaw",8} {"Pitch",8}");
                    for (int i = 0; i < _camFramesList.Count; i++) 
                    {
                        var camFrame = _camFramesList[i];
                        Console.WriteLine($"{i,4} | {camFrame.X,8:F3} {camFrame.Y,8:F3} {camFrame.Z,8:F3} │ {camFrame.Yaw,8:F2} {camFrame.Pitch,8:F2}");
                        
                    }*/
                    break;
                }
                debug_show_viewpos = _memory.ReadMemory<byte>(_processHandle, _baseaddress + Offsets.debug_show_viewpos );
                var CamFrame = ReadCamFrame();
                _camFramesList.Add(CamFrame);

                var framerate = _memory.ReadMemory<float>(_processHandle, _baseaddress + 0xC6EE228);
                
                Thread.Sleep(1000 / (int)framerate);
            }
            return _camFramesList;

        }

        public Structs.Entitys.Camera ReadCamFrame()
        {
            var CamFrame = new Structs.Entitys.Camera
            {
                X = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.Cam_X),
                Y = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.Cam_Y),
                Z = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.Cam_Z),
                Yaw = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.Cam_Yaw),
                Pitch = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.Cam_Pitch)
            };

            return CamFrame;

           
        }

        public void DebugRecord()
        {
            Stopwatch stopwatch = Stopwatch.StartNew();//temp


            stopwatch.Start();//tempo
            while (true)
            {
                if (stopwatch.ElapsedMilliseconds > 300)
                {
                    Console.WriteLine(_camFramesList.Count.ToString());
                    Console.WriteLine(_camFramesList[0].X + " " + _camFramesList[0].Y +" "+ _camFramesList[0].Z);
                    break;
                }
                var CamFrame = ReadCamFrame();
                _camFramesList.Add(CamFrame);
                var framerate = _memory.ReadMemory<float>(_processHandle, _baseaddress + 0xC6EE228);
                Thread.Sleep(1000 / (int)framerate);

            }

        }
    }

}
