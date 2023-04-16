using sneakers.Models;

namespace sneakers.Images;

public interface IMasterpieceCreator<in T>
{
    Image<Rgba32> Create(T sneakers);
    ColorPalette GetColors(Collection collection);
}