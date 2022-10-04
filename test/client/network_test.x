
import cantor thru 'lrpc:1000'
kv = cantor.KV()
host = cantor.Host()
net = cantor.Network()

Ok = net.Connect("192.168.1.16",1973)
test_var = cantor.gvar("test_1001")
test_var.changed+=(newVal){
	print("new val:",newVal);
}
x = test_var.get()
test_var.set("changed by client side")
y = input()
print("after testFunc call")


