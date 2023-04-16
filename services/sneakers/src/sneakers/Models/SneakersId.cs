namespace sneakers.Models;

public record SneakersId(Guid Value)
{
    public static SneakersId New() => new SneakersId(Guid.NewGuid());
};