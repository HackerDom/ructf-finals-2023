using sneakers.Models;

namespace sneakers.Filters;

public interface IFiltersProvider
{
    bool TryCreateFilter(FilterType type, string jsonString, out IFilter filter);
}