using System.Reflection;
using System.Text.Json;
using SixLabors.ImageSharp.Metadata.Profiles.Exif;
using sneakers.Configuration;
using sneakers.Models;
using Vostok.Commons.Threading;

namespace sneakers.Images;

public class SneakersCreator : IMasterpieceCreator<Sneakers>
{
    private readonly Asset coordinates;
    private readonly Image<Rgba32> template;
    private readonly FileInfo[] velvetCollection;
    private readonly FileInfo[] neuroCollection;

    public SneakersCreator(ISettingsProvider settingsProvider)
    {
        var settings = settingsProvider.GetSettings();

        var content = File.ReadAllText(settings.CoordsPath);
        coordinates = JsonSerializer.Deserialize<Asset>(content)!;
        template = Image.Load<Rgba32>(settings.TemplatePath);

        var directory = new DirectoryInfo(settings.ImagesPath);
        velvetCollection = directory.GetFiles(settings.VelvetPattert);
        neuroCollection = directory.GetFiles(settings.NeuroVelvetPattern);
    }

    public ColorPalette GetColors(Collection collection)
    {
        if (collection != Collection.Pixel)
            return GenerateSpecial(collection);

        var (dark, light) = GenerateTwinColors();

        return new ColorPalette(
            GenerateColor(),
            dark,
            light,
            GenerateColor()
        );
    }

    private ColorPalette GenerateSpecial(Collection collection)
    {
        var index = collection switch
        {
            Collection.Velvet => ThreadSafeRandom.Next(velvetCollection.Length),
            Collection.NeuralVelvet => ThreadSafeRandom.Next(neuroCollection.Length),
            _ => 255
        };

        var indexTruncated = (byte)Math.Min(index, 255);

        return ColorPalette.Empty with { Main = new Rgba32(indexTruncated, 255, 255) };
    }

    public Image<Rgba32> Create(Sneakers sneakers)
    {
        var result = sneakers.Collection switch
        {
            Collection.Pixel => GetPixelCollection(sneakers),
            Collection.Velvet => GetFromCollection(sneakers.Colors, velvetCollection),
            Collection.NeuralVelvet => GetFromCollection(sneakers.Colors, neuroCollection),
            _ => throw new ArgumentException("Unknown palette.")
        };

        var meta = result.Metadata.ExifProfile ?? new ExifProfile();
        meta.SetValue(ExifTag.ImageDescription, sneakers.Description);
        result.Metadata.ExifProfile = meta;

        return result;
    }

    private Image<Rgba32> GetPixelCollection(Sneakers sneakers)
    {
        var result = template.Clone();
        var palette = sneakers.Colors;

        void Set(List<Coord> l, Color color)
        {
            foreach (var coord in l)
                result[coord.X, coord.Y] = color;
        }

        Set(coordinates.Back, Color.White);
        Set(coordinates.Stroke, Color.Black);
        Set(coordinates.SneakerMain, palette.Main);
        Set(coordinates.NikeMain, palette.Secondary);
        Set(coordinates.NikeLight, palette.SecondaryLight);
        Set(coordinates.Laces, palette.Additional);

        return result;
    }

    private static Image<Rgba32> GetFromCollection(ColorPalette sneakersColors, FileInfo[] collection)
    {
        var index = sneakersColors.Main.R;
        if (index > collection.Length - 1)
            throw new ArgumentException("Unknown palette.");

        var file = collection[index].FullName;

        return Image.Load<Rgba32>(file);
    }

    private static (Color dark, Color light) GenerateTwinColors()
    {
        var dark = new Rgba32(
            (byte)ThreadSafeRandom.Next(256),
            (byte)ThreadSafeRandom.Next(128, 192),
            (byte)ThreadSafeRandom.Next(256)
        );

        var light = new Rgba32(
            (byte)Math.Min(dark.R + ThreadSafeRandom.Next(64, 96), 255),
            (byte)Math.Min(dark.G + ThreadSafeRandom.Next(64, 96), 255),
            (byte)Math.Min(dark.B + ThreadSafeRandom.Next(64, 96), 255)
        );

        return (dark, light);
    }

    private static Color GenerateColor()
    {
        var bytes = ThreadSafeRandom.NextBytes(3);
        return new Rgba32(bytes[0], bytes[1], bytes[2], 255);
    }
}