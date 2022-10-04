# coding=utf-8
from functools import wraps
from . import russell

def task(func):
    @wraps(func)    
    def wrapper(*args, **kwargs):
        return russell.SubmitTask(func.__Namespace__,func,[args,kwargs])
    func.__Namespace__ = russell.g["Namespace"]
    return wrapper