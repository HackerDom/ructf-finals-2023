using System.Text.Json;
using System.Text.Json.Serialization;
using sneakers.Configuration;
using sneakers.Filters;
using sneakers.Models;
using Vostok.Configuration.Primitives;
using Vostok.Hosting;
using Vostok.Hosting.Setup;
using Vostok.Logging.File.Configuration;

namespace sneakers;

public class Program
{
    public static async Task Main(string[] args)
    {
        var appDataPath = SneakersSettings.AppDataPrefix;
        var storageDataPath = SneakersSettings.StorageDataPrefix;

        if (!Directory.Exists(appDataPath))
            Directory.CreateDirectory(appDataPath);
        if (!Directory.Exists(storageDataPath))
            Directory.CreateDirectory(storageDataPath);

        var host = new VostokHost(
            new VostokHostSettings(
                new SneakersApplication(),
                SetUpEnvironment
            )
        );

        await host.RunAsync();
    }

    private static void SetUpEnvironment(IVostokHostingEnvironmentBuilder builder)
    {
        DisableUnnecessary(builder)
            .SetupApplicationIdentity(identityBuilder => identityBuilder
                .SetEnvironment("environment")
                .SetProject("project")
                .SetApplication("sneakers")
                .SetInstance("0"))
            .SetupLog(lb => lb
                .SetupConsoleLog()
                .SetupFileLog(flb =>
                    flb.CustomizeSettings(s =>
                    {
                        s.RollingStrategy.Type = RollingStrategyType.BySize;
                        s.RollingStrategy.MaxSize = 10.Megabytes().Bytes;
                    })))
            .SetPort(7777);
    }

    private static IVostokHostingEnvironmentBuilder DisableUnnecessary(IVostokHostingEnvironmentBuilder builder)
    {
        return builder
            .SetupSystemMetrics(s =>
            {
                s.EnableProcessMetricsLogging = false;
                s.EnableHostMetricsLogging = false;
                s.EnableGcEventsLogging = false;
            })
            .DisableHercules()
            .DisableServiceBeacon()
            .DisableClusterConfig();
    }
}