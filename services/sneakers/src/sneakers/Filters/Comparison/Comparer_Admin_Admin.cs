namespace sneakers.Filters.Comparison;

public class Comparer_Admin_Admin : ConditionsComparer<AdminFilter, AdminFilter>
{
    public override FilterComparisonResult Compare(AdminFilter x, AdminFilter y)
    {
        return x.OwnerToken == y.OwnerToken
            ? FilterComparisonResult.Equal
            : FilterComparisonResult.Contradictory;
    }
}