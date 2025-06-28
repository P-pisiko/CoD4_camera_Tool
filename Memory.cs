using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1
{
    public class Memory
    {
        #region Imports
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenProcess(uint processAccess, bool bInheritHandle, int processId);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, int dwSize, out IntPtr lpNumberOfBytesRead);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);

        #endregion

        // access rights constants
        public const uint PROCESS_VM_READ = 0x0010;
        public const uint PROCESS_QUERY_INFORMATION = 0x0400;

        // Helper method to read a specific data type from memory
        public  T ReadMemory<T>(IntPtr processHandle, IntPtr address) where T : struct
        {
            int size = Marshal.SizeOf<T>();
            byte[] buffer = new byte[size];
            IntPtr bytesRead;

            if (ReadProcessMemory(processHandle, address, buffer, size, out bytesRead))
            {
                GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                try
                {
                    return Marshal.PtrToStructure<T>(handle.AddrOfPinnedObject());
                }
                finally
                {
                    handle.Free();
                }
            }

            throw new InvalidOperationException("Failed to read memory");
        }

        // Helper method to read a string from memory
        public  string ReadString(IntPtr processHandle, IntPtr address, int maxLength = 256)
        {
            byte[] buffer = new byte[maxLength];
            IntPtr bytesRead;

            if (ReadProcessMemory(processHandle, address, buffer, maxLength, out bytesRead))
            {
                // Find null terminator
                int nullIndex = Array.IndexOf(buffer, (byte)0);
                if (nullIndex >= 0)
                {
                    Array.Resize(ref buffer, nullIndex);
                }

                return Encoding.UTF8.GetString(buffer);
            }

            return string.Empty;
        }

        // Helper method to read raw bytes from memory
        public  byte[] ReadBytes(IntPtr processHandle, IntPtr address, int size)
        {
            byte[] buffer = new byte[size];
            IntPtr bytesRead;

            if (ReadProcessMemory(processHandle, address, buffer, size, out bytesRead))
            {
                // Resize buffer to actual bytes read
                if ((int)bytesRead < size)
                {
                    Array.Resize(ref buffer, (int)bytesRead);
                }
                return buffer;
            }

            return new byte[0];
        }
    }
}
