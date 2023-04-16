namespace sneakers.Filters;

[Flags]
public enum FilterComparisonResult
{
    Equal = 1,
    Wider = 2,
    Narrower = 4,
    Unrelated = 8,
    Contradictory = 16,
    Consistent = 32
}