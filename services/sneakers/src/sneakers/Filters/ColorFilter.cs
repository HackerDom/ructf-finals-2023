using sneakers.Models;

namespace sneakers.Filters;

public class ColorFilter : IFilter
{
    public required ComparisonOperator ComparisonOperator { get; init; }
    public required ColorChannel ChannelToCompare { get; init; }
    public required byte Operand { get; init; }

    public bool IsSuitable(Sneakers sneakers)
    {
        if (!SelectChannelValue(sneakers.Colors.Main, out var value))
            return false;

        return ComparisonOperator switch
        {
            ComparisonOperator.Greater => value > Operand,
            ComparisonOperator.Less => value < Operand,
            _ => false
        };
    }

    private bool SelectChannelValue(Rgba32 source, out byte value)
    {
        value = default;

        switch (ChannelToCompare)
        {
            case ColorChannel.R:
                value = source.R;
                return true;
            case ColorChannel.G:
                value = source.G;
                return true;
            case ColorChannel.B:
                value = source.B;
                return true;
            default:
                return false;
        }
    }
}