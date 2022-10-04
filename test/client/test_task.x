import cantor thru 'lrpc:1000'

test_var = cantor.gvar("test_1001")
test_var.changed+=(newVal){
	print("new val:",newVal);
}
test_var.create()
test_var.set("1111")
x = test_var.get()

x=[](size=5000,init='rand(1.0,100.0)')
refId = cantor.Set(x)
x2=[](size=20,init='rand(100.0,200.0)')
refId2 = cantor.Set(x2)
y = cantor.Get(refId)
y2 = cantor.Get(refId2)

@cantor.Task()
def test_func(x,y):
	print("x=",x,",y=",y)
t = test_func.run(10,20)
t.get()
print("end")
