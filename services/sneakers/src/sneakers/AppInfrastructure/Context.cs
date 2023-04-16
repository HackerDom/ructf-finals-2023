using sneakers.Models;
using Vostok.Context;

namespace sneakers.AppInfrastructure;

public class Context
{
    public static Artist? Artist
    {
        private get => FlowingContext.Globals.Get<Artist>();
        set => FlowingContext.Globals.Set(value);
    }

    public static Artist GetAuthenticatedArtist()
    {
        return Artist ?? throw new Exception($"{nameof(Artist)} is empty.");
    }
}