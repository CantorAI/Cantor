#include "gvar.h"
#include "NetworkManager.h"
#include "FramePack.h"
#include "CantorHost.h"
#include "gxydef.h"
#include "service_def.h"
#include "utility.h"

void GVarManager::run()
{
	const int buf_Len = 1000;
	char szFilter[buf_Len];
	SPRINTF(szFilter, 1000,
		"type in (%d,%d)",(int)DataFrameType::Framework_GVar,
		(int)DataFrameType::Framework_GVar_Ack);
	std::string strFilter(szFilter);
	DataViewID	dvGVar = CantorHostImpl::I().CreateDataView(strFilter);
	while (m_run)
	{
		DataFrame frm;
		if (CantorHostImpl::I().PopFrame(dvGVar, 0, -1, frm))
		{
			PackDataFrame pack(frm);
			if (frm.head->type == (int)DataFrameType::Framework_GVar)
			{
				GVar_Op op = (GVar_Op)frm.head->format[0];
				switch (op)
				{
				case GVar_Op::QueryOwner:
				{
					std::string varName;
					pack >> varName;
					GVar* pVar = Query(varName);
					if (pVar && pVar->HaveOwnerId())
					{
						DataFrame frmAck;
						frmAck.head->type = (FrameType)DataFrameType::Framework_GVar_Ack;
						frmAck.head->srcAddr = CantorHostImpl::I().GetNodeId();
						frmAck.head->format[0] = (int)GVar_Op::QueryOwner;
						PackDataFrame packAck(frmAck);
						packAck << varName;
						packAck << pVar->GetOwner();
						bool haveValue = pVar->GetOwner() == CantorHostImpl::I().GetNodeId();
						packAck << haveValue;
						if (haveValue)
						{
							packAck << pVar->GetValue();
						}
						packAck.Finish();
						NetworkManager::I().SendFrameToNode(frm.head->srcAddr,frmAck);
					}
					else
					{//transfor to other nodes
						if (!HaveBeenTransfered(frm.head->srcAddr, frm.head->startTime))
						{
							PushTransferFrameInfo(frm.head->srcAddr, frm.head->startTime);
							std::vector<UID> exclude_uids{ frm.head->srcAddr };
							NetworkManager::I().SendFrameToAllNode(frm, exclude_uids);
						}
					}
				}
				break;
				case GVar_Op::GetValue:
				{
					std::string varName;
					pack >> varName;
					GVar* pVar = Query(varName);
					X::Value val;
					bool bHaveVar = false;
					if (pVar)
					{
						bHaveVar = true;
						val = pVar->GetValue();
					}
					DataFrame frmAck;
					frmAck.head->type = (FrameType)DataFrameType::Framework_GVar_Ack;
					frmAck.head->srcAddr = CantorHostImpl::I().GetNodeId();
					frmAck.head->format[0] = (int)GVar_Op::GetValue;
					PackDataFrame packAck(frmAck);
					packAck << varName;
					packAck << bHaveVar;
					if (bHaveVar)
					{
						packAck << val;
					}
					packAck.Finish();
					NetworkManager::I().SendFrameToNode(frm.head->srcAddr, frmAck);
				}
				break;
				case GVar_Op::SetValue:
				{
					std::string varName;
					pack >> varName;
					GVar* pVar = Query(varName);
					X::Value val;
					pack >> val;
					if (pVar)
					{
						pVar->SetValue(val);
						if (pVar->HaveWait())
						{
							pVar->ReleaseWait();
						}
					}
				}
				break;
				case GVar_Op::Subscribe:
				{
					std::string varName;
					pack >> varName;
					GVar* pVar = Query(varName);
					if (pVar)
					{
						pVar->AddSub(frm.head->srcAddr);
					}
				}
				break;
				case GVar_Op::Unsubscribe:
				{
					std::string varName;
					pack >> varName;
					GVar* pVar = Query(varName);
					if (pVar)
					{
						pVar->RemoveSub(frm.head->srcAddr);
					}
				}
				break;
				case GVar_Op::ValueChanged:
				{
					std::string varName;
					pack >> varName;
					X::Value val;
					pack >> val;
					GVar* pVar = Query(varName);
					if (pVar)
					{
						pVar->SetValue(val);
					}
				}
				break;
				default:
					break;
				}
			}
			else if (frm.head->type == (int)DataFrameType::Framework_GVar_Ack)
			{
				GVar_Op op = (GVar_Op)frm.head->format[0];
				switch (op)
				{
				case GVar_Op::QueryOwner:
				{
					std::string varName;
					pack >> varName;
					UID uid;
					pack >> uid;
					GVar* pVar = Query(varName);
					if (pVar)
					{
						pVar->SetOwner(uid);
					}
					bool HaveValue = false;
					pack >> HaveValue;
					if (HaveValue)
					{
						X::Value val;
						pack >> val;
						if (pVar)
						{
							pVar->SetValue(val);
						}
					}
					if (pVar && pVar->HaveWait())
					{
						pVar->ReleaseWait();
					}
				}
				break;
				case GVar_Op::GetValue:
				{
					std::string varName;
					pack >> varName;
					GVar* pVar = Query(varName);
					bool haveVar = false;
					pack >> haveVar;
					if (haveVar)
					{
						X::Value val;
						pack >> val;
						if (pVar)
						{
							pVar->SetValue(val);
							if (pVar->HaveWait())
							{
								pVar->ReleaseWait();
							}
						}
					}
				}
				break;
				default:
					break;
				}
			}
		}
	}
}
void GVar::Query()
{
	DataFrame frm;
	frm.head->type = (FrameType)DataFrameType::Framework_GVar;
	frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
	frm.head->startTime = getCurMilliTimeStamp();
	frm.head->format[0] = (int)GVar_Op::QueryOwner;
	PackDataFrame pack(frm);
	pack << m_key;
	pack.Finish();
	NetworkManager::I().SendFrameToAllNode(frm);
}
X::Value GVar::Get()
{
	if (m_ownerNodeId == CantorHostImpl::I().GetNodeId())
	{
		return m_val;
	}
	else if (!HaveOwnerId())
	{
		m_haveWait = true;
		m_valIsValid = false;
		Query();
		m_wait->Wait(-1);
		if (m_valIsValid)
		{
			return m_val;
		}
	}
	m_haveWait = true;
	m_valIsValid = false;

	DataFrame frm;
	frm.head->type = (FrameType)DataFrameType::Framework_GVar;
	frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
	frm.head->startTime = getCurMilliTimeStamp();
	frm.head->format[0] = (int)GVar_Op::GetValue;
	PackDataFrame pack(frm);
	pack << m_key;
	pack.Finish();
	NetworkManager::I().SendFrameToNode(m_ownerNodeId, frm);
	m_wait->Wait(-1);
	if (m_valIsValid)
	{
		return m_val;
	}
	else
	{
		return X::Value();
	}
}
bool GVar::Set(X::Value& var)
{
	SetValue(var);
	if (m_ownerNodeId == CantorHostImpl::I().GetNodeId())
	{
		return true;
	}
	if (!HaveOwnerId())
	{
		m_needSendUpdateFrame = true;
		Query();//when SetOwner called, will send out frame to update 
	}
	else
	{
		PostUpdateFrame();
	}
	return true;
}

void GVar::PostUpdateFrame()
{
	DataFrame frm;
	frm.head->type = (FrameType)DataFrameType::Framework_GVar;
	frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
	frm.head->startTime = getCurMilliTimeStamp();
	frm.head->format[0] = (int)GVar_Op::SetValue;
	PackDataFrame pack(frm);
	pack << m_key;
	pack << m_val;
	pack.Finish();
	NetworkManager::I().SendFrameToNode(m_ownerNodeId, frm);
}

void GVar::PostSubscribeFrame()
{
	if (m_ownerNodeId == CantorHostImpl::I().GetNodeId())
	{
		return;
	}
	DataFrame frm;
	frm.head->type = (FrameType)DataFrameType::Framework_GVar;
	frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
	frm.head->startTime = getCurMilliTimeStamp();
	frm.head->format[0] = (int)GVar_Op::Subscribe;
	PackDataFrame pack(frm);
	pack << m_key;
	pack.Finish();
	NetworkManager::I().SendFrameToNode(m_ownerNodeId, frm);
}

void GVar::PostUnsubscribeFrame()
{
	if (m_ownerNodeId == CantorHostImpl::I().GetNodeId())
	{
		return;
	}
	DataFrame frm;
	frm.head->type = (FrameType)DataFrameType::Framework_GVar;
	frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
	frm.head->startTime = getCurMilliTimeStamp();
	frm.head->format[0] = (int)GVar_Op::Unsubscribe;
	PackDataFrame pack(frm);
	pack << m_key;
	pack.Finish();
	NetworkManager::I().SendFrameToNode(m_ownerNodeId, frm);
}

void GVar::NotiSubs()
{
	DataFrame frm;
	frm.head->type = (FrameType)DataFrameType::Framework_GVar;
	frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
	frm.head->startTime = getCurMilliTimeStamp();
	frm.head->format[0] = (int)GVar_Op::ValueChanged;
	PackDataFrame pack(frm);
	pack << m_key;
	pack << m_val;
	pack.Finish();

	std::vector<UID> subs;
	m_lock.Lock();
	subs = m_subscribers;
	m_lock.Unlock();
	for (auto& uid : subs)
	{
		NetworkManager::I().SendFrameToNode(uid, frm);
	}
}
