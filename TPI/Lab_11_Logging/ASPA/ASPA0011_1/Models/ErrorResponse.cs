using System.Text.Json.Serialization;

namespace ASPA0011_1.Models
{
    public class ErrorResponse
    {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("error")]
        public string? Error { get; set; }
    }
}