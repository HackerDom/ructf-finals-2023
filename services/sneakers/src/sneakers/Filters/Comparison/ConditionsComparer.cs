namespace sneakers.Filters.Comparison;

public abstract class ConditionsComparer
{
    public abstract FilterComparisonResult Compare(IFilter x, IFilter y);
}

public abstract class ConditionsComparer<T1, T2> : ConditionsComparer
{
    public override FilterComparisonResult Compare(IFilter x, IFilter y) => Compare((T1)x, (T2)y);

    public abstract FilterComparisonResult Compare(T1 x, T2 y);
}