using System;
using System.Runtime.InteropServices;

namespace SP_06
{
    class Program
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        delegate int MathDelegate(int x, int y);

        static void Main(string[] args)
        {
            Console.WriteLine("--- Задание 06: SP_06 (C# Dynamic Load via NativeLibrary) ---");

            IntPtr libraryHandle = IntPtr.Zero;
            try
            {
                libraryHandle = NativeLibrary.Load("SP_XX.dll");
                Console.WriteLine("Библиотека успешно загружена!");

                IntPtr sumPtr = NativeLibrary.GetExport(libraryHandle, "sum");
                IntPtr subPtr = NativeLibrary.GetExport(libraryHandle, "sub");
                IntPtr mulPtr = NativeLibrary.GetExport(libraryHandle, "mul");
                IntPtr divPtr = NativeLibrary.GetExport(libraryHandle, "my_div");

                MathDelegate sumFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(sumPtr);
                MathDelegate subFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(subPtr);
                MathDelegate mulFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(mulPtr);
                MathDelegate divFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(divPtr);

                Console.WriteLine($"sum(40, 8) = {sumFunc(40, 8)}");
                Console.WriteLine($"sub(40, 8) = {subFunc(40, 8)}");
                Console.WriteLine($"mul(40, 8) = {mulFunc(40, 8)}");
                Console.WriteLine($"my_div(40, 8) = {divFunc(40, 8)}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Произошла ошибка: {ex.Message}");
                Console.WriteLine("Убедись, что SP_XX.dll лежит в папке с программой!");
            }
            finally
            {
                if (libraryHandle != IntPtr.Zero)
                {
                    NativeLibrary.Free(libraryHandle);
                    Console.WriteLine("Библиотека выгружена из памяти.");
                }
            }

            Console.ReadLine();
        }
    }
}