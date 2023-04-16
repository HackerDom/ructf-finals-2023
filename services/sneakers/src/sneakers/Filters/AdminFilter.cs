using sneakers.Models;

namespace sneakers.Filters;

public record class AdminFilter : IFilter
{
    public required ArtistToken OwnerToken { get; init; }

    public bool IsSuitable(Sneakers sneakers)
    {
        return sneakers.Collection == Collection.Pixel || OwnerToken.Equals(sneakers.Owner.Token);
    }
}