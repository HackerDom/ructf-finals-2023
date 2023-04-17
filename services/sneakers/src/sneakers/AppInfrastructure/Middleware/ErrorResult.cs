namespace sneakers.AppInfrastructure.Middleware;

public class ErrorResult
{
    public static readonly ErrorResult Empty = new ErrorResult();

    public ErrorResult(string? message = null)
    {
        Message = message;
    }

    public string? Message { get; }

    public static implicit operator ErrorResult(string input)
    {
        return new ErrorResult(input);
    }
}