using System.Collections.Concurrent;
using System.Text;
using sneakers.Configuration;
using sneakers.Extensions;
using sneakers.Models;
using Vostok.Commons.Time.TimeProviders;
using Vostok.Logging.Abstractions;

namespace sneakers.Storage
{
    public class ArtistsStorage : IArtistsStorage
    {
        private readonly ISettingsProvider settingsProvider;
        private readonly IDateTimeProvider timeProvider;
        private readonly ILog log;

        private readonly ConcurrentDictionary<ArtistId, TimedValue<Artist>> artists = new();
        private readonly ConcurrentDictionary<ArtistToken, Artist> artistsByTokens = new();

        public ArtistsStorage(
            ISettingsProvider settingsProvider,
            IDateTimeProvider timeProvider,
            ILog log
        )
        {
            this.settingsProvider = settingsProvider;
            this.timeProvider = timeProvider;
            this.log = log.ForContext<ArtistsStorage>();
        }

        #region Service

        public void Dump(bool allValues = false)
        {
            var dataPath = settingsProvider.GetSettings().ArtistsDataPath;

            if (!File.Exists(dataPath))
                File.Create(dataPath).Dispose();

            log.Info("Starting to dump data.");

            var values = artists.Select(x => x.Value).ToArray();

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

            foreach (var (key, value) in artists)
                if (value.IsStale(expiredTime))
                {
                    artistsByTokens.Remove(value.Value.Token, out _);
                    artists.Remove(key, out _);
                }

            Dump(true);
        }

        public void Initialize(IEnumerable<TimedValue<Artist>>? initialArtists)
        {
            if (initialArtists == null)
                return;

            foreach (var user in initialArtists)
                AddInternal(user);
        }

        #endregion

        public void Add(Artist artist)
        {
            if (!artists.TryGetValue(artist.Id, out _))
                AddInternal(new TimedValue<Artist>(artist, timeProvider.UtcNow));
        }

        public bool ArtistExists(ArtistToken artistToken, out Artist? artist)
        {
            return artistsByTokens.TryGetValue(artistToken, out artist);
        }

        private void AddInternal(TimedValue<Artist> artistValue)
        {
            var artist = artistValue.Value;
            try
            {
                if (!artists.TryAdd(artist.Id, artistValue))
                    throw new Exception($"Artist with id: {artist.Id} already exists.");
            }
            finally
            {
                artistsByTokens[artist.Token] = artist;
            }
        }
    }
}