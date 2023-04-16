namespace sneakers.Models;

public record SneakersGallery(SneakersId Id, ArtistId OwnerId, Collection Collection, SneakersToken DownloadToken);