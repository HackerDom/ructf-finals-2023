namespace sneakers.Models;

public record ColorPalette(Rgba32 Main, Rgba32 Secondary, Rgba32 SecondaryLight, Rgba32 Additional)
{
    public static readonly ColorPalette Empty = new ColorPalette(Color.White, Color.White, Color.White, Color.White);
};