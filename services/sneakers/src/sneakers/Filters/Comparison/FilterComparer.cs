namespace sneakers.Filters.Comparison;

public class FilterComparer : IFilterComparer
{
    private readonly Dictionary<(Type, Type), ConditionsComparer> conditionsComparers = new();

    public FilterComparer()
    {
        conditionsComparers.Add((typeof(AdminFilter), typeof(AdminFilter)), new Comparer_Admin_Admin());
        conditionsComparers.Add((typeof(CollectionFilter), typeof(CollectionFilter)), new Comparer_Collection_Collection());
        conditionsComparers.Add((typeof(ColorFilter), typeof(ColorFilter)), new Comparer_Color_Color());
    }

    public FilterComparisonResult CompareFilterConditions(IFilter x, IFilter y)
    {
        if (x.Equals(y))
            return FilterComparisonResult.Equal;

        if (conditionsComparers.TryGetValue((x.GetType(), y.GetType()), out var comparer))
            return comparer.Compare(x, y);

        return FilterComparisonResult.Unrelated;
    }
}