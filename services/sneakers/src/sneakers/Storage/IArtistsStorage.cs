using sneakers.Models;

namespace sneakers.Storage
{
    public interface IArtistsStorage : IManagedStorage<Artist>
    {
        void Add(Artist artist);

        bool ArtistExists(ArtistToken artistToken, out Artist? artist);
    }
}