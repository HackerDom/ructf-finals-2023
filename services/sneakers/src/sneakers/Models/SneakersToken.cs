namespace sneakers.Models;

public record SneakersToken(Guid Value)
{
    public static SneakersToken New() => new SneakersToken(Guid.NewGuid());
};