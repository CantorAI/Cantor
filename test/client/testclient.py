import os,time
import numpy as np
import io
pid = os.getpid()
import cantor
import pyjit

cantor.InitClient()

cantor.KVSet("NodeName","Galaxy.SanJose.TestServer")
nodeName = cantor.KVGet("NodeName")
nodeId = pyjit.generate_uid()
cantor.KVSet("NodeId",nodeId)
nodeId = cantor.KVGet("NodeId")

from cantor import Frame
df = Frame.DataFrame(1024*1024)
df.Type = 100
df.Format2 = 101
filter ="""
Type == 100 &&
Format2 > 100
"""
dvId = cantor.CreateDataView(filter)
#df.Data = np.random.rand(1024*100,1024*100)
df.SourceId = pyjit.generate_uid()

cantor.PushFrame(df)
df_back = cantor.DataViewPop(dvId)

for i in range(10000):
    try:
        cantor.PushFrame(df)
        df_back = cantor.DataViewPop(dvId)
    except:
        print("ex!\n")
    print(df,i)
input("Press any key to shutdown")

print("end")


