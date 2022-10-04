import cantor thru 'lrpc:1000'
kv = cantor.KV()
host = cantor.Host()
net = cantor.Network()

test_var = cantor.gvar("test_1001")
test_var.create()
test_var.set("from Nuc Server")

kv.Set("NodeName","Galaxy.SanJose.TestServer.Nuc01")
nodeId = kv.Get("NodeId")
if  nodeId == None:
    nodeId = host.generate_uid()
    kv.Set("NodeId",nodeId)
print("NodeId=",nodeId)
Ok = net.StartServer(1973,30)
in0 = "a"
while in0 != 'q':
    in0 = input()
    test_var.set(in0)
print("end")


