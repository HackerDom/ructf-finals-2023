namespace sneakers.Models;

public record ArtistId(Guid Value)
{
    public static ArtistId New() => new ArtistId(Guid.NewGuid());
};