using System;
using System.Runtime.InteropServices;

namespace SP_08
{
    class Program
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        delegate int MathDelegate(int x, int y);

        static void Main(string[] args)
        {
            Console.WriteLine("--- Задание 08: SP_08 (Linux C# Dynamic Load) ---");
            
            IntPtr libraryHandle = IntPtr.Zero;
            try
            {
                libraryHandle = NativeLibrary.Load("./SP_XX.so");
                Console.WriteLine("Библиотека .so успешно загружена динамически!");

                IntPtr sumPtr = NativeLibrary.GetExport(libraryHandle, "sum");
                IntPtr subPtr = NativeLibrary.GetExport(libraryHandle, "sub");
                IntPtr mulPtr = NativeLibrary.GetExport(libraryHandle, "mul");
                IntPtr divPtr = NativeLibrary.GetExport(libraryHandle, "my_div");

                MathDelegate sumFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(sumPtr);
                MathDelegate subFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(subPtr);
                MathDelegate mulFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(mulPtr);
                MathDelegate divFunc = Marshal.GetDelegateForFunctionPointer<MathDelegate>(divPtr);

                Console.WriteLine($"sum(80, 20) = {sumFunc(80, 20)}");
                Console.WriteLine($"sub(80, 20) = {subFunc(80, 20)}");
                Console.WriteLine($"mul(80, 20) = {mulFunc(80, 20)}");
                Console.WriteLine($"my_div(80, 20) = {divFunc(80, 20)}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Ошибка: {ex.Message}");
            }
            finally
            {
                if (libraryHandle != IntPtr.Zero)
                {
                    NativeLibrary.Free(libraryHandle);
                }
            }
        }
    }
}