using sneakers.Models;

namespace sneakers.Storage;

public interface ISneakersStorage : IManagedStorage<Sneakers>
{
    void Add(Sneakers sneakers);
    bool TryGet(SneakersToken sneakersToken, out Sneakers sneakers);
    IEnumerable<Sneakers> GetSneakers(List<SneakersId>? idsToSearch);
}