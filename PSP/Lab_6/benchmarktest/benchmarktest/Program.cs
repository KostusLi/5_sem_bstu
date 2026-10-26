using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Running;

namespace MyBenchmark
{
    internal class Program
    {
        static void Main(string[] args)
        {
            BenchmarkRunner.Run<ParserBenchmarks>();
        }
    }

    [MemoryDiagnoser]
    public class ParserBenchmarks
    {
        private const string input = "2023-10-24 14:32:01.000 user_logged_in 200";

        [Benchmark(Baseline = true)]
        public int SplitParse()
        {
            string[] parts = input.Split(' ');
            return int.Parse(parts[^1]);
        }

        [Benchmark]
        public int SpanParse()
        {
            ReadOnlySpan<char> span = input.AsSpan();
            int index = span.LastIndexOf(' ');
            return int.Parse(span[(index + 1)..]);
        }
    }
}
