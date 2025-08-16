using CoD4_dm1.config;
using System.Diagnostics;
using System.Reflection.Metadata;
using System.Runtime.InteropServices;

namespace CoD4_dm1
{
    internal class Entry
    {
        [DllImport("loader.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void loaderMain(string filePath);
        static void Main(string[] args)
        {
            const string processName = "iw3mp";
            int[] originalPids;
            string gameRootPath = GetProcessDirectory(processName, out originalPids);
            Process[] ps = Process.GetProcessesByName(processName);

            foreach (Process p in ps)
                p.Kill();
            loaderMain(gameRootPath);

            

                Process process = null;
                Console.ForegroundColor = ConsoleColor.Green;
                Console.Write("Waiting for iw3mp.exe");
                while (process == null)
                {
                    
                    Process[] target = Process.GetProcessesByName(processName);

                    if (target.Length > 0) 
                    {
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

            
            
            
            
            /*var List = rec.StartRecording();
            Console.WriteLine("[ + ]Finised Recording, writing to the file.");
            FileFormats.Csv csv = new FileFormats.Csv(List);
            */
            
            Memory.CloseHandle(processHandle);
            
            
        }

        

        static string GetProcessDirectory(string processName, out int[] observedPids)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.Write($"Waiting for {processName}.exe");

            while (true)
            {
                var procs = Process.GetProcessesByName(processName);
                if (procs.Length > 0)
                {
                    try
                    {
                        // Get directory of the first accessible instance and record all current PIDs
                        string dir = Path.GetDirectoryName(procs[0].MainModule.FileName);
                        observedPids = procs.Select(p => p.Id).ToArray();
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine(dir);
                        return dir;
                    }
                    catch
                    {
                        // Access denied or process exited while accessing MainModule — try again
                    }
                }

                Console.Write(".");
                Thread.Sleep(1000);
            }
        }




    }
}
