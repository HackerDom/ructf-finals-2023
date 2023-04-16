namespace sneakers.Models;

public record Sneakers(
    SneakersId Id,
    Artist Owner,
    Collection Collection,
    ColorPalette Colors,
    string Description,
    SneakersToken DownloadToken
)
{
    public SneakersModel ToModel()
    {
        return new SneakersModel(Id, Collection, Colors);
    }

    public SneakersGallery ToGallery()
    {
        return new SneakersGallery(Id, Owner.Id, Collection, DownloadToken);
    }
};