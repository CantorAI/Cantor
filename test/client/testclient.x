import cantor thru 'lrpc:1000'
kv = cantor.KV()
host = cantor.Host()

kv.Set("NodeName","Galaxy.SanJose.TestServer")
nodeName = kv.Get("NodeName")
nodeId = host.generate_uid()
kv.Set("NodeId",nodeId)
nodeId = kv.Get("NodeId")

df = cantor.DataFrame(1024*1024)
df.type = 100
df.format2 = 101
filter ="
    Type == 100 &&
    Format2 > 100
"
dvId = host.CreateDataView(filter)
d = [](size=500,init='rand(1.0,100.0)')
df.data = d
df.sourceId = host.generate_uid()

host.PushFrame(df)
df_back = host.PopFrame(dvId,0,-1)

for i in range(10000):
    host.PushFrame(df)
    df_back = host.PopFrame(dvId,0,-1)
    print(df_back,i)

print("end")


