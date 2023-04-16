namespace sneakers.Models;

public record ArtistToken(Guid Value)
{
    public static ArtistToken New() => new ArtistToken(Guid.NewGuid());
};