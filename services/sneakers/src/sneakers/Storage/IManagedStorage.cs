using sneakers.Models;

namespace sneakers.Storage
{
    public interface IManagedStorage<T> : IClearableStorage
    {
        void Initialize(IEnumerable<TimedValue<T>> initialArtists);
    }
}