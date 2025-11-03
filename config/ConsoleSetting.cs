using System;
using System.Runtime.InteropServices;


namespace CoD4_dm1.config
{
    public static class ConsoleSetting
    {
        const int STD_INPUT_HANDLE = -10;
        const uint ENABLE_QUICK_EDIT_MODE = 0x0040;
        const uint ENABLE_EXTENDED_FLAGS = 0x0080;

        [DllImport("kernel32.dll")]
        static extern IntPtr GetStdHandle(int nStdHandle);
        [DllImport("kernel32.dll")]
        static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);
        [DllImport("kernel32.dll")]
        static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);


        public static void SetQuickEdit()
        {
            var handle = GetStdHandle(STD_INPUT_HANDLE);
            if (!GetConsoleMode(handle, out var mode)) return;

            SetConsoleMode(handle, (mode & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_EXTENDED_FLAGS);
        }
    }
}
