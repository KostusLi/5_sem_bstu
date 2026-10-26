using System;
using System.Runtime.InteropServices;

namespace SP_07
{
    class Program
    {
        const string LibName = "./SP_XX.so";

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int sum(int x, int y);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int sub(int x, int y);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int mul(int x, int y);

        [DllImport(LibName, EntryPoint = "my_div", CallingConvention = CallingConvention.Cdecl)]
        public static extern int my_div(int x, int y);

        static void Main(string[] args)
        {
            Console.WriteLine("--- Задание 07: SP_07 (Linux C# Static Load) ---");
            try
            {
                Console.WriteLine($"sum(70, 10) = {sum(70, 10)}");
                Console.WriteLine($"sub(70, 10) = {sub(70, 10)}");
                Console.WriteLine($"mul(70, 10) = {mul(70, 10)}");
                Console.WriteLine($"my_div(70, 10) = {my_div(70, 10)}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка: {ex.Message}");
            }
        }
    }
}