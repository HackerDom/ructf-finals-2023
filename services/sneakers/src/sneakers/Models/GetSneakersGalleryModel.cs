namespace sneakers.Models;

public record GetSneakersGalleryModel(int FiltersApplied, IEnumerable<SneakersGallery> Items);