using CoD4_dm1.config;
using System.Diagnostics;
using System.Reflection.Metadata;
using System.Runtime.InteropServices;

namespace CoD4_dm1
{
    internal class Entry
    {
        static void Main(string[] args)
        {
       
                Process process = null;

                while (process == null)
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Process[] target = GetGameProcess();

                    if (target.Length > 0) {
                        process = target[0];
                        Console.ForegroundColor = ConsoleColor.Gray;
                    }
                    else
                    {
                        Console.WriteLine("Waiting for iw3mp.exe..");
                        Thread.Sleep(1000);
                    }
                }
                

                Memory mem = new Memory();
                Process targetProcess = process;
                IntPtr baseAddress = targetProcess.MainModule.BaseAddress;

                Console.WriteLine($"Target Process: {targetProcess.ProcessName} (PID: {targetProcess.Id})");

                // Open the process with read access
                IntPtr processHandle = Memory.OpenProcess(Memory.PROCESS_VM_READ | Memory.PROCESS_QUERY_INFORMATION,false,targetProcess.Id);

                if (processHandle == IntPtr.Zero)
                {
                    Console.WriteLine("Failed to open process. Error: " + Marshal.GetLastWin32Error());
                    return;
                }

                int runtime = 0;
                List<Structs.Entitys.FrameRate> Framerate = new List<Structs.Entitys.FrameRate>();
                List<Structs.Entitys.Camera> CameraFrames = new List<Structs.Entitys.Camera>();
            Console.WriteLine("Starting capture in 3");
            Thread.Sleep(3000);
                while (runtime < 500)
                {
                    byte[] buffer = mem.ReadBytes(processHandle, baseAddress + Offsets.FpsCounterAddress, 4);

                    if (buffer.Length > 0)
                    {
                    
                    //Console.WriteLine("Current value "+BitConverter.ToSingle(buffer, 0) + " fps");
                    
                    

                    Structs.Entitys.FrameRate FpsStruct = new Structs.Entitys.FrameRate { fps = mem.ReadMemory<float>(processHandle, baseAddress + Offsets.FpsCounterAddress) };
                    Structs.Entitys.Camera CamFrame = new Structs.Entitys.Camera();
                    
                    CamFrame.X = mem.ReadMemory<float>(processHandle, baseAddress + Offsets.Cam_X);
                    CamFrame.Y = mem.ReadMemory<float>(processHandle, baseAddress + Offsets.Cam_Y);
                    CamFrame.Z = mem.ReadMemory<float>(processHandle, baseAddress + Offsets.Cam_Z);
                    CamFrame.Yaw = mem.ReadMemory<float>(processHandle, baseAddress + Offsets.Cam_Yaw);
                    CamFrame.Pitch = mem.ReadMemory<float>(processHandle, baseAddress + Offsets.Cam_Pitch);
                    CameraFrames.Add(CamFrame);
                    runtime++;
                    Thread.Sleep(1000 / (int)FpsStruct.fps);

                        
                    }
                    else
                    {
                        Console.WriteLine("Failed to read memory. Error: " + Marshal.GetLastWin32Error());
                        break;
                    }
                }
            for (int i = 0; i < CameraFrames.Count; i++)
            {
                var frame = CameraFrames[i];
                Console.WriteLine($"{i,3} | {frame.X,8:F3} {frame.Y,8:F3} {frame.Z,8:F3} | {frame.Yaw,8:F3} {frame.Pitch,8:F3} ");
            }

            // Always close the handle when done
            Memory.CloseHandle(processHandle);
            
            
        }

        public static Process[] GetGameProcess()
        {
            return Process.GetProcessesByName("iw3mp");
        }
    }
}
