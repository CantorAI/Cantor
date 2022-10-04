#include "DataFrameWrapper.h"
#include "tensor.h"

DataFrameWrapper::DataFrameWrapper(long long size)
{
	if (size > 0)
	{
		data = new char[size];
		head->dataSize = (unsigned long long)size;
	}
	else
	{
		data = nullptr;
	}
}

bool DataFrameWrapper::Clone(DataFrame* f)
{
	head = f->head;
	context = f->context;
	session = f->session;
	if (data != nullptr)
	{
		delete[] data;
	}
	data = new char[head->dataSize];
	if (f->data)
	{
		memcpy(data, f->data, head->dataSize);
	}
	return true;
}

void DataFrameWrapper::setData(X::XObj v)
{
#if _-TODO__
	if (v.IsArray())
	{
		PyJit::Array<char> ary(v);
		unsigned int dataSize = (unsigned int)ary.Size();
		head->dataSize = dataSize;
		if (data)
		{
			delete data;
		}
		data = new char[dataSize];
		memcpy(data, ary.GetData(), dataSize);
	}
	else if (v.IsDict())
	{
		PyJit::Dict dict(v);
		auto keys = dict.Keys();
		int cnt = keys.GetCount();

		std::vector<TensorPackInfo> packs;
		std::vector<std::vector<unsigned long long>> dim_all;
		dim_all.resize(cnt);
		for (int i = 0; i < cnt; i++)
		{
			std::string name = (std::string)keys[i];
			TensorPackInfo packInfo = { name,0,0,0,0,0 };
			PyJit::Object val = (PyJit::Object)dict[name.c_str()];
			if (val.IsArray())
			{
				PyJit::Array<char> ary(val);
				packInfo.itemDataType = (int)ary.ItemType();
				packInfo.itemDataSize = ary.ItemSize();
				packInfo.nd = ary.nd();
				dim_all[i] = ary.GetDims();
				packInfo.pdims = dim_all[i].data();
				packInfo.data = (char*)ary.GetData();
				packs.push_back(packInfo);
			}
		}

		TensorFrame ts;
		ts.Init(packs);
		Clone(&ts);
	}
#endif
}

X::XObj DataFrameWrapper::getData()
{
#if __TODO__
	if (head->dataItemNum == 0)
	{
		unsigned long long llSize = head->dataSize;
		PyJit::Array<char> ary(1, &llSize);
		memcpy(ary.GetData(),data,head->dataSize);
		return ary;
	}
	else
	{
		PyJit::Dict dict;
		TensorFrame ts(*this);
		for (auto kv : ts.KV())
		{
			PyJit::Array<char> ary(kv.second.Head().nd, kv.second.Dims(),kv.second.Head().itemDataType);
			memcpy(ary.GetData(), kv.second.D<char>(), ary.SizeOfBytes());
			dict[kv.first.c_str()] = ary;
		}
		return dict;
	}
#endif
	return X::XObj();
}

void DataFrameWrapper::serialize(unsigned long long streamId, bool InputOrOutput)
{
#if __TODO__
	PyJit::Stream stream(streamId);
	if (InputOrOutput)
	{
		stream.Read((char*)head, sizeof(head));
		if (data)
		{
			delete data;
			data = nullptr;
		}
		if (head->dataSize > 0)
		{
			data = new char[head->dataSize];
			stream.Read(data, head->dataSize);
		}
	}
	else
	{
		stream.Write((char*) head, sizeof(head));
		if (data)
		{
			stream.Write(data,head->dataSize);
		}
	}
#endif
}

UID DataFrameWrapper::UIDFromString(std::string id)
{
	UID uid;
	sscanf(id.c_str(),"%016llx%016llx",&uid.h,&uid.l);
	return uid;
}

std::string DataFrameWrapper::UIDToString(UID uid)
{
	char id[32+1];
	id[32] = 0;
	snprintf(id, sizeof(id), "%016llx%016llx", uid.h, uid.l);
	return id;
}
