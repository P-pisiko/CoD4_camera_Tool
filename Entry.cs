using CoD4_dm1.config;
using CoD4_dm1.PipeServer;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace CoD4_dm1
{
    internal class Entry
    {
        [DllImport("loader.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void loaderMain(string filePath);
        static void Main(string[] args)
        {
            ConsoleSetting.SetQuickEdit();

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

            // Open the process with read access
            IntPtr processHandle = Memory.OpenProcess(Memory.PROCESS_VM_READ | Memory.PROCESS_QUERY_INFORMATION,false,targetProcess.Id);

            if (processHandle == IntPtr.Zero)
            {
                Console.WriteLine("Failed to open process. Error: " + Marshal.GetLastWin32Error());
                return;
            }
            Console.WriteLine($"Got a handle to {targetProcess.ProcessName} {targetProcess.Id}");
            
            Record rec = new Record(baseAddress,processHandle);
            NamedPipeServer pipeServer = new NamedPipeServer(rec);
            
            pipeServer.PipeServerStart();
            
            Memory.CloseHandle(processHandle);
            
            
        }

        
        /// <summary>
        /// Find out where game is located.
        /// </summary>
        /// <param name="processName"></param>
        /// <param name="Pids"></param>
        /// <returns></returns>
        static string GetProcessDirectory(string processName, out int[] Pids)
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
                        Pids = procs.Select(p => p.Id).ToArray();
                        Console.ForegroundColor = ConsoleColor.Gray;
                        Console.WriteLine($"\nFound root directory: {dir}");
                        return dir;
                    }
                    catch
                    {
                        // Access denied or process exited while accessing MainModule
                    }
                }

                Console.Write(".");
                Thread.Sleep(1000);
            }
        }




    }
}
