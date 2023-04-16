using Microsoft.AspNetCore.Mvc;
using sneakers.AppInfrastructure;
using sneakers.Models;
using sneakers.Requests;
using sneakers.Storage;

namespace sneakers.Controllers;

[ApiController]
[Route("[controller]")]
public class ArtistController : ControllerBase
{
    private readonly IArtistsStorage storage;

    public ArtistController(IArtistsStorage storage)
    {
        this.storage = storage;
    }

    [HttpPost]
    public Artist Create(CreateArtistRequest request)
    {
        var artist = new Artist(request.Id, request.Name, ArtistToken.New());

        storage.Add(artist);

        return artist;
    }

    [HttpGet]
    public Artist Get()
    {
        return Context.GetAuthenticatedArtist();
    }
}