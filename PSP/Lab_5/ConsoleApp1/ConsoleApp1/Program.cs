internal class Program
{
    private static void Main(string[] args)
    {
        try
        {
            Console.WriteLine("Hello, World!");
        }
        catch (Exception ex)
        {

        }
        catch(ArgumentException ex1)
        {

        }
    }
}

public interface IJson
{
    public void GetJson();
}

public class XmlLibrary
{
    public void GetXml()
    {

    }
}


public class Adapter : IJson
{
    public XmlLibrary library;
    public Adapter(XmlLibrary library)
    {
        this.library = library;
    }

    public void GetJson()
    {
        library.GetXml();
        //fff

    }
}

