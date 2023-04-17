using sneakers.Models;

namespace sneakers.Filters;

public interface IFilter
{
    bool IsSuitable(Sneakers sneakers);
}