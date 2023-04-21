using System.Text.Json;
using Microsoft.AspNetCore.Mvc;
using SixLabors.ImageSharp.Formats.Png;
using sneakers.AppInfrastructure;
using sneakers.Filters;
using sneakers.Images;
using sneakers.Models;
using sneakers.Requests;
using sneakers.Storage;

namespace sneakers.Controllers;

[ApiController]
[Route("[controller]")]
public class GalleryController : ControllerBase
{
    private readonly IMasterpieceCreator<Sneakers> creator;
    private readonly ISneakersStorage storage;
    private readonly IFiltersProvider filtersProvider;
    private readonly IFilterReducer reducer;

    public GalleryController(
        IMasterpieceCreator<Sneakers> creator,
        ISneakersStorage storage,
        IFiltersProvider filtersProvider,
        IFilterReducer reducer)
    {
        this.creator = creator;
        this.storage = storage;
        this.filtersProvider = filtersProvider;
        this.reducer = reducer;
    }

    [HttpGet("available")]
    public GetSneakersModel GetAvailable(int skip = 0, int take = 20)
    {
        var sneakers = storage
            .GetSneakers(null)
            .ToList();

        var count = sneakers.Count;
        var result = sneakers
            .Skip(skip)
            .Take(take)
            .Select(x => x.ToModel())
            .Take(count: 50)
            .ToList();

        return new GetSneakersModel(count, result);
    }

    [HttpPost("exhibits")]
    public GetSneakersGalleryModel GetFromGallery([FromBody] GetExhibitsRequest request)
    {
        var artist = Context.GetAuthenticatedArtist();

        var filters = CreateAllOrThrow(artist, request.Conditions);

        return new GetSneakersGalleryModel(
            filters.Count,
            GetFiltered(filters, request.IdsToSearch)
                .Select(s => s.ToGallery())
                .Take(count: 50)
        );
    }

    [HttpGet("show/{downloadToken}")]
    public IActionResult Show(Guid downloadToken)
    {
        if (!storage.TryGet(new SneakersToken(downloadToken), out var sneakers))
            throw new KeyNotFoundException("Unknown download token.");

        var image = creator.Create(sneakers);

        return ToFileResult(name: $"{downloadToken}", image);
    }

    private IEnumerable<Sneakers> GetFiltered(List<IFilter> filters, List<SneakersId>? IdsToSearch)
    {
        foreach (var sneakers in storage.GetSneakers(IdsToSearch))
        {
            if (filters.All(f => f.IsSuitable(sneakers)))
                yield return sneakers;
        }
    }

    private FileStreamResult ToFileResult(string name, Image image)
    {
        var memoryStream = new MemoryStream();
        image.Save(memoryStream, new PngEncoder());
        memoryStream.Seek(0, SeekOrigin.Begin);

        return File(memoryStream, "application/octet-stream", $"{name}.png");
    }

    private List<IFilter> CreateAllOrThrow(Artist artist, List<FilterConditionJsonModel> conditions)
    {
        var filters = new List<IFilter>(conditions.Count);
        foreach (var (filterType, jsonString) in conditions)
        {
            if (!filtersProvider.TryCreateFilter(filterType, jsonString, out var filter))
                throw new JsonException($"Unable to deserialize filter.");

            filters.Add(filter);
        }

        var adminFilter = new AdminFilter { OwnerToken = artist.Token };
        if (!filters.Contains(adminFilter))
            filters.Add(adminFilter);

        var context = new OptimizationContext(filters);
        reducer.RemoveRedundantConditions(context);

        return filters.ToList();
    }
}
