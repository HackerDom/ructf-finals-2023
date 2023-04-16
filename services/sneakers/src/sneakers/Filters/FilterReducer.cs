using sneakers.Filters.Comparison;

namespace sneakers.Filters;

public class FilterReducer : IFilterReducer
{
    private readonly Func<IFilter, IFilter, FilterComparisonResult> fieldConditionsComparison;

    public FilterReducer(IFilterComparer comparer)
    {
        fieldConditionsComparison = comparer.CompareFilterConditions;
    }

    public void RemoveRedundantConditions(OptimizationContext context)
    {
        var redundant = FindRedundantConditions(context.FilterConditions, fieldConditionsComparison);
        if (redundant != null) 
            RemoveRedundant(context.FilterConditions, redundant);
    }

    private static List<T>? FindRedundantConditions<T>(
        List<T> conditions,
        Func<T, T, FilterComparisonResult> comparison)
        where T : IFilter
    {
        var redundantConditions = null as HashSet<T>;

        for (var i = 0; i < conditions.Count; i++)
        {
            if (redundantConditions != null && redundantConditions.Contains(conditions[i]))
                continue;

            for (var j = i + 1; j < conditions.Count; j++)
            {
                if (redundantConditions != null && redundantConditions.Contains(conditions[j]))
                    continue;

                var comparisonResult = comparison(conditions[i], conditions[j]);

                if (comparisonResult == FilterComparisonResult.Wider 
                    || comparisonResult == FilterComparisonResult.Equal)
                {
                    (redundantConditions ??= new HashSet<T>()).Add(conditions[i]);
                    break;
                }

                if (comparisonResult == FilterComparisonResult.Narrower)
                {
                    (redundantConditions ??= new HashSet<T>()).Add(conditions[j]);
                }
            }
        }

        return redundantConditions?.ToList();
    }

    private static void RemoveRedundant<T>(List<T> conditions, List<T> redundant)
        where T : IFilter
    {
        if (redundant != null)
            conditions.RemoveAll(x => redundant.Contains(x));
    }
}