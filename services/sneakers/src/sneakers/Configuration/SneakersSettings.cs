using System.Reflection;
using Vostok.Commons.Time;

namespace sneakers.Configuration
{
    public class SneakersSettings
    {
        private static readonly string appPrefix = Environment.CurrentDirectory;
        public static readonly string AppDataPrefix = Path.Combine(appPrefix, "data");
        public static readonly string StorageDataPrefix = Path.Combine(AppDataPrefix, "storage");
        public static readonly string ImageDataPath = Path.Combine(appPrefix, "img");

        public string ArtistsDataPath = Path.Combine(AppDataPrefix, StorageDataPrefix, "artists_data");
        public string SneakersDataPath = Path.Combine(AppDataPrefix, StorageDataPrefix, "sneakers_data");

        public string ImagesPath = ImageDataPath;
        public string CoordsPath = Path.Combine(ImageDataPath, "content.json");
        public string TemplatePath = Path.Combine(ImageDataPath, "template_data");
        public string VelvetPattert = "velvet*";
        public string NeuroVelvetPattern = "neuro*";

        public TimeSpan StorageDumpPeriod = 10.Seconds();
        public TimeSpan StorageClearPeriod = 1.Hours();
    }
}