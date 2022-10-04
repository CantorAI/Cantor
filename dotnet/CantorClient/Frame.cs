using System;

namespace Cantor
{
    public static class CONST
    {
        public static int DF_FMT_NUM = 8;
        public static int METADATA_NUM = 4;
    }
    public struct UID
    {
        public UInt64 h;
        public UInt64 l;
    }
    public class DataFrameHead
    {
        public const UInt32 Tag = 0x6466726d;//'dfrm'
        public UInt32 Version = 0;

        public UInt64 Type = 0;
        public UInt64 StartTime = 0;

        public UID SourceId;
        public UID SrcAddr;
        public UID DstAddr;

        public UInt64[] Format = new UInt64[CONST.DF_FMT_NUM];
        public UInt64[] Metadata = new UInt64[CONST.METADATA_NUM];

        public UInt32 RefId = 0;
        public UInt32 RefIndex = 0;

        public UInt64 DataSize = 0;
        public UInt64 DataItemNum = 0;//bytesdata, if >=1, Key(string):Val 
    }

    public class DataFrame
    {
        public DataFrameHead? Head;
        public byte[]? Data;
        public UInt64 Context=0;
        public UInt64 Session=0;
    }
}
