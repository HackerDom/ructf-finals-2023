using Microsoft.AspNetCore.Mvc;
using sneakers.AppInfrastructure;
using sneakers.Images;
using sneakers.Models;
using sneakers.Requests;
using sneakers.Storage;

namespace sneakers.Controllers;

[ApiController]
[Route("[controller]")]
public class SneakersController : ControllerBase
{
    private readonly IMasterpieceCreator<Sneakers> creator;
    private readonly ISneakersStorage storage;

    public SneakersController(
        IMasterpieceCreator<Sneakers> creator,
        ISneakersStorage storage)
    {
        this.creator = creator;
        this.storage = storage;
    }

    [HttpPost("masterpiece")]
    public Sneakers Masterpiece([FromBody] CreateMasterpieceRequest request)
    {
        var artist = Context.GetAuthenticatedArtist();

        var palette = creator.GetColors(request.Collection);

        var masterpiece = new Sneakers(
            SneakersId.New(),
            artist,
            request.Collection,
            palette,
            request.Description,
            SneakersToken.New()
        );

        storage.Add(masterpiece);

        return masterpiece;
    }
}