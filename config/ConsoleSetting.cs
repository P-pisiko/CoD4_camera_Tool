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
        public static void WriteError(string message)
        {
            WriteColoredBackground(message, ConsoleColor.White, ConsoleColor.DarkRed, Console.Error);
        }

        public static void WriteSuccess(string message)
        {
            WriteColored(message, ConsoleColor.Green);
        }

        public static void WriteWarning(string message)
        {
            WriteColoredBackground(message, ConsoleColor.Black, ConsoleColor.DarkYellow);
        }

        public static void WriteInfo(string message)
        {
            WriteColored(message, ConsoleColor.DarkYellow);
        }
        private static void WriteColored(string message, ConsoleColor color, TextWriter writer = null)
        {
            var original = Console.ForegroundColor;
            Console.ForegroundColor = color;
            (writer ?? Console.Out).WriteLine(message);
            Console.ForegroundColor = original;
        }
        private static void WriteColoredBackground(string message, ConsoleColor foreground, ConsoleColor background, TextWriter writer = null)
        {
            var originalFg = Console.ForegroundColor;
            var originalBg = Console.BackgroundColor;

            Console.ForegroundColor = foreground;
            Console.BackgroundColor = background;
            (writer ?? Console.Out).WriteLine(message);

            Console.ForegroundColor = originalFg;
            Console.BackgroundColor = originalBg;
        }
    }
}
