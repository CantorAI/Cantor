#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "frame.h"
#include "value.h"

class CantorActivity
{
public:
	CantorActivity();
	~CantorActivity();

	CantorActivity& operator<<(X::Value root);
	//CantorActivity& operator<<(X::Value root);

	std::string& Name() { return m_name; }
	int FindPinIndex(std::string& pinName, bool Input);

	int AddRef()
	{
		return ++m_ref;
	}
	int DecRef()
	{
		int ref = --m_ref;
		if ( ref == 0)
		{
			delete this;
		}
		return ref;
	}
protected:
	std::string m_module;//py file, dll file?
	std::string m_name;
	std::string m_desc;
	UID m_id;

	std::unordered_map<std::string, unsigned long long> m_resMapRequired;
	//key-value for resources such as GPU:4,CPU:16,CUDA:True(1),...
	//match with NodeInfo in NodeManager
	//or point out NodeId here
	std::vector<UID> m_nodeIds;

	std::vector<std::string> m_inputPins;
	std::vector<std::string> m_outputPins;
	bool m_IsSingleton = false;//if true, only support one instance

	int m_ref = 1;
};
struct CantorActivityInstance
{
	CantorActivity* m_activity;
	std::string m_instanceName;
	std::string m_runParams;//like setting, parameters to run
};
struct ConnectInfo
{
	CantorActivityInstance* startActivity;//just ref to item in CantorGraph's m_Instances
	int startPinIndex;
	CantorActivityInstance* endActivity;//just ref to item in CantorGraph's m_Instances
	int endPinIndex;
};
class CantorPipeline;
class CantorGraph
{
public:
	void SetParent(CantorPipeline* p)
	{
		m_parent = p;
	}
	CantorGraph& operator<<(X::Value root);
protected:
	std::vector<CantorActivityInstance> m_Instances;//after CantorGraph created, no change
	std::vector<ConnectInfo> m_connects;
	CantorPipeline* m_parent;
};
class CantorPipeline
{
public:
	CantorPipeline();
	~CantorPipeline();
	CantorPipeline& operator<<(X::Value root);
	//CantorPipeline& operator<<(X::Value root);

	CantorActivity* FindActivity(std::string& actName);
protected:
	std::vector<CantorActivity*> m_activities;
	CantorGraph m_graph;
};
