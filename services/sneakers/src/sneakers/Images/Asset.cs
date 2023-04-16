namespace sneakers.Images;

public class Asset
{
    public Asset(
        List<Coord> back,
        List<Coord> sneakerMain,
        List<Coord> laces,
        List<Coord> stroke,
        List<Coord> nikeMain,
        List<Coord> nikeLight)
    {
        Back = back;
        SneakerMain = sneakerMain;
        Laces = laces;
        Stroke = stroke;
        NikeMain = nikeMain;
        NikeLight = nikeLight;
    }

    public List<Coord> Back { get; set; }
    public List<Coord> SneakerMain { get; set; }
    public List<Coord> Laces { get; set; }
    public List<Coord> Stroke { get; set; }
    public List<Coord> NikeMain { get; set; }
    public List<Coord> NikeLight { get; set; }
}