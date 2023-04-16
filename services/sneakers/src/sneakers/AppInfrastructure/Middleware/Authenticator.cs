using System.Security.Authentication;
using sneakers.Models;
using sneakers.Storage;

namespace sneakers.AppInfrastructure.Middleware;

public class Authenticator : IAuthenticator
{
    private readonly IArtistsStorage artistsStorage;

    public Authenticator(IArtistsStorage artistsStorage)
    {
        this.artistsStorage = artistsStorage;
    }

    public Artist? Authenticate(string? artistTokenString)
    {
        if (artistTokenString == null)
            return null;

        if (!Guid.TryParse(artistTokenString, out var token))
            throw new AuthenticationException("A valid API token was not specified with the request.");

        if (!artistsStorage.ArtistExists(new ArtistToken(token), out var artist))
            throw new AuthenticationException("Artist for provided Artist-Token was not found.");

        return artist;
    }
}