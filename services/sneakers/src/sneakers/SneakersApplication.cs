using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using sneakers.AppInfrastructure;
using sneakers.AppInfrastructure.Middleware;
using sneakers.Configuration;
using sneakers.Extensions;
using sneakers.Models;
using sneakers.Storage;
using Vostok.Applications.AspNetCore;
using Vostok.Applications.AspNetCore.Builders;
using Vostok.Hosting.Abstractions;
using Vostok.Hosting.Abstractions.Requirements;
using Vostok.Throttling.Config;

namespace sneakers
{
    [RequiresConfiguration(typeof(SneakersSettings))]
    internal class SneakersApplication : VostokAspNetCoreWebApplication
    {
        private IStorageManager storageManager;
        private IArtistsStorage artistsStorage;
        private ISneakersStorage sneakersStorage;

        public override Task SetupAsync(
            IVostokAspNetCoreWebApplicationBuilder vostokBuilder,
            IVostokHostingEnvironment environment
        )
        {
            vostokBuilder
                .SetupThrottling(throttling => throttling
                    .UseEssentials(() => new ThrottlingEssentials())
                    .DisableMetrics())
                .SetupWebApplication(app => SetupApp(app, environment))
                .CustomizeWebApplication(ConfigureApp);

            return Task.CompletedTask;
        }

        private static void SetupApp(WebApplicationBuilder builder, IVostokHostingEnvironment environment)
        {
            builder.Services
                .AddControllers()
                .AddJsonOptions(x => x.JsonSerializerOptions.Converters.Add(new JsonStringEnumConverter()));

            DependencyResolution.RegisterDependencies(builder.Services, environment);
        }

        private static void ConfigureApp(WebApplication app)
        {
            app.UseRouting();
            app.UseAuthorization();

            app.UseMiddleware<ExceptionHandleMiddleware>();
            app.UseMiddleware<AuthenticationMiddleware>();

            app.MapControllers();
        }

        public override Task WarmupAsync(IVostokHostingEnvironment environment, WebApplication app)
        {
            var settings = app.Services.GetService<ISettingsProvider>()!.GetSettings();

            artistsStorage = app.Services.GetService<IArtistsStorage>()!;
            InitializeStorage(artistsStorage, settings.ArtistsDataPath);

            sneakersStorage = app.Services.GetService<ISneakersStorage>()!;
            InitializeStorage(sneakersStorage, settings.SneakersDataPath);

            storageManager = app.Services.GetService<IStorageManager>()!;
            storageManager.Start();

            return Task.CompletedTask;
        }

        public override Task DoDisposeAsync()
        {
            storageManager.Stop();

            return Task.CompletedTask;
        }

        private static void InitializeStorage<T>(IManagedStorage<T> storage, string dataPath)
        {
            if (!File.Exists(dataPath))
                return;

            var bytes = File.ReadAllBytes(dataPath);
            if (bytes.Length == 0)
                return;

            var initialElements = Encoding.UTF8.GetString(bytes).FromJson<TimedValue<T>[]>();
            if (initialElements == null)
                throw new JsonException($"Failed to deserialize storage of {typeof(T)}");

            storage.Initialize(initialElements);
        }
    }
}