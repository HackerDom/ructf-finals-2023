namespace sneakers.AppInfrastructure.Middleware;

public class AuthenticationMiddleware
{
    private const string ApiTokenCookieName = "Artist-Token";

    private readonly IAuthenticator authenticator;
    private readonly RequestDelegate next;

    public AuthenticationMiddleware(IAuthenticator authenticator, RequestDelegate next)
    {
        this.authenticator = authenticator;
        this.next = next;
    }

    public async Task Invoke(HttpContext context)
    {
        var apiTokenString = Extract(context.Request);

        Context.Artist = authenticator.Authenticate(apiTokenString);

        await next(context);
    }

    private static string? Extract(HttpRequest request)
    {
        var apiTokenString = FindInAuthorizationHeader(request);
        if (!string.IsNullOrEmpty(apiTokenString))
            return apiTokenString;

        return request.Cookies.TryGetValue(ApiTokenCookieName, out apiTokenString)
            ? apiTokenString
            : null;
    }

    private static string? FindInAuthorizationHeader(HttpRequest request)
    {
        if (!request.Headers.TryGetValue("Authorization", out var authorizationHeader))
            return null;

        return authorizationHeader;
    }
}