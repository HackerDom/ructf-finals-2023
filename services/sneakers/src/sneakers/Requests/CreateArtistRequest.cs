using sneakers.Models;

namespace sneakers.Requests;

public record CreateArtistRequest(ArtistId Id, string Name);