using System.Collections.Concurrent;
using System.Threading.Channels;
using ASPA0011_1.Models;

namespace ASPA0011_1.Services
{
    public class ChannelManager
    {
        public readonly ConcurrentDictionary<Guid, ManagedChannel> _channels = new();
        private readonly ILogger<ChannelManager> _logger;
        private readonly int _waitEnqueueSeconds;

        public ChannelManager(ILogger<ChannelManager> logger, IConfiguration config)
        {
            _logger = logger;
            _waitEnqueueSeconds = config.GetValue<int>("WaitEnqueue", 5);
        }

        public List<ChannelInfo> GetAllChannels() =>
            _channels.Values.Select(c => new ChannelInfo { Id = c.Id, Name = c.Name, State = c.State, Description = c.Description }).ToList();

        public ChannelInfo? GetChannel(Guid id) =>
            _channels.TryGetValue(id, out var c)
                ? new ChannelInfo { Id = c.Id, Name = c.Name, State = c.State, Description = c.Description }
                : null;

        public ChannelInfo CreateChannel(string? name, string? desc, State? state)
        {
            var id = Guid.NewGuid();
            var ch = new ManagedChannel(id, name ?? "New Channel", desc ?? "", state ?? State.ACTIVE);
            _channels.TryAdd(id, ch);
            _logger.LogInformation($"Создан канал {id}.");
            return new ChannelInfo { Id = ch.Id, Name = ch.Name, State = ch.State, Description = ch.Description };
        }

        public void ChangeState(Guid? id, State newState)
        {
            if (id.HasValue && _channels.TryGetValue(id.Value, out var ch))
            {
                if (ch.State == newState)
                    _logger.LogWarning($"Команда {newState} для канала {id.Value}, который уже в этом состоянии.");
                else
                    _logger.LogInformation($"Состояние канала {id.Value} изменено на {newState}.");

                ch.State = newState;
            }
            else if (!id.HasValue)
            {
                foreach (var c in _channels.Values) c.State = newState;
                _logger.LogInformation($"Состояние всех каналов изменено на {newState}.");
            }
        }

        public void DeleteChannels(State? state)
        {
            var toRemove = state.HasValue
                ? _channels.Values.Where(c => c.State == state.Value).Select(c => c.Id).ToList()
                : _channels.Keys.ToList();

            foreach (var id in toRemove)
            {
                _channels.TryRemove(id, out _);
                _logger.LogInformation($"Канал {id} удален.");
            }
        }

        public async Task<(string? Data, string? Error)> QueueOperationAsync(Guid id, CommandQueue cmd, string? data)
        {
            if (!_channels.TryGetValue(id, out var ch)) return (null, "Канал не найден");
            if (ch.State == State.CLOSED) return (null, "Канал закрыт (CLOSED)");

            switch (cmd)
            {
                case CommandQueue.enqueue:
                    if (data != null) ch.Channel.Writer.TryWrite(data);
                    return (null, null);

                case CommandQueue.dequeue:
                case CommandQueue.peek:
                    using (var cts = new CancellationTokenSource(TimeSpan.FromSeconds(_waitEnqueueSeconds)))
                    {
                        try
                        {
                            if (cmd == CommandQueue.dequeue)
                            {
                                var msg = await ch.Channel.Reader.ReadAsync(cts.Token);
                                return (msg, null);
                            }
                            else
                            {
                                await ch.Channel.Reader.WaitToReadAsync(cts.Token);
                                ch.Channel.Reader.TryPeek(out var peekMsg);
                                return (peekMsg, null);
                            }
                        }
                        catch (OperationCanceledException)
                        {
                            _logger.LogWarning($"Истечение WaitEnqueue ({_waitEnqueueSeconds} сек) для канала {id}.");
                            return (null, "Истекло время ожидания (WaitEnqueue).");
                        }
                    }
                default:
                    return (null, "Неизвестная команда");
            }
        }
    }
}