#include "CantorActivity.h"
#include "utility.h"

CantorActivity::CantorActivity()
{
}

CantorActivity::~CantorActivity()
{
}
CantorActivity& CantorActivity::operator<<(X::Value root)
{
	//todo
	return *this;// operator<<(&root);
}
int CantorActivity::FindPinIndex(std::string& pinName, bool Input)
{
	if (Input)
	{
		for (int i = 0; i < (int)m_inputPins.size(); i++)
		{
			if (m_inputPins[i] == pinName)
			{
				return i;
			}
		}
	}
	else
	{
		for (int i = 0; i < (int)m_outputPins.size(); i++)
		{
			if (m_outputPins[i] == pinName)
			{
				return (int)m_inputPins.size()+i;
			}
		}
	}
	return -1;
}
#if __todo__ //Using XLang
CantorActivity& CantorActivity::operator<<(X::Value root)
{
	if (root->Type() == SuperNode::NodeType::Map)
	{
		m_name = (std::string)root->Key();
		X::Value map = root->Value();
		if (map.Type() == SuperNode::NodeType::Map)
		{
			m_module = (std::string)map["Module"];
			m_desc = (std::string)map["Desc"];
			m_id = UIDFromString(map["ID"]);
			X::Value inputs = map["Inputs"];
			if (inputs.Type() == SuperNode::NodeType::Sequence)
			{
				for (int i = 0; i < inputs.Count(); i++)
				{
					m_inputPins.push_back(inputs[i]);
				}
			}
			X::Value outputs = map["Outputs"];
			if (outputs.Type() == SuperNode::NodeType::Sequence)
			{
				for (int i = 0; i < outputs.Count(); i++)
				{
					m_outputPins.push_back(outputs[i]);
				}
			}
		}
	}
	return *this;
}
#endif
#if __todo__ //XLANG
CantorPipeline& CantorPipeline::operator<<(X::Value root)
{
	return operator<<(&root);
}
#endif
CantorPipeline& CantorPipeline::operator<<(X::Value root)
{
#if __todo__ //XLANG
	X::Value Root = *(X::Value)root;
	X::Value acts = Root["Activities"];
	if (acts.Type() == SuperNode::NodeType::Sequence)
	{
		for (int i = 0; i < acts.Count(); i++)
		{
			CantorActivity* pAct = new CantorActivity();
			*pAct << acts[i];
			m_activities.push_back(pAct);
		}
	}
	X::Value graph = Root["Graph"];
	m_graph << graph;
#endif
	return *this;
}

CantorPipeline::CantorPipeline()
{
	m_graph.SetParent(this);
}

CantorPipeline::~CantorPipeline()
{
	for (auto p : m_activities)
	{
		p->DecRef();
	}
}

CantorActivity* CantorPipeline::FindActivity(std::string& actName)
{
	for (auto p : m_activities)
	{
		if (p->Name() == actName)
		{
			return p;
		}
	}
	return nullptr;
}

CantorGraph& CantorGraph::operator<<(X::Value root)
{
	X::Value instances = root["ActivityInstances"];
	if (instances.IsList())
	{
		auto size = instances.Size();
		for (int i = 0; i < size; i++)
		{
			CantorActivityInstance instDesc;
#if __todo__ //using xlang
			X::Value inst = instances[i];
			instDesc.m_instanceName = (std::string)inst.Key();
			X::Value inst_descNode = inst.Value();
			std::string actName = inst_descNode["Activity"];
			instDesc.m_activity = m_parent->FindActivity(actName);
			instDesc.m_runParams = (std::string)inst_descNode["Settings"];
#endif
			m_Instances.push_back(instDesc);
		}
	}
	auto findActInstance = [=](std::string instName)
	{
		for (auto& inst : m_Instances)
		{
			if (inst.m_instanceName == instName)
			{
				return &inst;
			}
		}
		return (CantorActivityInstance*)nullptr;
	};
	X::Value connections = root["Connections"];
#if __todo__ //using XLANG 
	if (connections.Type() == SuperNode::NodeType::Sequence)
	{
		for (int i = 0; i < connections.Count(); i++)
		{
			X::Value c = connections[i];
			X::Value start = c["Start"];
			std::string startAct = start["Activity"];
			auto startActInst = findActInstance(startAct);
			std::string startPin = start["Pin"];
			X::Value end = c["End"];
			std::string endAct = end["Activity"];
			auto endActInst = findActInstance(endAct);
			std::string endPin = end["Pin"];
			ConnectInfo connInfo;
			connInfo.startActivity = startActInst;
			connInfo.startPinIndex = (startActInst != nullptr) ? 
				startActInst->m_activity->FindPinIndex(startPin, false):-1;
			connInfo.endActivity = endActInst;
			connInfo.endPinIndex = (endActInst != nullptr) ?
				endActInst->m_activity->FindPinIndex(endPin, true) : -1;
			m_connects.push_back(connInfo);
		}
	}
#endif
	return *this;
}
