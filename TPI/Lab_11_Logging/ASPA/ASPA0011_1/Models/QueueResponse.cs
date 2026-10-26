using System.Text.Json.Serialization;

namespace ASPA0011_1.Models
{
    public class QueueResponse
    {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("data")]
        public string? Data { get; set; }
    }
}