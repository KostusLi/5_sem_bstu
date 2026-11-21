var actions = new List<Action>();

int x = 0;

var numbers = Enumerable.Range(1, 3)
    .Select(i =>
    {
        int value = i;

        actions.Add(() => Console.WriteLine($"A: {value}, {x}"));

        x++;

        return value;
    });

x = 100;

var result = numbers.ToList();

x = 200;

foreach (var action in actions)
{
    action();
}

Console.WriteLine(string.Join(", ", result));

/*
1, 200
2, 200
3, 200
*/























//class A
//{
//    public static int X = InitA();

//    static int InitA()
//    {
//        Console.WriteLine("A initialized");
//        return 1;
//    }
//}

//class B : A
//{
//    public static int Y = InitB();

//    static B()
//    {
//        Console.WriteLine("B .cctor");
//    }

//    static int InitB()
//    {
//        Console.WriteLine("B initialized");
//        return 2;
//    }
//}

//public class Program
//{
//    public static void Main()
//    {
//        Console.WriteLine(B.Y);
//    }
//}

/*
B initialized
B .cctor
2
*/




























//public class BaseClass
//{
//    public int baseField = GetBaseValue();
//    public static int baseStaticField = GetBaseStaticValue();

//    static BaseClass()
//    {
//        Console.WriteLine("Base static constructor");
//    }

//    public BaseClass()
//    {
//        Console.WriteLine("Base instance constructor");
//    }

//    private static int GetBaseStaticValue()
//    {
//        Console.WriteLine("Base static field initializer");
//        return 1;
//    }

//    private static int GetBaseValue()
//    {
//        Console.WriteLine("Base instance field initializer");
//        return 10;
//    }
//}

//public class DerivedClass : BaseClass
//{
//    public int derivedField = GetDerivedValue();
//    public static int derivedStaticField = GetDerivedStaticValue();

//    static DerivedClass()
//    {
//        Console.WriteLine("Derived static constructor");
//    }

//    public DerivedClass()
//    {
//        Console.WriteLine("Derived instance constructor");
//    }

//    private static int GetDerivedStaticValue()
//    {
//        Console.WriteLine("Derived static field initializer");
//        return 2;
//    }

//    private static int GetDerivedValue()
//    {
//        Console.WriteLine("Derived instance field initializer");
//        return 20;
//    }
//}

//public class Program
//{
//    public static void Main()
//    {
//        Console.WriteLine("=== First instance ===");
//        DerivedClass obj1 = new DerivedClass();
//        Console.WriteLine("\n=== Second instance ===");
//        DerivedClass obj2 = new DerivedClass();
//    }
//}

/*
"=== First instance ==="
"Base static field initializer"
"Base static constructor"
"Derived static field initializer"
"Derived static constructor"
"Derived instance field initializer"
"Base instance field initializer"
"Base instance constructor"
"Derived instance constructor"
"\n=== Second instance ==="
"Derived instance field initializer"
"Base instance field initializer"
"Base instance constructor"
"Derived instance constructor"
*/



//var actions = new List<Action>();

//foreach (var i in Enumerable.Range(0, 3))
//{
//    actions.Add(() => Console.WriteLine(i));
//}

//foreach (var action in actions)
//{
//    action();
//}








//using System;

//public class A
//{
//    public static int X = B.Y + 1;
//    static A() => Console.WriteLine("A .cctor");
//}

//public class B
//{
//    public static int Y = A.X + 1;
//    static B() => Console.WriteLine("B .cctor");
//}

//public class Program
//{
//    public static void Main()
//    {
//        Console.WriteLine("First access to A.X:");
//        Console.WriteLine($"A.X = {A.X}");
//        Console.WriteLine("\nFirst access to B.Y:");
//        Console.WriteLine($"B.Y = {B.Y}");
//        Console.WriteLine("\nSecond access:");
//        Console.WriteLine($"A.X = {A.X}, B.Y = {B.Y}");
//    }
//}

///*
//"First access to A.X:"
//"A .cctor"

//*/