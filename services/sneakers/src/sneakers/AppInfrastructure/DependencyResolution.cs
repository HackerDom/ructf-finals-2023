using sneakers.AppInfrastructure.Middleware;
using sneakers.Configuration;
using sneakers.Filters;
using sneakers.Filters.Comparison;
using sneakers.Images;
using sneakers.Models;
using sneakers.Storage;
using Vostok.Commons.Time.TimeProviders;
using Vostok.Hosting.Abstractions;
using Vostok.Logging.Abstractions;

namespace sneakers.AppInfrastructure;

public class DependencyResolution
{
    public static void RegisterDependencies(
        IServiceCollection services,
        IVostokHostingEnvironment vostokHostingEnvironment
    )
    {
        services.AddSingleton<ILog>(_ => vostokHostingEnvironment.Log);
        services.AddSingleton<ISettingsProvider>(_ =>
            new SettingsProvider(vostokHostingEnvironment.ConfigurationProvider));

        services.AddSingleton<IDateTimeProvider, DateTimeProvider>();
        services.AddSingleton<IMasterpieceCreator<Sneakers>, SneakersCreator>();

        services.AddSingleton<IAuthenticator, Authenticator>();
        services.AddSingleton<IStorageManager, StorageManager>();
        services.AddSingleton<IFiltersProvider, FiltersProvider>();
        services.AddSingleton<IFilterReducer, FilterReducer>();
        services.AddSingleton<IFilterComparer, FilterComparer>();

        services.AddSingleton<IArtistsStorage, ArtistsStorage>();
        services.AddSingleton<IClearableStorage>(c => c.GetService<IArtistsStorage>());

        services.AddSingleton<ISneakersStorage, SneakersStorage>();
        services.AddSingleton<IClearableStorage>(c => c.GetService<ISneakersStorage>());
    }
}