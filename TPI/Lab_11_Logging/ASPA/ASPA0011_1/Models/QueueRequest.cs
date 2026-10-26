using System.Text.Json.Serialization;

namespace ASPA0011_1.Models
{
    public class QueueRequest
    {
        [JsonPropertyName("command")]
        public CommandQueue Command { get; set; }

        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("data")]
        public string? Data { get; set; }
    }
}