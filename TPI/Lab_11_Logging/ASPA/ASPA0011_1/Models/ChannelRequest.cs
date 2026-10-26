using System.Text.Json.Serialization;

namespace ASPA0011_1.Models
{
    public class ChannelRequest
    {
        [JsonPropertyName("command")]
        public CommandChannels Command { get; set; }

        [JsonPropertyName("id")]
        public Guid? Id { get; set; }

        [JsonPropertyName("reason")]
        public string? Reason { get; set; }

        [JsonPropertyName("name")]
        public string? Name { get; set; }

        [JsonPropertyName("state")]
        public State? State { get; set; }

        [JsonPropertyName("description")]
        public string? Description { get; set; }
    }
}