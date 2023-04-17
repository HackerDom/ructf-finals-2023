using System.Collections.Concurrent;
using System.Text;
using sneakers.Configuration;
using sneakers.Extensions;
using sneakers.Models;
using Vostok.Commons.Time.TimeProviders;
using Vostok.Logging.Abstractions;

namespace sneakers.Storage;

public class SneakersStorage : ISneakersStorage
{
    private readonly ConcurrentDictionary<SneakersToken, TimedValue<Sneakers>> sneakersByTokens = new();
    private readonly ConcurrentDictionary<SneakersId, Sneakers> sneakersByIds = new();

    private readonly IDateTimeProvider timeProvider;
    private readonly ISettingsProvider settingsProvider;
    private readonly ILog log;

    public SneakersStorage(IDateTimeProvider timeProvider, ISettingsProvider settingsProvider, ILog log)
    {
        this.timeProvider = timeProvider;
        this.settingsProvider = settingsProvider;
        this.log = log.ForContext<SneakersStorage>();
    }

    #region Service

    public void Dump(bool allValues = false)
    {
        var dataPath = settingsProvider.GetSettings().SneakersDataPath;

        if (!File.Exists(dataPath))
            File.Create(dataPath).Dispose();

        log.Info("Starting to dump data.");

        var values = sneakersByTokens.Values.ToArray();

        if (!allValues && values.Length < 3)
            return;

        var tmpFileName = $"{dataPath}_tmp_{Guid.NewGuid()}";
        using (var tmpFile = new FileStream(tmpFileName, FileMode.Create))
        {
            tmpFile.Write(Encoding.UTF8.GetBytes(values.ToJson()));
        }

        File.Replace(tmpFileName, dataPath, null);
    }

    public void ClearOldValues()
    {
        log.Info("Starting to drop stale data.");

        var expiredTime = timeProvider.UtcNow - settingsProvider.GetSettings().StorageClearPeriod;

        foreach (var (_, sneakers) in sneakersByTokens)
            if (sneakers.IsStale(expiredTime))
            {
                sneakersByIds.Remove(sneakers.Value.Id, out _);
                sneakersByTokens.Remove(sneakers.Value.DownloadToken, out _);
            }

        Dump(true);
    }

    public void Initialize(IEnumerable<TimedValue<Sneakers>>? initialSneakers)
    {
        if (initialSneakers == null)
            return;

        foreach (var timedValue in initialSneakers)
            Add(timedValue.Value);
    }

    #endregion

    public void Add(Sneakers sneakers)
    {
        if (!sneakersByTokens.TryGetValue(sneakers.DownloadToken, out _))
            AddInternal(new TimedValue<Sneakers>(sneakers, timeProvider.UtcNow));
    }

    public bool TryGet(SneakersToken token, out Sneakers sneakers)
    {
        sneakers = null;

        if (!sneakersByTokens.TryGetValue(token, out var value))
            return false;

        sneakers = value.Value;
        return true;
    }

    public IEnumerable<Sneakers> GetSneakers(List<SneakersId>? idsToSearch)
    {
        return idsToSearch == null
            ? sneakersByTokens.Values.Select(x => x.Value)
            : GetSneakersById(idsToSearch);
    }

    private IEnumerable<Sneakers> GetSneakersById(List<SneakersId> idsToSearch)
    {
        foreach (var sneakersId in idsToSearch)
            if (sneakersByIds.TryGetValue(sneakersId, out var value))
                yield return value;
    }

    private void AddInternal(TimedValue<Sneakers> sneakersValue)
    {
        var sneakers = sneakersValue.Value;
        try
        {
            if (!sneakersByTokens.TryAdd(sneakers.DownloadToken, sneakersValue))
                throw new Exception($"Sneakers with id: {sneakers.Id} already exists.");
        }
        finally
        {
            sneakersByIds[sneakers.Id] = sneakers;
        }
    }
}