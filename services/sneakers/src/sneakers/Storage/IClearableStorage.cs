namespace sneakers.Storage
{
    public interface IClearableStorage
    {
        void ClearOldValues();
        void Dump(bool allValues = false);
    }
}