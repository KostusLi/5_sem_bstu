using System.Text.Json.Serialization;

namespace ASPA0011_1.Models
{
    public class ChannelInfo
    {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("name")]
        public string? Name { get; set; }

        [JsonPropertyName("state")]
        public State State { get; set; }

        [JsonPropertyName("description")]
        public string? Description { get; set; }
    }
}