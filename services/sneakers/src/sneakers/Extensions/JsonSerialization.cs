using System.Text.Json;

namespace sneakers.Extensions
{
    public static class JsonSerialization
    {
        public static string ToJson(this object? @object) => JsonSerializer.Serialize(@object);

        public static T? FromJson<T>(this string serialized) => JsonSerializer.Deserialize<T>(serialized);
    }
}