using sneakers.Models;

namespace sneakers.Filters.Comparison;

public class Comparer_Color_Color : ConditionsComparer<ColorFilter, ColorFilter>
{
    public override FilterComparisonResult Compare(ColorFilter x, ColorFilter y)
    {
        if (x.ChannelToCompare != y.ChannelToCompare)
            return FilterComparisonResult.Unrelated;

        return x.ComparisonOperator == y.ComparisonOperator
            ? CompareWithEqualOperators(x, y)
            : CompareWithDifferentOperators(x, y);
    }

    private static FilterComparisonResult CompareWithEqualOperators(ColorFilter x, ColorFilter y)
    {
        if (x.Operand < y.Operand)
            return x.ComparisonOperator == ComparisonOperator.Greater
                ? FilterComparisonResult.Wider
                : FilterComparisonResult.Narrower;

        if (x.Operand > y.Operand)
            return x.ComparisonOperator == ComparisonOperator.Greater
                ? FilterComparisonResult.Narrower
                : FilterComparisonResult.Wider;

        return FilterComparisonResult.Equal;
    }

    private static FilterComparisonResult CompareWithDifferentOperators(ColorFilter x, ColorFilter y)
    {
        if (x.Operand < y.Operand && x.ComparisonOperator == ComparisonOperator.Greater)
            return FilterComparisonResult.Consistent;

        if (y.Operand < x.Operand && x.ComparisonOperator == ComparisonOperator.Less)
            return FilterComparisonResult.Consistent;

        return FilterComparisonResult.Contradictory;
    }
}