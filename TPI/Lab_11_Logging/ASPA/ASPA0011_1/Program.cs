using ASPA0011_1.Services;
using ASPA0011_1.Logging;

var builder = WebApplication.CreateBuilder(args);

builder.Logging.ClearProviders();
builder.Logging.AddConsole();
builder.Logging.SetMinimumLevel(LogLevel.Trace);

bool isDev = builder.Environment.IsDevelopment();
builder.Logging.AddProvider(new FileLoggerProvider("logs.txt", isDev));

builder.Services.AddControllers();
builder.Services.AddSingleton<ChannelManager>();

var app = builder.Build();

var logger = app.Services.GetRequiredService<ILogger<Program>>();
logger.LogInformation("Старт приложения.");

app.UseAuthorization();
app.MapControllers();

app.Lifetime.ApplicationStopped.Register(() =>
{
    logger.LogInformation("Остановка приложения.");
});

app.Run();