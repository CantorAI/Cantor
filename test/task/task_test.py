import os
pid = os.getpid()
import cantor
cantor.g["Namespace"] ="task.test"
@cantor.task
def test2(info):
    #time.sleep(0.1)
    print("call test2(10),info=,",info,"namespace=",cantor.g["Namespace"])
    return "ret from test2 with "+info

t =test2("this is a task")
print("end")