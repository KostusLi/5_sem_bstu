using Microsoft.Extensions.Logging;
using System.Diagnostics;

namespace ASPA0011_1.Logging
{
    public class FileLoggerProvider : ILoggerProvider
    {
        private readonly string _filePath;
        private readonly bool _isDevelopment;
        public static int _logCounter = 0;

        public FileLoggerProvider(string filePath, bool isDevelopment)
        {
            _filePath = filePath;
            _isDevelopment = isDevelopment;
            if (!File.Exists(_filePath)) File.WriteAllText(_filePath, "~~~~AVE MARIA!~~~~\n");
        }

        public ILogger CreateLogger(string categoryName) => new FileLogger(categoryName, _filePath, _isDevelopment);

        public void Dispose() { }
    }

    public class FileLogger : ILogger
    {
        private readonly string _category;
        private readonly string _filePath;
        private readonly bool _isDevelopment;
        private static readonly object _lock = new object();

        public FileLogger(string category, string filePath, bool isDevelopment)
        {
            _category = category;
            _filePath = filePath;
            _isDevelopment = isDevelopment;
        }

        public IDisposable? BeginScope<TState>(TState state) where TState : notnull => null;

        public bool IsEnabled(LogLevel logLevel) => true;

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception? exception, Func<TState, Exception?, string> formatter)
        {
            if (!IsEnabled(logLevel)) return;

            if ((logLevel == LogLevel.Trace || logLevel == LogLevel.Debug) && !_isDevelopment) return;

            if (_category.StartsWith("Microsoft") && (logLevel == LogLevel.Error || logLevel == LogLevel.Critical)) return;

            var msg = formatter(state, exception);
            var num = Interlocked.Increment(ref FileLoggerProvider._logCounter);
            var logLine = $"[{num}] {DateTime.Now:dd.MM.yyyy HH:mm:ss} | {logLevel} | {msg}\n";

            lock (_lock)
            {
                File.AppendAllText(_filePath, logLine);
            }

            if (logLevel >= LogLevel.Warning)
            {
                Console.WriteLine(logLine.Trim());
            }

            if (logLevel == LogLevel.Debug)
            {
                Debug.WriteLine(logLine.Trim());
            }
        }
    }
}