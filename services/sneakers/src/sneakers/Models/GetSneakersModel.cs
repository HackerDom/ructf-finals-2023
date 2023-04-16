namespace sneakers.Models;

public record GetSneakersModel(int Count, IEnumerable<SneakersModel> Elements);