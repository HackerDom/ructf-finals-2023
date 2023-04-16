namespace sneakers.Models;

public class TimedValue<T>
{
    public DateTime TimeStamp { get; }
    public T Value { get; }

    public TimedValue(T value, DateTime timeStamp)
    {
        Value = value;
        TimeStamp = timeStamp;
    }

    public bool IsStale(DateTime expiration)
    {
        return TimeStamp <= expiration;
    }
}