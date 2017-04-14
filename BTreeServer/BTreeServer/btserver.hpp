#ifndef BTREE_SERVER_MJ_
#define BTREE_SERVER_MJ_

#include "acio.hpp"
#include "acio_mock.hpp"
#include "verifier.hpp"
#include "seq_client.hpp"
#include "burst_client.hpp"
#include <cstdint>
#include <map>
#include <vector>

using IO = ACIO<mock, seq_client<100, 10, 30>, verifier>;

template<size_t blockSize>
class BTree {

	using blockArray = std::array<std::uint64_t, blockSize / sizeof(uint64_t)>;


private:

	static const int cacheSize = 10;			// number of blocks in cache
	static const size_t blockSize_64 = blockSize / sizeof(std::uint64_t);
	static const size_t blockSize_32 = blockSize / sizeof(std::uint32_t);
	static const size_t blockSize_8 = blockSize / sizeof(std::uint8_t);
	static const size_t maxRecords = ((blockSize_64 / 2) - 2) % 2 == 0 ? (blockSize_64 / 2) - 2 : (blockSize_64 / 2) - 3;
	static const size_t minRecords = maxRecords / 2;

	blockArray _root;								// root block
	std::uint64_t _rootID = 0;							// ID of root block
	std::array<blockArray, cacheSize> _cache;		// cache
	std::size_t _cachePtr = 0;							// index of oldest cached block;
	std::map<std::uint64_t, size_t> _map;		// maps block_ID to index in memory
	std::map<std::uint64_t, size_t> _cacheMap;	// maps block_ID in cache
	std::size_t _currentSize = 0;				// number of currently allocated blocks
	std::size_t _occupied = 0;					// number of currently occupied blocks
	std::vector<bool> _blockTable;				// table of occupied blocks
	IO & io;									// acio reference
	std::uint64_t _nextID = 1;					// authority giving blocks their IDs

	struct InsertStruct {
		bool split = false;
		std::uint64_t leftBlockID;
		std::uint64_t rightBlockID;
		std::uint64_t pushedKey;
	};

	struct DeleteStruct {
		bool merge = false;
		size_t idx = 0;
	};

	struct MergeStruct {
		bool merged = false;
		std::uint64_t newKey = 0;
	};

	// Resizes tree safely
	bool Resize(size_t size) {
		if (!io.resize(size)) {
			return false;
		}
		_blockTable.resize(size, false);
		_currentSize = size;
		return true;
	}

	// allocates new disk space
	bool Enlarge() {
		size_t newSize = ceil((double)_currentSize * 1.5);
		return Resize(newSize);
	}

	// returns an ID for a new block
	std::uint64_t GetNewID() {
		return _nextID++;
	}

	// initializes new block
	void InitNewBlock(blockArray & block, std::uint64_t newID, bool leaf) {
		// clean memory
		for (size_t i = 0; i < blockSize_64; i++)
			block[i] = -1;
		// save ID
		block[blockSize_64 - 2] = newID;
		// save size
		SetBlockSize(block, 0);
		// save leaf flag
		((std::uint8_t *) block.data())[blockSize_8 - 1] = leaf;
	}

	// returns ID ob block
	std::uint64_t GetBlockID(blockArray & block) {
		return block[blockSize_64 - 2];
	}

	// returns number of keys in block
	std::uint32_t GetBlockSize(blockArray & block) {
		return ((std::uint32_t *) block.data())[blockSize_32 - 2];
	}

	// sets block size
	void SetBlockSize(blockArray & block, std::uint32_t size) {
		((std::uint32_t *) block.data())[blockSize_32 - 2] = size;
	}

	// checks if block is a leaf
	bool IsLeaf(blockArray & block) {
		return ((std::uint8_t *) block.data())[blockSize_8 - 1];
	}

	bool SyncOp(IO&io, IO::op*o)
	{
		std::list<IO::op*> ops;
		//loop until the operation is done, ignore pending requests
		while (io.poll(ops, 1.0, false))
			for (auto&i : ops) if (i == o) {
				io.finish(o);
				return true;
			}
		return false;
	}

	bool SyncWrite(IO&io, size_t block, blockArray & b)
	{
		//call write and wait for it to finish
		IO::op o(block, b.data());
		io.write(&o);
		return SyncOp(io, &o);
	}

	bool SyncRead(IO&io, size_t block, blockArray & b)
	{
		IO::op o(block, b.data());
		io.read(&o);
		return SyncOp(io, &o);
	}

	bool SaveBlock(blockArray & b) {
		auto i = _map.find(GetBlockID(b));
		if (i != _map.end()) {
			return SyncWrite(io, i->second, b);
		}
		return false;
	}

	bool LoadBlock(std::uint64_t id, blockArray & b) {
		auto i = _map.find(id);
		if (i != _map.end()) {
			return SyncRead(io, i->second, b);
		}
		return false;
	}

	bool RemoveBlock(blockArray & b) { return true; };

	size_t GetCacheSpace() {
		size_t rtc = _cachePtr++;
		_cachePtr %= cacheSize;
		return rtc;
	}

	// searches in cache and eventually loads block to cache if it is not yet cached
	blockArray & Block(std::uint64_t id) {
		if (id == _rootID) {
			return _root;
		}
		auto i = _cacheMap.find(id);
		if (i != _cacheMap.end()) {
			// block is cached, no need to load
			return _cache[i->second];
		}
		else {
			// block is not cached, we need to load it
			auto idx = GetCacheSpace();
			auto oldId = GetBlockID(_cache[idx]);
			if (LoadBlock(id, _cache[idx])) {
				_cacheMap[id] = idx;
				_cacheMap.erase(oldId);
				return _cache[idx];
			}
			else {
				// TODO fail
			}
		}
		//return (blockArray &)NULL;
		throw std::exception();
	}

	// returns index of the first unused block
	std::size_t FindFreeSpace() {
		size_t idx;
		for (idx = 0; idx < _currentSize; idx++) {
			if (!_blockTable[idx]) {
				return idx;
			}
		}
		return -1;
	}

	// Registers space for new block
	void RegisterSpace(std::uint64_t id, size_t idx) {
		_map[id] = idx;
		_blockTable[idx] = true;
	}



	// Saves newly created block
	bool SaveNewBlock(blockArray & b) {
		if (_occupied < _currentSize || Enlarge()) {
			auto newSpot = FindFreeSpace();
			RegisterSpace(GetBlockID(b), newSpot);
			if (SaveBlock(b)) {
				_occupied++;
				return true;
			}
		}
		return false;
	}

	// returns ID of block where value for the key should be searched for
	std::uint64_t GetPointerForKey(std::uint64_t key, blockArray & b) {
		for (std::uint32_t i = 0; i < GetBlockSize(b); i++) {
			if (key < b[2 * i + 1]) {
				return b[2 * i];
			}
		}
		return b[2 * GetBlockSize(b)];
	}

	// returns ID of leaf block where key might be
	std::uint64_t SeekLeaf(std::uint64_t key, blockArray & b) {
		if (IsLeaf(b)) {
			return GetBlockID(b);
		}
		else {
			return SeekLeaf(key, Block(GetPointerForKey(key, b)));
		}
	}

	std::uint64_t GetValue(std::uint64_t key, blockArray & b) {
		for (size_t i = 0; i < GetBlockSize(b); i++)
		{
			if (b[2 * i] >= key) {
				if (b[2 * i] == key) {
					return b[2 * i + 1];
				}
				else {
					return 0;
				}
			}
		}
	}

	blockArray & GetNewBlock(bool leaf) {
		auto cacheIdx = GetCacheSpace();
		auto oldId = GetBlockID(_cache[cacheIdx]);
		_cacheMap.erase(oldId);
		auto newId = GetNewID();
		InitNewBlock(_cache[cacheIdx], newId, leaf);
		return _cache[cacheIdx];
	}

	InsertStruct InsertToLeaf(std::uint64_t key, std::uint64_t value, blockArray & b) {
		auto size = GetBlockSize(b);
		// find the spot
		size_t i;
		for (i = 0; i < size; i++)
		{
			if (b[2 * i] >= key) {
				if (b[2 * i] == key) {
					b[2 * i + 1] = value;
					return InsertStruct{ false, 0,0,0 };
				}
				else {
					break;
				}
			}
		}
		// key is not present, need to insert
		if (GetBlockSize(b) == maxRecords) {
			// block is full, must split
			return SplitLeaf(key, value, b);
		}
		else {
			// block is not full, just insert

			// skip smaller keys
			while (b[2 * i] < key && i < GetBlockSize(b)) {
				i++;
			}
			while (i <= GetBlockSize(b)) {
				std::uint64_t tmpKey = b[2 * i];
				std::uint64_t tmpVal = b[2 * i + 1];
				b[2 * i] = key;
				b[2 * i + 1] = value;
				key = tmpKey;
				value = tmpVal;
				i++;
			}
			SetBlockSize(b, GetBlockSize(b) + 1);
			SaveBlock(b);
			return InsertStruct{ false, 0,0,0 };
		}
	}

	InsertStruct SplitLeaf(std::uint64_t key, std::uint64_t value, blockArray & b) {
		auto newBlock = GetNewBlock(true);
		std::uint64_t midVal = 0;
		size_t i = 0;
		while (i < minRecords && key > b[2 * i]) {
			newBlock[2 * i] = b[2 * i];
			newBlock[2 * i + 1] = b[2 * i + 1];
			i++;
		}
		size_t j = 0;
		if (i == minRecords) {
			// new leaf is finished
			if (key < b[2 * i]) {
				midVal = key;
			}
			else {
				midVal = b[2 * i];
				while (key > b[2 * i] && i < maxRecords) {
					b[2 * j] = b[2 * i];
					b[2 * j + 1] = b[2 * i + 1];
					i++; j++;
				}
			}
			b[2 * j] = key;
			b[2 * j + 1] = value;
			j++;
		}
		else {
			newBlock[2 * i] = key;
			newBlock[2 * i + 1] = value;
			while (i < minRecords - 1) {
				newBlock[2 * (i + 1)] = b[2 * i];
				newBlock[2 * (i + 1) + 1] = b[2 * i + 1];
				i++;
			}
			midVal = b[2 * i];
		}
		while (i < maxRecords) {
			b[2 * j] = b[2 * i];
			b[2 * j + 1] = b[2 * i + 1];
			i++; j++;
		}
		// set sizes
		SetBlockSize(newBlock, minRecords);
		SetBlockSize(b, minRecords + 1);

		// Save new block
		SaveNewBlock(newBlock);
		// Save old block
		SaveBlock(b);
		// return information about splitting;
		return InsertStruct{ true, GetBlockID(newBlock), GetBlockID(b), midVal };
	}

	InsertStruct InsertToInnerNode(std::uint64_t key, std::uint64_t value, blockArray & b) {
		auto id = GetPointerForKey(key, b);
		InsertStruct rtc;
		if (IsLeaf(Block(id))) {
			rtc = InsertToLeaf(key, value, Block(id));
		}
		else {
			rtc = InsertToInnerNode(key, value, Block(id));
		}
		if (rtc.split) {
			if (GetBlockSize(b) == maxRecords) {
				// can not insert new key, must split
				return SplitInnerNode(rtc.pushedKey, rtc.leftBlockID, rtc.rightBlockID, b);
			}
			else {
				size_t i = 0;
				while (b[2 * i + 1] < rtc.pushedKey && i < GetBlockSize(b)) {
					i++;
				}
				if (i == GetBlockSize(b)) {
					b[2 * i] = rtc.leftBlockID;
					b[2 * i + 1] = rtc.pushedKey;
					b[2 * i + 2] = rtc.rightBlockID;
				}
				else {
					b[2 * i] = rtc.leftBlockID;
					while (i <= GetBlockSize(b)) {
						std::uint64_t tmpKey = b[2 * i + 1];
						std::uint64_t tmpPtr = b[2 * i + 2];
						b[2 * i + 1] = rtc.pushedKey;
						b[2 * i + 2] = rtc.rightBlockID;
						rtc.pushedKey = tmpKey;
						rtc.rightBlockID = tmpPtr;
						i++;
					}
				}
				SetBlockSize(b, GetBlockSize(b) + 1);
				SaveBlock(b);
				rtc.split = false;
			}
		}
		return rtc;
	}

	InsertStruct SplitInnerNode(std::uint64_t key, std::uint64_t leftNode, std::uint64_t rightNode, blockArray & b) {
		auto newBlock = GetNewBlock(false);
		InsertStruct rtc;
		rtc.split = true;
		size_t i = 0;
		while (i < minRecords && key > b[2 * i + 1]) {
			newBlock[2 * i] = b[2 * i];
			newBlock[2 * i + 1] = b[2 * i + 1];
			i++;
		}
		size_t j = 0;
		if (i == minRecords) {
			// new node is finished
			if (key < b[2 * i + 1]) {
				rtc.pushedKey = key;
				newBlock[2 * i] = leftNode;
			}
			else {
				rtc.pushedKey = b[2 * i + 1];
				newBlock[2 * i] = b[2 * i];
				i++;
				while (key > b[2 * i + 1] && i < maxRecords) {
					b[2 * j] = b[2 * i];
					b[2 * j + 1] = b[2 * i + 1];
					i++; j++;
				}
				b[2 * j] = leftNode;
				b[2 * j + 1] = key;
				j++;
			}
			b[2 * j] = rightNode;
			b[2 * j + 1] = b[2 * i + 1];
			i++; j++;
		}
		else {
			newBlock[2 * i] = leftNode;
			newBlock[2 * i + 1] = key;
			newBlock[2 * (i + 1)] = rightNode;
			if (i < minRecords - 1) {
				newBlock[2 * (i + 1) + 1] = b[2 * i + 1];
				while (i < minRecords - 1) {
					newBlock[2 * (i + 1)] = b[2 * i];
					newBlock[2 * (i + 1) + 1] = b[2 * i + 1];
					i++;
				}
				newBlock[2 * (i + 1)] = b[2 * i];
				rtc.pushedKey = b[2 * i + 1];
			}
			else {
				rtc.pushedKey = b[2 * i + 1];
			}
		}
		while (i < maxRecords) {
			b[2 * j] = b[2 * i];
			b[2 * j + 1] = b[2 * i + 1];
			i++; j++;
		}
		// move last pointer
		b[2 * j] = b[2 * i];

		// set sizes
		SetBlockSize(newBlock, minRecords);
		SetBlockSize(b, minRecords);

		// Save new block
		SaveNewBlock(newBlock);
		// Save old block
		SaveBlock(b);
		// return information about splitting;
		rtc.leftBlockID = GetBlockID(newBlock);
		rtc.rightBlockID = GetBlockID(b);
		return rtc;
	}

	DeleteStruct DeleteFromLeaf(std::uint64_t key, blockArray & b) {
		size_t i = 0;
		auto size = GetBlockSize(b);
		// search the key
		while (key > b[2 * i] && i < size) {
			i++;
		}
		if (key == b[2 * i]) {
			// key was found, must delete it
			if (size > minRecords) {
				// chill, there are enough keys
				for (; i < size - 1; i++)
				{
					b[2 * i] = b[2 * (i + 1)];
					b[2 * i + 1] = b[2 * (i + 1) + 1];
				}
				SetBlockSize(b, size - 1);
				SaveBlock(b);
				return DeleteStruct{ false, 0 };
			}
			else {
				// too few keys, need some
				return DeleteStruct{ true, i };
			}
		}
		// key was not present, no job needs to be done
		return DeleteStruct{ false, 0 };
	}

	MergeStruct MergeLeaves(std::uint64_t leftID, std::uint64_t rightID, std::uint64_t key, size_t removedIdx, bool leftMerge) {
		auto && left = Block(leftID);
		auto && right = Block(rightID);
		if (leftMerge) {
			auto leftSize = GetBlockSize(left);
			if (leftSize == minRecords) {
				// must merge blocks
				size_t i = 0;
				size_t j = leftSize;
				for (; i < removedIdx; i++, j++)
				{
					left[2 * j] = right[2 * i];
					left[2 * j + 1] = right[2 * i + 1];
				}
				// skip deleted pair
				i++;
				for (; i < minRecords; i++, j++) {
					left[2 * j] = right[2 * i];
					left[2 * j + 1] = right[2 * i + 1];
				}
				SaveBlock(left);
				RemoveBlock(right);
				return MergeStruct{ true, 0 };
			}
			else {
				// borrow max element from left				
				for (size_t i = 0; i < removedIdx; i++)
				{
					std::uint64_t tmpKey = right[2 * i];
					std::uint64_t tmpVal = right[2 * i + 1];
					right[2 * i] = left[2 * leftSize];
					right[2 * i + 1] = left[2 * leftSize + 1];
					left[2 * leftSize] = tmpKey;
					left[2 * leftSize + 1] = tmpVal;
				}
				// delete removed pair
				right[2 * removedIdx] = left[2 * leftSize];
				right[2 * removedIdx + 1] = left[2 * leftSize + 1];
				left[2 * leftSize] = -1;
				left[2 * leftSize + 1] = -1;

				SetBlockSize(left, leftSize - 1);

				SaveBlock(left);
				SaveBlock(right);

				return MergeStruct{ false, right[0] };
			}
		}
		else {
			auto rightSize = GetBlockSize(right);
			// delete pair
			size_t i = removedIdx;
			for (; i < minRecords - 1; i++)
			{
				left[2 * i] = left[2 * (i + 1)];
				left[2 * i + 1] = left[2 * (i + 1) + 1];
			}
			if (rightSize == minRecords) {
				// must merge blocks				
				for (size_t j = 0; j < minRecords; i++, j++) {
					left[2 * i] = right[2 * j];
					left[2 * i + 1] = right[2 * j + 1];
				}
				SetBlockSize(left, maxRecords - 1);
				SaveBlock(left);
				RemoveBlock(right);
				return MergeStruct{ true, 0 };
			}
			else {
				// borrow min element from right
				left[2 * minRecords] = right[0];
				left[2 * minRecords + 1] = right[1];
				for (i = 1; i < rightSize; i++)
				{
					right[2 * i] = right[2 * i + 2];
					right[2 * i + 1] = right[2 * i + 3];
				}

				SetBlockSize(right, rightSize - 1);

				SaveBlock(left);
				SaveBlock(right);

				return MergeStruct{ false, right[0] };
			}
		}
	}

	MergeStruct MergeInnerNodes(std::uint64_t leftID, std::uint64_t rightID, std::uint64_t key, size_t removedIdx, bool leftMerge) {
		auto && left = Block(leftID);
		auto && right = Block(rightID);
		if (leftMerge) {
			auto leftSize = GetBlockSize(left);
			if (leftSize == minRecords) {
				// must merge blocks
				size_t i = 0;
				size_t j = leftSize;
				for (; i < removedIdx; i++, j++)
				{
					left[2 * j] = right[2 * i];
					left[2 * j + 1] = right[2 * i + 1];
				}
				// skip deleted key
				left[2 * j] = right[2 * i];
				i++;
				for (; i < minRecords; i++, j++) {
					left[2 * j + 1] = right[2 * i + 1];
					left[2 * j + 2] = right[2 * i + 2];
				}
				SaveBlock(left);
				RemoveBlock(right);
				return MergeStruct{ true, 0 };
			}
			else {
				// borrow max child from left
				MergeStruct rtc = MergeStruct{ false, left[2 * leftSize + 1] };
				std::uint64_t tmpPtr = right[0];
				std::uint64_t tmpKey = right[1];
				right[0] = left[2 * leftSize + 2];
				right[1] = key;
				left[2 * leftSize + 2] = tmpKey;
				left[2 * leftSize + 1] = tmpPtr;
				size_t i = 1;
				for (; i <= removedIdx; i++)
				{
					tmpPtr = right[2 * i];
					tmpKey = right[2 * i + 1];
					right[2 * i] = left[2 * leftSize + 1];
					right[2 * i + 1] = left[2 * leftSize + 2];
					left[2 * leftSize + 2] = tmpKey;
					left[2 * leftSize + 1] = tmpPtr;
				}
				// delete removed key
				right[2 * i] = left[2 * leftSize + 1];
				left[2 * leftSize + 2] = -1;
				left[2 * leftSize + 1] = -1;

				SetBlockSize(left, leftSize - 1);

				SaveBlock(left);
				SaveBlock(right);

				return rtc;
			}
		}
		else {
			auto rightSize = GetBlockSize(right);
			// delete key
			size_t i = removedIdx;
			for (; i < GetBlockSize(left) - 1; i++)
			{
				left[2 * i + 1] = left[2 * 1 + 3];
				left[2 * i + 2] = left[2 * i + 4];
			}
			left[2 * i + 1] = key;
			i++;
			if (rightSize == minRecords) {
				// must merge blocks
				size_t j = 0;
				for (; j < rightSize; i++, j++)
				{
					left[2 * i] = right[2 * j];
					left[2 * i + 1] = right[2 * j + 1];
				}
				// skip deleted key
				left[2 * i] = right[2 * j];
				SaveBlock(left);
				RemoveBlock(right);
				return MergeStruct{ true, 0 };
			}
			else {
				// borrow max child from right
				MergeStruct rtc = MergeStruct{ false, right[1] };
				left[2 * i] = right[0];
				for (i = 1; i < rightSize; i++)
				{
					right[2 * i] = right[2 * i + 2];
					right[2 * i + 1] = right[2 * i + 3];
				}
				right[2 * i] = right[2 * i + 2];
				right[2 * i + 1] = -1;
				right[2 * i + 2] = -1;

				SetBlockSize(right, rightSize - 1);

				SaveBlock(left);
				SaveBlock(right);

				return rtc;
			}
		}
	}

	DeleteStruct DeleteFromInnerNode(std::uint64_t key, blockArray & b) {
		size_t i = 0;
		DeleteStruct rtc;
		while (key >= b[2 * i + 1] && i < GetBlockSize(b)) {
			i++;
		}
		auto && bl = Block(b[2 * i]);
		if (IsLeaf(bl)) {
			rtc = DeleteFromLeaf(key, bl);
			if (rtc.merge) {
				MergeStruct merged;
				if (i == 0) {
					// merge with right sibling
					merged = MergeLeaves(b[2 * i], b[2 * i + 2], b[2 * i + 1], rtc.idx, false);
				}
				else {
					// merge with left sibling
					merged = MergeLeaves(b[2 * i - 2], b[2 * i], b[2 * i - 1], rtc.idx, true);
					// point i to deleted key
					i--;
				}
				if (merged.merged) {
					// leaves merged, must remove key
					if (GetBlockSize(b) <= minRecords) {
						// propagate upwards
						return DeleteStruct{ true, i };
					}
					else {
						// just delete the key
						for (; i < GetBlockSize(b); i++)
						{
							b[2 * i + 1] = b[2 * i + 3];
							b[2 * i + 2] = b[2 * i + 4];
						}
						SetBlockSize(b, GetBlockSize(b) - 1);
						SaveBlock(b);
						return DeleteStruct{ false, 0 };
					}
				}
				else {
					//change the key
					b[2 * i + 1] = merged.newKey;
					SaveBlock(b);
					return DeleteStruct{ false, 0 };
				}
			}
			else {
				return DeleteStruct{ false, 0 };
			}
		}
		else {
			rtc = DeleteFromInnerNode(key, bl);
			if (rtc.merge) {
				MergeStruct merged;
				if (i == 0) {
					// merge with right sibling
					merged = MergeInnerNodes(b[2 * i], b[2 * i + 2], b[2 * i + 1], rtc.idx, false);
				}
				else {
					// merge with left sibling
					merged = MergeInnerNodes(b[2 * i - 2], b[2 * i], b[2 * i - 1], rtc.idx, true);
					// point i to deleted key
					i--;
				}
				if (merged.merged) {
					// nodes merged, must remove key
					if (GetBlockSize(b) == minRecords) {
						// propagate upwards
						return DeleteStruct{ true, i };
					}
					else {
						// just delete the key
						for (; i < GetBlockSize(b); i++)
						{
							b[2 * i + 1] = b[2 * i + 3];
							b[2 * i + 2] = b[2 * i + 4];
						}
						SetBlockSize(b, GetBlockSize(b) - 1);
						SaveBlock(b);
						return DeleteStruct{ false, 0 };
					}
				}
				else {
					//change the key
					b[2 * i + 1] = merged.newKey;
					SaveBlock(b);
					return DeleteStruct{ false, 0 };
				}
			}
			else {
				return DeleteStruct{ false, 0 };
			}
		}
	}

public:
	BTree(IO & io) : io(io)
	{
		Resize(1);
		_rootID = GetNewID();
		InitNewBlock(_root, _rootID, true);
		SaveNewBlock(_root);
	}

	std::uint64_t Read(std::uint64_t key) {
		auto leafID = SeekLeaf(key, _root);
		return GetValue(key, Block(leafID));
	}

	void Write(std::uint64_t key, std::uint64_t value) {
		InsertStruct rtc;
		if (IsLeaf(_root)) {
			rtc = InsertToLeaf(key, value, _root);
		}
		else {
			rtc = InsertToInnerNode(key, value, _root);
		}
		if (rtc.split) {
			// change root manually
			_rootID = GetNewID();
			InitNewBlock(_root, _rootID, false);
			_root[0] = rtc.leftBlockID;
			_root[1] = rtc.pushedKey;
			_root[2] = rtc.rightBlockID;
			SetBlockSize(_root, 1);
			SaveNewBlock(_root);
		}
	}

	void Erase(std::uint64_t key) {
		DeleteStruct rtc;
		if (IsLeaf(_root)) {
			rtc = DeleteFromLeaf(key, _root);
			if (rtc.merge) {
				// just remove key, root doesn't need merging
				size_t i = rtc.idx;
				for (; i < GetBlockSize(_root); i++)
				{
					_root[2 * i] = _root[2 * i + 2];
					_root[2 * i + 1] = _root[2 * i + 3];
				}
				_root[2 * i] = -1;
				_root[2 * i + 1] = -1;

				SetBlockSize(_root, GetBlockSize(_root));
			}
		}
		else {
			rtc = DeleteFromInnerNode(key, _root);
			if (rtc.merge) {
				// children merged, must remove key
				if (GetBlockSize(_root) == 1) {
					// change root
					_root = Block(_root[0]);
					_rootID = _root[0];
				}
				else {
					size_t i = rtc.idx;
					// just delete the key
					for (; i < GetBlockSize(_root); i++)
					{
						_root[2 * i + 1] = _root[2 * i + 3];
						_root[2 * i + 2] = _root[2 * i + 4];
					}
					SetBlockSize(_root, GetBlockSize(_root) - 1);
					SaveBlock(_root);
				}
			}
		}
	}

};


#endif

