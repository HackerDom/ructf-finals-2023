namespace sneakers.Filters.Comparison;

public class Comparer_Collection_Collection : ConditionsComparer<CollectionFilter, CollectionFilter>
{
    public override FilterComparisonResult Compare(CollectionFilter x, CollectionFilter y)
    {
        return x.Collection == y.Collection
            ? FilterComparisonResult.Equal
            : FilterComparisonResult.Contradictory;
    }
}