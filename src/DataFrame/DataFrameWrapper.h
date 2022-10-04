#pragma once

#include "frame.h"
#include "xhost.h"
#include "xpackage.h"
#include "xlang.h"
#include "xlstream.h"

class DataFrameWrapper :
	public DataFrame
{
    X::XPackageAPISet<DataFrameWrapper> m_Apis;
public:
    X::XPackageAPISet<DataFrameWrapper>& APISET() { return m_Apis; }
public:
#define ADD_PROP(name) m_Apis.AddPropL(#name, \
            [this](X::Value v) {head->##name = v; },\
            [this]() {return head->##name; });
#define ADD_PROP2(name,convert) m_Apis.AddPropL(#name, \
            [this](X::Value v) {head->##name = convert(v); },\
            [this]() {return convert(head->##name); });
#define ADD_PROP3(name,inner_name) m_Apis.AddPropL(#name, \
            [this](X::Value v) {head->inner_name = v; },\
            [this]() {return head->inner_name; });
    DataFrameWrapper()
    {
        ADD_PROP(type);
        ADD_PROP(startTime);
        ADD_PROP2(sourceId, Convert);
        ADD_PROP2(srcAddr, Convert);
        ADD_PROP2(dstAddr, Convert);
        ADD_PROP3(format1, format[0]);
        ADD_PROP3(format2, format[1]);
        ADD_PROP3(format3, format[2]);
        ADD_PROP3(format4, format[3]);
        ADD_PROP3(format5, format[4]);
        ADD_PROP3(format6, format[5]);
        ADD_PROP3(format7, format[6]);
        ADD_PROP3(format8, format[7]);
        ADD_PROP3(metadata1, metadata[0]);
        ADD_PROP3(metadata2, metadata[1]);
        ADD_PROP3(metadata3, metadata[2]);
        ADD_PROP3(metadata4, metadata[3]);
        ADD_PROP(refId);
        ADD_PROP(refIndex);
        ADD_PROP(dataSize);
        ADD_PROP(dataItemNum);
        m_Apis.AddPropL("data",
            [this](X::Value v) {data = ConvertToData(v); },
            [this]() {return ConvertToData(data); });
        m_Apis.AddFunc<0>("GetData",&DataFrameWrapper::GetData);
        m_Apis.Create(this);
    }
    inline std::string Convert(UID& id)
    {
        return UIDToString(id);
    }
    inline UID Convert(X::Value& vId)
    {
        return UIDFromString(vId.ToString());
    }
    inline X::Value GetData()
    {
        return X::Value(X::g_pXHost->CreateBin(data,head->dataSize));
    }
    inline char* ConvertToData(X::Value& val)
    {
        X::XLStream* pStream = X::g_pXHost->CreateStream();
        val.ToBytes(pStream);
        auto size = pStream->Size();
        Lock();
        if (data)
        {
            delete data;
            data = new char[size];
        }
        pStream->FullCopyTo(data, size);
        head->dataSize = size;
        Unlock();
        X::g_pXHost->ReleaseStream(pStream);
        return data;
    }
    inline X::Value ConvertToData(char* data)
    {
        X::Value val;
        Lock();
        X::XLStream* pStream = X::g_pXHost->CreateStream(data,head->dataSize);
        val.FromBytes(pStream);
        X::g_pXHost->ReleaseStream(pStream);
        Unlock();
        return val;
    }
    DataFrameWrapper(long long size);
    bool Clone(DataFrame* f);
    void setData(X::XObj v);
    X::XObj getData();
    void serialize(unsigned long long streamId, bool InputOrOutput);
private:
    UID UIDFromString(std::string id);
    std::string UIDToString(UID uid);
};