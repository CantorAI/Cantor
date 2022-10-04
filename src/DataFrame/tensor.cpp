#include "tensor.h"
#include <string.h>
#include <stdarg.h>  

Tensor::Tensor()
{
	head = nullptr;
	pdims = nullptr;
	data = nullptr;
}
Tensor::Tensor(const Tensor& td)
{
	Copy(td);
}
Tensor::Tensor(char* dataptr)
{
	head = (header*)dataptr;
	int x = sizeof(header);
	pdims = (unsigned long long*)(dataptr + x);
	data = dataptr + sizeof(header) + head->nd * sizeof(unsigned long long);
	int a = 1;
	dimProd.push_back(a);
	for (int i = head->nd - 1; i >= 1; i--)
	{
		a *= (int)pdims[i];
		dimProd.insert(dimProd.begin(), a);
	}
}

Tensor& Tensor::operator=(const Tensor& f)
{
	Copy(f);
	return *this;
}

void Tensor::Copy(const Tensor& f)
{
	for (auto d : f.dimProd)
	{
		dimProd.push_back(d);
	}
	head = f.head;
	pdims = f.pdims;
	data = f.data;
}

TensorFrame::TensorFrame()
{
}

TensorFrame::TensorFrame(const DataFrame& f) :
	DataFrame(f)
{
	ParseData();
}

TensorFrame::TensorFrame(const TensorFrame& t)
{
	Copy(t);
}

TensorFrame::~TensorFrame()
{
}

TensorFrame& TensorFrame::operator=(const DataFrame& f)
{
	DecRef(); //Reduce refcount,may destroy old DataFrame data
	Copy(f);
	ParseData();
	return *this;
}

TensorFrame& TensorFrame::operator=(const TensorFrame& f)
{
	DecRef(); //Reduce refcount,may destroy old DataFrame data
	Copy(f);
	return *this;
}

Tensor& TensorFrame::operator[](const char* key)
{
	auto it = mKV.find(key);
	if (it != mKV.end())
	{
		return it->second;
	}
	else
	{
		return mEmptyTensor;
	}
}

void TensorFrame::Init(std::vector<TensorPackInfo>& packs)
{
	ReleaseMemory();
	//calc total size
	int allSize = sizeof(int) + packs.size() * sizeof(int);//count+{startpos}
	for (auto p : packs)
	{
		int keyLen = p.key.length();
		int headLen = sizeof(header) + p.nd * sizeof(unsigned long long);
		int itemdataLen = 1;
		for (int i = 0; i < p.nd; i++)
		{
			itemdataLen *= p.pdims[i];
		}
		itemdataLen *= p.itemDataSize;

		allSize += (sizeof(int) + keyLen) + headLen + itemdataLen;
	}
	char* pData = new char[allSize];
	int packNum = packs.size();
	*(int*)pData = packNum;

	char* ptrHead = pData + sizeof(int);
	char* ptr = pData + sizeof(int) + packNum * sizeof(int);
	char* ptr0 = ptr;
	for (auto& p : packs)
	{
		*(int*)ptrHead = (ptr - ptr0);
		ptrHead += sizeof(int);

		auto l = *(int*)ptr = p.key.length();
		ptr += sizeof(int);
		memcpy(ptr, p.key.data(), l);
		ptr += l;
		*(int*)ptr = p.itemDataType;
		ptr += sizeof(p.itemDataType);
		*(int*)ptr = p.itemDataSize;
		ptr += sizeof(p.itemDataSize);
		*(int*)ptr = p.nd;
		ptr += sizeof(int);
		memcpy(ptr, p.pdims, sizeof(unsigned long long) * p.nd);
		ptr += sizeof(unsigned long long) * p.nd;
		int tensorSize = 1;
		for (auto k = 0; k < p.nd; k++)
		{
			tensorSize *= p.pdims[k];
		}
		int tSize = tensorSize * p.itemDataSize;
		if (p.data != nullptr)
		{
			memcpy(ptr, p.data, tSize);
		}
		ptr += tSize;
	}
	this->head->dataItemNum = (unsigned int)packNum;
	this->head->dataSize = allSize;
	this->data = pData;

	ParseData();

}

bool TensorFrame::Add(const char* skey, std::vector<int> dims,
	int itemDataSize, TENSOR_DATA_TYPES itemDataType, char* da)
{
	auto it = mKV.find(skey);
	if (it != mKV.end())
	{
		return false;
	}
	std::string key(skey);
	int keyLen = key.length();
	int headLen = sizeof(header) + dims.size() * sizeof(unsigned long long);
	int itemdataLen = 1;
	for (auto n : dims)
	{
		itemdataLen *= n;
	}
	itemdataLen *= itemDataSize;

	int newDataLen = this->head->dataSize;
	if (this->head->dataSize == 0)
	{
		newDataLen += sizeof(int) + sizeof(int);//itemnum+itemStartPos
	}
	else
	{
		newDataLen += sizeof(int);//add one new ItemStartPos
	}
	newDataLen += (sizeof(int) + keyLen) + headLen + itemdataLen;

	char* pData = new char[newDataLen];
	int newItemNum = 1;
	char* pNewDataStart;
	if (this->data != nullptr)
	{
		int itemNum = *(int*)this->data;
		//copy old header
		memcpy(pData + sizeof(int),
			this->data + sizeof(int), itemNum * sizeof(int));
		newItemNum += itemNum;
		int oldTDataSize = this->head->dataSize
			- sizeof(int) - itemNum * sizeof(int);
		memcpy(pData + sizeof(int) + newItemNum * sizeof(int),
			this->data + sizeof(int) + itemNum * sizeof(int),
			oldTDataSize);
		pNewDataStart = pData + sizeof(int) +
			newItemNum * sizeof(int) + oldTDataSize;
		*(int*)(pData + sizeof(int) + itemNum * sizeof(int)) = oldTDataSize;
	}
	else
	{
		*(int*)(pData + sizeof(int)) = 0;
		pNewDataStart = pData + sizeof(int) + sizeof(int);
	}
	*(int*)pData = newItemNum;
	//Pack Key
	*(int*)pNewDataStart = keyLen;
	pNewDataStart += sizeof(int);
	memcpy(pNewDataStart, key.c_str(), keyLen);
	pNewDataStart += keyLen;
	//Add ItemData header
	*(header*)pNewDataStart = { (int)itemDataType,itemDataSize,(int)dims.size() };
	pNewDataStart += sizeof(header);
	for (auto d : dims)
	{
		*(unsigned long long*)pNewDataStart = d;
		pNewDataStart += sizeof(unsigned long long);
	}

	if (da != nullptr)
	{
		memcpy(pNewDataStart, da, itemdataLen);
	}
	if (this->data != nullptr)
	{
		delete[] this->data;
	}
	this->head->dataItemNum = (unsigned int)newItemNum;
	this->data = pData;
	this->head->dataSize = newDataLen;
	mKV.clear();
	ParseData();
	return true;
}

void TensorFrame::Clone(const TensorFrame& f)
{
	DataFrame::Clone(f);
	ParseData();
}

void TensorFrame::Copy(const DataFrame& f)
{
	DataFrame::Copy(f);
	ParseData();
}

void TensorFrame::Copy(const TensorFrame& f)
{
	DataFrame::Copy(f);
	for (auto it : f.mKV)
	{
		mKV.emplace(it);
	}
}

void TensorFrame::ParseData()
{
	if (data == nullptr)
	{
		return;
	}
	//data field:tensor desc+{key:tensor}
	char* ptr = data;
	int itemNum = *(int*)ptr;
	ptr += sizeof(int);
	char* ptrData = ptr + itemNum * sizeof(int);//start after header
	for (int i = 0; i < itemNum; i++)
	{
		int keyStartPos = *(int*)ptr;//start from end of header area
		ptr += sizeof(int);

		int keyLen = *(int*)(ptrData + keyStartPos);
		std::string key(ptrData + keyStartPos + sizeof(int), keyLen);

		int dataStartPos = keyStartPos + sizeof(int) + keyLen;

		char* tensorStartPtr = ptrData + dataStartPos;
		Tensor ts(tensorStartPtr);
		mKV.emplace(std::make_pair(key, ts));
	}
}

