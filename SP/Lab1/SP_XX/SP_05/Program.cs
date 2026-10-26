using System;
using System.Runtime.InteropServices;

namespace SP_05
{
    class Program
    {
        const string DllName = "SP_XX.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int sum(int x, int y);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int sub(int x, int y);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern int mul(int x, int y);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, EntryPoint = "my_div")]
        public static extern int my_div(int x, int y);

        static void Main(string[] args)
        {
            Console.WriteLine("--- Задание 05: SP_05 (C# Static Load via DllImport) ---");

            try
            {
                Console.WriteLine($"sum(30, 6) = {sum(30, 6)}");
                Console.WriteLine($"sub(30, 6) = {sub(30, 6)}");
                Console.WriteLine($"mul(30, 6) = {mul(30, 6)}");
                Console.WriteLine($"my_div(30, 6) = {my_div(30, 6)}");
            }
            catch (DllNotFoundException)
            {
                Console.WriteLine("Ошибка: SP_XX.dll не найдена! Скопируй её в папку с .exe файлом SP_05.");
            }

            Console.ReadLine();
        }
    }
}