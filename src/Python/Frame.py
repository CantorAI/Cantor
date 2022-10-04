import os
pid = os.getpid()
import pyjit

@pyjit.object(
    lang="cpp",
    Debug=True,
    LibName="self",
    Serialization =True,
    include_dir=["Data/src","Data/inc","Common"],
    ClassName="PyDataFrame",
    include=["PyDataFrame.h"],
    impl=["Data/inc/*.cpp","Data/src/*.cpp","Common/*.cpp"]
    ) 
class DataFrame:
    version:int="__bind__:head->version"
    Type:int ="__bind__:head->type"
    StartTime:int="__bind__:head->startTime"
    SourceId:str
    SrcAddr:str
    DstAddr:str
    Format0:int="__bind__:head->format[0]"
    Format1:int="__bind__:head->format[1]"
    Format2:int="__bind__:head->format[2]"
    Format3:int="__bind__:head->format[3]"
    Format4:int="__bind__:head->format[4]"
    Format5:int="__bind__:head->format[5]"
    Format6:int="__bind__:head->format[6]"
    Format7:int="__bind__:head->format[7]"
    Metadata0:int="__bind__:head->metadata[0]"
    Metadata1:int="__bind__:head->metadata[1]"
    Metadata2:int="__bind__:head->metadata[2]"
    Metadata3:int="__bind__:head->metadata[3]"
    RefID:int="__bind__:head->refId"
    RefIndex:int="__bind__:head->refIndex"
    DataSize:int="__bind__:head->dataSize"
    DataItemNum:int="__bind__:head->dataItemNum"
    Data:any

    def __init__(self,size) -> None:
        pass
    def IsEmpty(self)->bool:
        pass
    def Clone(self,df:"DataFrame"):
        pass

if __name__ == '__main__':
    import numpy as np
    df = DataFrame(1024)
    df_t = type(df)
    t2 = type(DataFrame)
    df.Data = np.random.rand(3,3)
    df.Format0 =10000
    uid = pyjit.generate_uid()
    df.SourceId = uid

    print(df)

