x =1
y=2
z =3
print("\nin-remotingtest,x={}\n".format(x))

def Func1(info):
    global x
    print("inside Func1:info={},x={}".format(info,x))
    x =101
    return x

class Class_For_Remoting():
    def func1(self):
        global x
        print("in Class_For_Remoting.func1")
        x =102
        return 1
    def func2(self,info):
        print("in Class_For_Remoting.func2,",info)
        return info
