namespace sneakers.Filters;

public interface IFilterReducer
{
    void RemoveRedundantConditions(OptimizationContext context);
}