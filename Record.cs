﻿using CoD4_dm1.config;
using System.Diagnostics;

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


        private Structs.Entitys.Camera ReadCamFrame()
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
        public Structs.Entitys.Header InitRecord()
        {
            _camFramesList.Clear();
            return new Structs.Entitys.Header
            {   //Peak var naming
                ConstCaptureFps = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.FpsCounterAddress),
                MapName = _memory.ReadString(_processHandle, _baseaddress + Offsets.CurrentMap),
                TotalFrames = -1
            };

        }
        public int AddNewFrameToList()
        {
            var CamFrame = ReadCamFrame();
            _camFramesList.Add(CamFrame);
            return _camFramesList.Count;
        }

        public void DebugRecord()
        {
            Stopwatch stopwatch = Stopwatch.StartNew();//temp

            int deadline = 8000;
            stopwatch.Start();//tempo
            while (true)
            {
                if (stopwatch.ElapsedMilliseconds > deadline)
                {
                    Console.WriteLine($"[ + ] Eclipsed Time:  {stopwatch.ElapsedMilliseconds / 1000}s");
                    Console.WriteLine($"[ + ] Total Recorded Frame Count:  {_camFramesList.Count()}");
                    Console.WriteLine($"[ + ] Avarage time accuracy:  {deadline / _camFramesList.Count()}ms ");
                    Console.WriteLine($"[ + ] Sample Frame from the list: {_camFramesList[30].X} { _camFramesList[30].Y} {_camFramesList[30].Z}");
                    break;
                }
                var CamFrame = ReadCamFrame();
                _camFramesList.Add(CamFrame);
                var framerate = _memory.ReadMemory<float>(_processHandle, _baseaddress + Offsets.FpsCounterAddress);
                Thread.Sleep(1000 / (int)framerate);

            }

        }

        public List<Structs.Entitys.Camera> GetCamFrameList()
        {
            return _camFramesList;
        }

        public List<Structs.Entitys.Camera> RecordOnDVAR()
        {
            Console.WriteLine("Waiting for the Dvar");
            byte debug_show_viewpos;
            do
            {
                debug_show_viewpos = _memory.ReadMemory<byte>(_processHandle, _baseaddress + Offsets.debug_show_viewpos);
                Thread.Sleep(250);

            } while (debug_show_viewpos == 0 && debug_show_viewpos != 1);

            while (true)
            {
                if (debug_show_viewpos == 0 && debug_show_viewpos != 1)
                {
                    Console.WriteLine($"{"Frm",3} | {"X",8} {"Y",8} {"Z",8} │  {"Yaw",8} {"Pitch",8}");
                    for (int i = 0; i < _camFramesList.Count; i++)
                    {
                        var camFrame = _camFramesList[i];
                        Console.WriteLine($"{i,4} | {camFrame.X,8:F3} {camFrame.Y,8:F3} {camFrame.Z,8:F3} │ {camFrame.Yaw,8:F2} {camFrame.Pitch,8:F2}");

                    }
                    break;
                }
                debug_show_viewpos = _memory.ReadMemory<byte>(_processHandle, _baseaddress + Offsets.debug_show_viewpos);
                var CamFrame = ReadCamFrame();
                _camFramesList.Add(CamFrame);

                var framerate = _memory.ReadMemory<float>(_processHandle, _baseaddress + 0xC6EE228);


            }
            return _camFramesList;

        }

        public void PrintFramesConsole()
        {
            Console.WriteLine($"{"Frm",3} | {"X",8} {"Y",8} {"Z",8} │  {"Yaw",8} {"Pitch",8}");
            for (int i = 0 ; i < _camFramesList.Count;  i++)
            {
                var camFrame = _camFramesList[i];
                Console.WriteLine($"{i,4} | {camFrame.X,8:F3} {camFrame.Y,8:F3} {camFrame.Z,8:F3} │ {camFrame.Yaw,8:F2} {camFrame.Pitch,8:F2}");
            }
        }
    }

}
