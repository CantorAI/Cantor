namespace Cantor
{
    public class CantorBridge
    {
        public bool Test()
        {
            SMSwapBuffer sm = new SMSwapBuffer();
            sm.ClientConnect(1234, 1024*1024, -1);
            return true;
        }
    }
}