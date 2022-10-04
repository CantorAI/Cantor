#pragma once

#include <map>
#include <vector>
#include "frame.h"
#include "Locker.h"
#include "gxydef.h"

enum class DataViewFilterOp
{
    Equal,NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    In,NotIn,
    Between,NotBetween,
};
struct Condition
{
    int Left;
    DataViewFilterOp Op;
    int Right;
};
class DataViewValue;
class DataView;
typedef DataViewValue* (*ParseValueFunc)(
    DataView* dv, std::string& strValue,
    DataViewValue* pValue);


class DataViewValue
{
public:
    DataViewValue(){}
    virtual ~DataViewValue(){}
    virtual bool IsTrue(DataFrame& frm,
        int fieldOffset, int indexOfArray,
        DataViewFilterOp op) = 0;
protected:
    template<typename V>
    bool CheckExpression(V& v,
        V& ref_v,
        std::vector<V>& ref_vs,
        DataViewFilterOp op)
    {
        bool bRet = false;
        switch (op)
        {
        case DataViewFilterOp::Equal:
            bRet = (v == ref_v);
            break;
        case DataViewFilterOp::NotEqual:
            bRet = (v != ref_v);
            break;
        case DataViewFilterOp::Less:
            bRet = (v < ref_v);
            break;
        case DataViewFilterOp::LessEqual:
            bRet = (v <= ref_v);
            break;
        case DataViewFilterOp::Greater:
            bRet = (v > ref_v);
            break;
        case DataViewFilterOp::GreaterEqual:
            bRet = (v >= ref_v);
            break;
        case DataViewFilterOp::In:
            bRet = false;
            for (auto& ref_v0 : ref_vs)
            {
                if (v == ref_v0)
                {
                    bRet = true;
                    break;
                }
            }
            break;
        case DataViewFilterOp::NotIn:
            bRet = true;
            for (auto& ref_v0 : ref_vs)
            {
                if (v == ref_v0)
                {
                    bRet = false;
                    break;
                }
            }
            break;
        case DataViewFilterOp::Between:
            bRet = ((ref_vs.size() == 2) && (ref_vs[0]<= ref_vs[1]) &&
                (v >= ref_vs[0]) && (v <= ref_vs[1]));
            break;
        case DataViewFilterOp::NotBetween:
            bRet = ((ref_vs.size() == 2) && (ref_vs[0] <= ref_vs[1]) &&
                (v < ref_vs[0]) && (v > ref_vs[1]));
            break;
        default:
            break;
        }
        return bRet;
    }
    bool CheckExpressionUID(UID& v,
        UID& ref_v,
        std::vector<UID>& ref_vs,
        DataViewFilterOp op)
    {
        bool bRet = false;
        switch (op)
        {
        case DataViewFilterOp::Equal:
            bRet = (v.h == ref_v.h && v.l == ref_v.l);
            break;
        case DataViewFilterOp::NotEqual:
            bRet = (v.h != ref_v.h || v.l != ref_v.l);
            break;
        case DataViewFilterOp::In:
            bRet = false;
            for (auto& ref_v0 : ref_vs)
            {
                if (v.h == ref_v0.h && v.l == ref_v0.l)
                {
                    bRet = true;
                    break;
                }
            }
            break;
        case DataViewFilterOp::NotIn:
            bRet = true;
            for (auto& ref_v0 : ref_vs)
            {
                if (v.h == ref_v0.h && v.l == ref_v0.l)
                {
                    bRet = false;
                    break;
                }
            }
            break;
        default:
            break;
        }
        return bRet;
    }
};

class ULValue:
    public DataViewValue
{
public:
    ULValue(unsigned long v)
    {
        m_val = v;
        m_vals.push_back(v);
    }
    void AddValue(unsigned long v)
    {
        m_vals.push_back(v);
    }
    virtual bool IsTrue(DataFrame& frm,
        int fieldOffset, int indexOfArray,
        DataViewFilterOp op) override;
private:
    unsigned long m_val=0;
    std::vector<unsigned long> m_vals;
};
class ULLValue :
    public DataViewValue
{
public:
    ULLValue(unsigned long long v)
    {
        m_val = v;
        m_vals.push_back(v);
    }
    void AddValue(unsigned long long v)
    {
        m_vals.push_back(v);
    }
    virtual bool IsTrue(DataFrame& frm,
        int fieldOffset, int indexOfArray,
        DataViewFilterOp op) override;
private:
    unsigned long long m_val;
    std::vector<unsigned long long> m_vals;
};
enum class ValueType
{
    Scalar,
    NodeId,
    KV
};
class UIDValue :
    public DataViewValue
{
    struct ValInfo
    {
        ValueType type;
        std::string key;
        UID val;
    };
public:
    UIDValue(UID v)
    {
        m_valInfos.push_back(ValInfo{ ValueType::Scalar,"",v});
    }
    UIDValue(std::string& script);
    void AddValue(std::string& script);
    void AddValue(UID v)
    {
        m_valInfos.push_back(ValInfo{ ValueType::Scalar,"",v });
    }
    virtual bool IsTrue(DataFrame& frm,
        int fieldOffset, int indexOfArray,
        DataViewFilterOp op) override;
private:
    std::vector<ValInfo> m_valInfos;
};

struct FieldInfo
{
    std::string name;
    ParseValueFunc parseFunc;
    int offset;
    int itemNum;// for format and metadata in DataFrameHead, it is the ary size, and other is 1
};
struct FilterInfo
{
    int fieldOffset=0;
    int indexOfArray = 0;//for format and metadata in DataFrameHead
    DataViewFilterOp op;
    DataViewValue* value = nullptr;
};
class CantorWait;
class  DataView
{
public:
    DataView();
    ~DataView();

    bool CheckAndInsert(DataFrame& frm);
    bool Pop(DataFrame& df, TimeStamp ts,int timeout_ms);
    void ReleaseWaits();
public:
    static DataView* Create(std::string& filter);
    static void Init();
private:
    bool ParseOneExpression(std::vector<FilterInfo>& filters,std::string& exp);
    bool ParseFilter(std::string filter);
    std::string m_strFilter;
    std::vector <std::vector<FilterInfo>> m_filters;
private:
    static std::vector<FieldInfo> m_FieldInfos;
    static std::vector<std::string> m_ops;
private:
    bool IsInView(DataFrame& frm);
    //DataFrame Queue
    Locker mLockFrms;
    int m_maxCount = 10;
    unsigned long long m_totalSize = 0;
    std::vector<DataFrame> m_frms;
    CantorWait* m_wait = nullptr;
};

