using System.Threading.Channels;
using ASPA0011_1.Models;

namespace ASPA0011_1.Services
{
    public class ManagedChannel
    {
        public Guid Id { get; }
        public string Name { get; set; }
        public string Description { get; set; }
        public State State { get; set; }
        public Channel<string> Channel { get; }

        public ManagedChannel(Guid id, string name, string description, State state = State.ACTIVE)
        {
            Id = id;
            Name = name;
            Description = description;
            State = state;
            Channel = System.Threading.Channels.Channel.CreateUnbounded<string>();
        }
    }
}