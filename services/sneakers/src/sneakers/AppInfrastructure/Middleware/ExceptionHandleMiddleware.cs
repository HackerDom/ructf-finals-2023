using System.ComponentModel.DataAnnotations;
using System.Security.Authentication;
using System.Text;
using Microsoft.AspNetCore.Connections;
using sneakers.Extensions;
using Vostok.Clusterclient.Core.Model;
using Vostok.Logging.Abstractions;

namespace sneakers.AppInfrastructure.Middleware;

public class ExceptionHandleMiddleware
{
    private readonly RequestDelegate next;
    private readonly ILog log;

    public ExceptionHandleMiddleware(RequestDelegate next, ILog log)
    {
        this.next = next;
        this.log = log;
    }

    public async Task Invoke(HttpContext context)
    {
        try
        {
            await next(context);
        }
        catch (Exception error) when (
            error is TaskCanceledException or OperationCanceledException or ConnectionResetException)
        {
            log.Warn("Request has been canceled. This is likely due to connection close from client side.");
        }
        catch (Exception error)
        {
            var responseCode = GetResponseCode(error);

            const string logMessage =
                "An unhandled exception occurred during request processing. Response started = {ResponseHasStarted}.";

            if (responseCode == ResponseCode.InternalServerError)
                log.Error(error, logMessage, context.Response.HasStarted);
            else
                log.Warn(error, logMessage, context.Response.HasStarted);

            if (context.Response.HasStarted)
                throw;

            var response = Encoding.UTF8.GetBytes(new ErrorResult(GetResponseMessage(error)).ToJson());

            context.Response.Clear();
            context.Response.StatusCode = (int)responseCode;
            context.Response.ContentLength = response.Length;
            context.Response.ContentType = "application/json";

            await context.Response.Body.WriteAsync(response);
        }
    }

    public static string GetResponseMessage(Exception error)
    {
        var message = $"{error.GetType().Name}: {error.Message}";

#if DEBUG
            message += $"\n{error.StackTrace}";
#endif

        return message;
    }

    private static ResponseCode GetResponseCode(Exception exception)
        => exception switch
        {
            AuthenticationException => ResponseCode.Unauthorized,
            UnauthorizedAccessException => ResponseCode.Forbidden,
            ValidationException => ResponseCode.BadRequest,
            KeyNotFoundException => ResponseCode.NotFound,
            _ => ResponseCode.InternalServerError
        };
}