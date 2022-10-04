#include "DataView.h"
#include "utility.h"
#include "CantorWait.h"
#include "NodeManager.h"
#include "CantorHost.h"
#include "port.h"

#define DV_FOURCC( a, b, c, d ) \
        ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
           | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )


std::vector<FieldInfo> DataView::m_FieldInfos;
std::vector<std::string> DataView::m_ops;

void DataView::Init()
{
	static ParseValueFunc ulongFunc
	{
		[](DataView* dv,std::string& strValue,DataViewValue* pValue)
		{
			unsigned long v = 0;
			if (strValue.size() >= 2 && strValue[0] =='\'' 
				&& strValue[strValue.size()-1] == '\'')
			{
				v = (unsigned long)byteStringToNumber(strValue.c_str(), strValue.size() - 2);
			}
			else
			{
				SCANF(strValue.c_str(), "%lu", &v);
			}
			if (pValue)
			{
				((ULValue*)pValue)->AddValue(v);
			}
			else
			{
				pValue = (DataViewValue*)new ULValue(v);
			}
			return pValue;
		}
	};
	static ParseValueFunc ulonglongFunc
	{
		[](DataView* dv,std::string& strValue,DataViewValue* pValue)
		{
			unsigned long long v = 0;
			if (strValue.size() >= 2 && strValue[0] == '\''
				&& strValue[strValue.size() - 1] == '\'')
			{
				v = byteStringToNumber(strValue.c_str()+1, strValue.size() - 2);
			}
			else
			{
				SCANF(strValue.c_str(), "%llu", &v);
			}
			if (pValue)
			{
				((ULLValue*)pValue)->AddValue(v);
			}
			else
			{
			pValue = (DataViewValue*)new ULLValue(v);
			}
			return pValue;
		}
	};
	static ParseValueFunc uidFunc
	{
		[](DataView* dv,std::string& strValue,DataViewValue* pValue)
		{
			if (strValue.size() >= 2 &&
				strValue[0] == '\'' && strValue[strValue.size() - 1] == '\'')
			{
				strValue = strValue.substr(1, strValue.size() - 2);
			}
			if (strValue.size() >=2 && strValue[0] == '$'
				&& strValue[strValue.size() - 1] == '$')
			{//$NodeId:ThisNode$ or $KV:key$
				strValue = strValue.substr(1, strValue.size() - 2);
				if (pValue)
				{
					((UIDValue*)pValue)->AddValue(strValue);
				}
				else
				{
					pValue = (DataViewValue*)new UIDValue(strValue);
				}
			}
			else 
			{
				UID v;
				SCANF(strValue.c_str(), "%llu%llu",&v.h,&v.l);
				if (pValue)
				{
					((UIDValue*)pValue)->AddValue(v);
				}
				else
				{
					pValue = (DataViewValue*)new UIDValue(v);
				}
			}
			return pValue;
		}
	};
	m_FieldInfos =
	{
		{"version",ulongFunc,(int)offsetof(DataFrameHead,version),1},
		{"type",ulonglongFunc,(int)offsetof(DataFrameHead,type),1},
		{"startTime",ulonglongFunc,(int)offsetof(DataFrameHead,startTime),1},
		{"sourceId",uidFunc,(int)offsetof(DataFrameHead,sourceId),1},
		{"srcAddr",uidFunc,(int)offsetof(DataFrameHead,srcAddr),1},
		{"dstAddr",uidFunc,(int)offsetof(DataFrameHead,dstAddr),1},
		{"format",ulonglongFunc,(int)offsetof(DataFrameHead,format),FMT_NUM},
		{"metadata",ulonglongFunc,(int)offsetof(DataFrameHead,metadata),METADATA_NUM},
		{"refId",ulongFunc,(int)offsetof(DataFrameHead,refId),1},
		{"refIndex",ulongFunc,(int)offsetof(DataFrameHead,refIndex),1},
		{"dataSize",ulonglongFunc,(int)offsetof(DataFrameHead,dataSize),1},
		{"dataItemNum",ulonglongFunc,(int)offsetof(DataFrameHead,dataItemNum),1}
	};
	m_ops =
	{
		"==","!=","<","<=",">",">=",
		"in","!in",
		"between","!between"
	};
}

bool DataView::IsInView(DataFrame& frm)
{
	bool bIsInView = false;
	for (auto& group : m_filters)
	{
		bool IsTrueInGroup = true;
		for (auto& f : group)
		{
			if (f.value)
			{
				IsTrueInGroup = f.value->IsTrue(frm, f.fieldOffset,
					f.indexOfArray,f.op);
				if (!IsTrueInGroup)
				{
					break;
				}
			}
		}
		if (IsTrueInGroup)
		{//each group or(||) with other group, just need one group is true
			//then return true
			bIsInView = true;
			break;
		}
	}
	return bIsInView;
}

DataView::DataView()
{
	m_wait = new CantorWait();
}

DataView::~DataView()
{
	for (auto& g : m_filters)
	{
		for (auto& f : g)
		{
			if (f.value)
			{
				delete f.value;
			}
		}
	}
	delete m_wait;
}
bool DataView::Pop(DataFrame& df, TimeStamp ts, int timeout_ms)
{
	bool bHave = false;
	mLockFrms.Lock();
	if (m_frms.size() == 0)
	{
		mLockFrms.Unlock();
		if (!m_wait->Wait(timeout_ms))
		{
			return false;
		}
		mLockFrms.Lock();
	}
	if (ts == 0)
	{
		auto b = m_frms.begin();
		if (b != m_frms.end())
		{
			df = *b;
			m_frms.erase(b);
			m_totalSize -= df.head->dataSize;
			bHave = true;
		}
	}
	else
	{
		for (auto b = m_frms.rbegin(); b != m_frms.rend(); ++b)
		{
			if (ts == b->head->startTime)
			{
				df = *b;
				m_frms.erase((b + 1).base());
				m_totalSize -= df.head->dataSize;
				bHave = true;
				break;
			}
		}
	}
	mLockFrms.Unlock();
	return bHave;
}
void DataView::ReleaseWaits()
{
	if (m_wait)
	{
		m_wait->Release();
	}
}
bool DataView::CheckAndInsert(DataFrame& frm)
{
	bool isIn = IsInView(frm);
	if (isIn)
	{
		mLockFrms.Lock();
		if ((int)m_frms.size() > m_maxCount)
		{//pop up top one
			m_totalSize -= m_frms.begin()->head->dataSize;
			m_frms.erase(m_frms.begin());
		}
		m_frms.push_back(frm);
		m_totalSize += frm.head->dataSize;
		mLockFrms.Unlock();
		m_wait->Release();
	}
	return isIn;
}

DataView* DataView::Create(std::string& filter)
{
	DataView* pDataView = new DataView();
	pDataView->m_strFilter = filter;
	bool bOK = pDataView->ParseFilter(filter);
	if (bOK)
	{
		return pDataView;
	}
	else
	{
		delete pDataView;
		return nullptr;
	}
}

bool DataView::ParseFilter(std::string filter)
{
	ReplaceAll(filter, "\t", " ");
	ReplaceAll(filter, "\n", "");
	ReplaceAll(filter, "\r", "");
	auto groups = split(filter, "||");
	for (auto& g : groups)
	{
		std::vector<FilterInfo> filters;
		auto list = split(g, "&&");
		for (auto& exp : list)
		{
			exp = trim(exp);
			bool bOK = ParseOneExpression(filters,exp);
			if (!bOK)
			{
				return false;
			}
		}
		m_filters.push_back(filters);
	}
	return true;
}

bool DataView::ParseOneExpression(std::vector<FilterInfo>& filters, std::string& exp)
{
	const int kw_num = (int)m_FieldInfos.size();
	const int op_num = (int)m_ops.size();
	int kwIndex = -1;
	int ary_index = 0;
	for (int i = 0; i < kw_num; i++)
	{
		auto size = m_FieldInfos[i].name.size();
		if (0 == STRCMPNOCASE(exp.c_str(), m_FieldInfos[i].name.c_str(), size))
		{
			kwIndex = i;
			exp = exp.substr(size);
			exp = ltrim(exp);
			if (exp.size() > 0 && exp[0] == '[' )
			{
				auto pos = exp.find(']', 1);
				if (pos != std::string::npos)
				{
					SCANF(exp.c_str(), "[%d]", &ary_index);
					exp = exp.substr(pos + 1);
					exp = ltrim(exp);
				}
				else
				{//syntax error
					return false;
				}
			}
			else if (exp.size() > 0 && exp[0] >= '1' && exp[0] <= '9')
			{//for example:format1-format8
				ary_index = exp[0]-'0'-1;
				exp = exp.substr(1);
				exp = ltrim(exp);
			}
			if (ary_index < 0 || ary_index >= m_FieldInfos[i].itemNum)
			{
				return false;
			}
			break;
		}
	}
	if (kwIndex == -1)
	{
		return false;
	}
	int opIndex = -1;
	for (int i = 0; i < op_num; i++)
	{
		auto size = m_ops[i].size();
		if (0 == STRCMPNOCASE(exp.c_str(), m_ops[i].c_str(), size))
		{
			opIndex = i;
			exp = exp.substr(size + 1);
			exp = ltrim(exp);
			break;
		}
	}
	if (opIndex == -1)
	{
		return false;
	}
	if (exp.size() == 0)
	{
		return false;
	}
	DataViewValue* pDVValue = nullptr;
	if (exp[0] == '(')
	{
		size_t pos = exp.rfind(')');
		if (pos == std::string::npos)
		{
			return false;
		}
		exp = exp.substr(1, pos - 1);
		auto list = split(exp, ',');
		for (auto& item : list)
		{
			item = trim(item);
			pDVValue = m_FieldInfos[kwIndex].parseFunc(this, item,pDVValue);
		}
	}
	else
	{
		exp = rtrim(exp);
		pDVValue = m_FieldInfos[kwIndex].parseFunc(this, exp, pDVValue);
	}
	filters.push_back(FilterInfo{
		m_FieldInfos[kwIndex].offset,
		ary_index,
		(DataViewFilterOp)opIndex,
		pDVValue
		});
	return true;
}

bool ULValue::IsTrue(DataFrame& frm,
	int fieldOffset, int indexOfArray,DataViewFilterOp op)
{
	unsigned long frmV = *(unsigned long*)(((char*)frm.head) 
		+ fieldOffset+sizeof(unsigned long)* indexOfArray);
	return CheckExpression<unsigned long>(frmV, m_val, m_vals,op);
}

bool ULLValue::IsTrue(DataFrame& frm,
	int fieldOffset, int indexOfArray,DataViewFilterOp op)
{
	unsigned long long frmV = *(unsigned long long*)(((char*)frm.head) 
		+ fieldOffset+ sizeof(unsigned long long) * indexOfArray);
	return CheckExpression<unsigned long long>(frmV, m_val, m_vals, op);
}

UIDValue::UIDValue(std::string& script)
{
	AddValue(script);
}

void UIDValue::AddValue(std::string& script)
{
	auto list = split(script, ':');
	if (list.size() == 2)
	{
		if (list[0] == "NodeId")
		{
			m_valInfos.push_back(
				ValInfo{ ValueType::NodeId,
				trim(list[1]),UID()});
		}
		else if (list[0] == "KV")
		{
			m_valInfos.push_back(
				ValInfo{ ValueType::KV,
				trim(list[1]),UID() });
		}
	}
}

bool UIDValue::IsTrue(DataFrame& frm,
	int fieldOffset, int indexOfArray,DataViewFilterOp op)
{
	UID val;
	std::vector<UID> ref_vs;
	for (int i = 0; i < (int)m_valInfos.size();i++)
	{
		UID val0;
		auto& it = m_valInfos[i];
		switch (it.type)
		{
		case ValueType::Scalar:
			val0 = it.val;
			break;
		case ValueType::NodeId:
			if (it.key == "ThisNode")
			{
				val0 = CantorHostImpl::I().GetNodeId();
			}
			else
			{
				NodeManager::I().QueryNodeId(it.key, val0);
			}
			break;
		case ValueType::KV:
			break;
		default:
			break;
		}
		if (i == 0)
		{
			val = val0;
		}
		ref_vs.push_back(val0);
	}

	//no array field for UID in DataFrameHead
	UID& frmV = *(UID*)(((char*)frm.head) + fieldOffset);
	return CheckExpressionUID(frmV, val, ref_vs, op);
}
