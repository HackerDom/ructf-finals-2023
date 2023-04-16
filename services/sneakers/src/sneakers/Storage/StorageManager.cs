using sneakers.Configuration;
using Vostok.Commons.Time;
using Vostok.Logging.Abstractions;

namespace sneakers.Storage;

public class StorageManager : IStorageManager
{
    private readonly List<IClearableStorage> storages;
    private readonly PeriodicalAction dumpAction;
    private readonly PeriodicalAction dropAction;
    private readonly ILog log;

    public StorageManager(
        IEnumerable<IClearableStorage> storages,
        ISettingsProvider settingsProvider,
        ILog log
    )
    {
        this.storages = storages.ToList();
        this.log = log.ForContext<StorageManager>();

        dumpAction = new PeriodicalAction(
            () => Dump(),
            e => log.Error(e),
            () => settingsProvider.GetSettings().StorageDumpPeriod,
            true);

        dropAction = new PeriodicalAction(
            () => ClearOldValues(),
            e => log.Error(e),
            () => settingsProvider.GetSettings().StorageClearPeriod,
            true);
    }

    public void Start()
    {
        dumpAction.Start();
        dropAction.Start();
    }

    public void Stop()
    {
        dumpAction.Stop();
        dropAction.Stop();
    }

    private void Dump()
    {
        log.Info("Starting to dump storages.");

        foreach (var storage in storages)
            try
            {
                storage.Dump();
            }
            catch
            {
                // ignored
            }
    }

    private void ClearOldValues()
    {
        log.Info("Starting to clear storages.");

        foreach (var storage in storages)
            try
            {
                storage.ClearOldValues();
            }
            catch
            {
                // ignored
            }
    }
}