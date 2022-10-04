using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.MemoryMappedFiles;
using System.Runtime.InteropServices;

namespace Cantor
{
    internal class SMSwapBuffer
    {
        private const string PAS_MSG_KEY = "\\\\.\\mailslot\\galaxy_msgslot";
        private MemoryMappedFile _MappedFile;
        private MemoryMappedViewAccessor _ViewAccessor;

        private EventWaitHandle? mNotiEvent_Server =null;
        private EventWaitHandle? mNotiEvent_Client =null;
        private bool mClosed = false;
        private int m_BufferSize = 0;
        public bool ClientConnect(UInt64 key, int bufSize,int timeoutMS, bool needSendMsg = true)
        {
            bool bOk = SendMsg(key);
            if(!bOk)
            {
                return false;
            }
            string mappingName = String.Format("Galaxy_FileMappingObject_{0}", key);
            string szKey_s = String.Format("Galaxy_SM_Notify_Server_{0}", key);
            string szKey_c = String.Format("Galaxy_SM_Notify_Client_{0}", key);
            const int loopNum = 1000;
            int loopNo = 0;
            bool bSrvReady = false;
            while (loopNo< loopNum)
            {
                var mmf = MemoryMappedFile.CreateOrOpen(mappingName, bufSize, MemoryMappedFileAccess.ReadWrite);
                if (mmf != null)
                {
                    var accessor = mmf.CreateViewAccessor(0, bufSize);
                    //UInt64 x = 10;
                    //accessor.Read(i, out color);
                    //accessor.Write(0, x);
                    _ViewAccessor = accessor;
                    _MappedFile = mmf;
                    bSrvReady = true;
                    break;
                }
                Thread.Sleep(100);
                loopNo++;
            }
            if (!bSrvReady)
            {
                return false;
            }
            loopNo = 0;
            bSrvReady = false;
            while (mNotiEvent_Server == null && loopNo < loopNum)
            {
                mNotiEvent_Server = EventWaitHandle.OpenExisting(szKey_s);
                if (mNotiEvent_Server != null)
                {
                    bSrvReady = true;
                    break;
                }
                Thread.Sleep(100);
                loopNo++;
            }
            if (!bSrvReady)
            {
                return false;
            }
            loopNo = 0;
            bSrvReady = false;
            while (mNotiEvent_Client == null && loopNo < loopNum)
            {
                mNotiEvent_Client = EventWaitHandle.OpenExisting(szKey_c);
                if (mNotiEvent_Client != null)
                {
                    bSrvReady = true;
                    break;
                }
                Thread.Sleep(100);
                loopNo++;
            }
            if (!bSrvReady)
            {
                return false;
            }
            mClosed = false;
            m_BufferSize = bufSize;

            return true;
        }
        private bool SendMsg(UInt64 key)
        {
            try
            {
                // Create the file, or overwrite if the file exists.
                using (FileStream fs = File.Open(PAS_MSG_KEY, 
                    FileMode.Open,FileAccess.Write,FileShare.Read))
                {
                    byte[] byteBuf = new Byte[16];
                    using (MemoryStream stream = new MemoryStream(byteBuf))
                    {
                        BinaryWriter wr = new BinaryWriter(stream);
                        UInt64 CreateSharedMem = 1;
                        wr.Write(CreateSharedMem);
                        wr.Write(key);
                        stream.Close();
                    }
                    fs.Write(byteBuf,0,16);
                    fs.Close();
                }
            }

            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            return true;
        }
        public void BeginWrite()
        {
            mNotiEvent_Server.Reset();
            mNotiEvent_Client.Reset();
        }
        public void EndWrite()
        {
            mNotiEvent_Server.Set();
        }
        public bool BeginRead(int timeoutMS = -1)
        {
            bool bOK =  mNotiEvent_Client.WaitOne(timeoutMS);
            return bOK&& (!mClosed);
        }
        public void EndRead()
        {

        }
    }
}
