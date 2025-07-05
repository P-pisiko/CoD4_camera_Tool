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
                Console.ForegroundColor = ConsoleColor.Green;
                Console.Write("Waiting for iw3mp.exe");
                while (process == null)
                {
                    
                    Process[] target = GetGameProcess();

                    if (target.Length > 0) {
                        process = target[0];
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine();
                    }
                    else
                    {
                        Console.Write(".");
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

                Record rec = new Record(baseAddress,processHandle);

            
            //rec.DebugRecord();
            
            var List = rec.StartRecording();
            Console.WriteLine("[ + ]Finised Recording, writing to the file.");
            FileFormats.Csv csv = new FileFormats.Csv(List);
            
            // close the handle when done
            Memory.CloseHandle(processHandle);
            
            
        }

        public static Process[] GetGameProcess()
        {
            return Process.GetProcessesByName("iw3mp");
        }
    }
}
