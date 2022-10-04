import cantor
kv = cantor.KV()
kv.Set("NodeName","Galaxy.SanJose.TestServer")
nodeName = kv.Get("NodeName")
nodeId = utils.generate_uid()
cantor.KVSet("NodeId",nodeId)
nodeId = cantor.KVGet("NodeId")
print("nodeId=",nodeId)

df = cantor.DataFrame(1024*1024)
#df.Type = 100
#df.Format2 = 101
filter ="""
Type == 100 &&
Format2 > 100
"""
dvId = cantor.CreateDataView(filter)
#df.Data = np.random.rand(1024*100,1024*100)
df.SourceId = utils.generate_uid()

cantor.PushFrame(df)
df_back = cantor.DataViewPop(dvId)

for i in range(10):
    try:
        cantor.PushFrame(df)
        df_back = cantor.DataViewPop(dvId)
    except:
        print("ex!\n")
    print(df,i)

print("end")




print("end")
