using System.Text.Json;
using System.Text.Json.Serialization;
using sneakers.Models;

namespace sneakers.Filters;

public class FiltersProvider : IFiltersProvider
{
    private static JsonSerializerOptions options = new JsonSerializerOptions()
    {
        Converters = { new JsonStringEnumConverter() }
    };

    public bool TryCreateFilter(FilterType type, string jsonString, out IFilter filter)
    {
        filter = null;

        var returnType = type switch
        {
            FilterType.Collection => typeof(CollectionFilter),
            FilterType.Color => typeof(ColorFilter),
            _ => typeof(AdminFilter)
        };

        try
        {
            var deserialized = JsonSerializer.Deserialize(jsonString, returnType,options);
            if (deserialized is not IFilter result)
                return false;

            filter = result;
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }
}