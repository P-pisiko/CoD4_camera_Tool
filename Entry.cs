using System.Diagnostics;

namespace CoD4_dm1
{
    internal class Entry
    {
        static void Main(string[] args)
        {
            Console.WriteLine("PID\tProcess");
            foreach (var p in Process.GetProcesses().OrderBy(pr => pr.ProcessName, StringComparer.OrdinalIgnoreCase))
            {
                // Some system services throw if you touch certain properties—wrap in try/catch.
                try
                {
                    Console.WriteLine($"PID: {p.Id}\t NAME: {p.ProcessName}");
                }
                catch { /* swallow inaccessible processes */ }
            }
        }
    }
}
