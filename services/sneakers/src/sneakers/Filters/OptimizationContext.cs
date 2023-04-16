namespace sneakers.Filters;

public class OptimizationContext
{
    public List<IFilter> FilterConditions { get; }

    public OptimizationContext(List<IFilter> filterConditions)
    {
        FilterConditions = filterConditions;
    }
}