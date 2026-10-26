using System.Text.Json.Serialization;

namespace ASPA0011_1.Models
{
    [JsonConverter(typeof(JsonStringEnumConverter))]
    public enum CommandChannels
    {
        @new,
        close,
        open,
        del,
    }
}
