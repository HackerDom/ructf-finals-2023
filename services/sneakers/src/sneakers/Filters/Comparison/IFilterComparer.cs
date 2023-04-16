namespace sneakers.Filters.Comparison;

public interface IFilterComparer
{
    FilterComparisonResult CompareFilterConditions(IFilter first, IFilter second);
}