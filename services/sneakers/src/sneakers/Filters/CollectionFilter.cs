using sneakers.Models;

namespace sneakers.Filters;

public class CollectionFilter : IFilter
{
    public required Collection Collection { get; init; }

    public bool IsSuitable(Sneakers sneakers)
    {
        return Collection == sneakers.Collection;
    }
}