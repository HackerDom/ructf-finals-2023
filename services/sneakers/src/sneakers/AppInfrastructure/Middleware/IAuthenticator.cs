using sneakers.Models;

namespace sneakers.AppInfrastructure.Middleware;

public interface IAuthenticator
{
    Artist? Authenticate(string? artistTokenString);
}