namespace sneakers.Configuration
{
    internal class SettingsProvider : ISettingsProvider
    {
        public SettingsProvider(Vostok.Configuration.Abstractions.IConfigurationProvider configProvider)
        {
            provider = configProvider;
        }

        public SneakersSettings GetSettings()
        {
            return provider.Get<SneakersSettings>();
        }

        private readonly Vostok.Configuration.Abstractions.IConfigurationProvider provider;
    }
}