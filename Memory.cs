using System.Runtime.InteropServices;
using System.Text;

namespace CoD4_dm1
{
    /// <summary>
    /// I used AI to optimze the memory reads go ahead make fun of me.
    /// </summary>
    public class Memory
    {
        #region Imports
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenProcess(uint processAccess, bool bInheritHandle, int processId);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool ReadProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, IntPtr lpBuffer, int dwSize, out IntPtr lpNumberOfBytesRead);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);

        #endregion

        // access rights constants
        public const uint PROCESS_VM_READ = 0x0010;
        public const uint PROCESS_QUERY_INFORMATION = 0x0400;
        const int MaxStackBytes = 128;

        unsafe public T ReadMemory<T>(IntPtr processHandle, IntPtr address) where T : struct
        {
            // tweakable; larger structs need more
            var size = Marshal.SizeOf<T>();

            // Quick sanity check – we can't stack‑alloc too much on the stack
            if (size > MaxStackBytes)
                throw new ArgumentException($"Struct too large for stack allocation (>{MaxStackBytes} bytes).", nameof(size));

            // Allocate buffer on the stack
            byte* buf = stackalloc byte[size];

            if (!ReadProcessMemory(processHandle, address, new IntPtr(buf), size, out var bytesRead))
            {
                int err = Marshal.GetLastWin32Error();
                throw new InvalidOperationException($"ReadProcessMemory failed with error {err}");
            }

            return MemoryMarshal.Read<T>(new ReadOnlySpan<byte>(buf, size));
        }

        unsafe public string ReadString(IntPtr processHandle, IntPtr address, int maxLength = 64)
        {
            const int StackLimit = 128;
            if (maxLength > StackLimit)
                throw new ArgumentOutOfRangeException(nameof(maxLength), $"maxLength must be ≤ {StackLimit} bytes for stack allocation.");

            byte* buf = stackalloc byte[maxLength];
            if (!ReadProcessMemory(processHandle, address, new IntPtr(buf), maxLength, out IntPtr bytesRead))
            {
                return string.Empty;
            }

            // Find null terminator inside the read bytes
            Span<byte> span = new Span<byte>(buf, (int)bytesRead);
            int nullIndex = span.IndexOf((byte)0);
            int len = nullIndex >= 0 ? nullIndex : span.Length;

            return Encoding.UTF8.GetString(span.Slice(0, len));
        }

        /* read raw bytes from memory
        public  byte[] ReadBytes(IntPtr processHandle, IntPtr address, int size)
        {
            byte[] buffer = new byte[size];
            IntPtr bytesRead;

            if (ReadProcessMemory(processHandle, address, buffer, size, out bytesRead))
            {
                
                if ((int)bytesRead < size)
                {
                    Array.Resize(ref buffer, (int)bytesRead);
                }
                return buffer;
            }

            return [];
        }

        */
    }
}
