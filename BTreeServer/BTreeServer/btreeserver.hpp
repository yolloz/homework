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
#include <unordered_map>
#include <unordered_set>

using IO = ACIO<mock, burst_client<100, 1000, 3, 10000000>, verifier>;

template<size_t blockSize>
class BTree {

	using blockArray = std::array<std::uint64_t, blockSize / sizeof(uint64_t)>;


private:

	int cacheSize = 10000;			// number of blocks in cache
	static const size_t blockSize_64 = blockSize / sizeof(std::uint64_t);
	static const size_t blockSize_32 = blockSize / sizeof(std::uint32_t);
	static const size_t blockSize_8 = blockSize / sizeof(std::uint8_t);
	static const size_t maxRecords = ((blockSize_64 / 2) - 2) % 2 == 0 ? (blockSize_64 / 2) - 2 : (blockSize_64 / 2) - 3;
	static const size_t minRecords = maxRecords / 2;

	std::uint64_t _rootID = 0;							// ID of root block
	std::unordered_map<std::uint64_t, blockArray> _cache;		// cache
	std::size_t _cachePtr = 0;							// index of oldest cached block;
	std::map<std::uint64_t, size_t> _map;		// maps block_ID to index in memory
	std::vector<std::uint64_t> _cacheHistory;	// maps block_ID in cache
	size_t freeCache = cacheSize;				// number of free cache blocks
	size_t oldestCacheIndex = 0;				// index of oldest id in cache history
	std::size_t _currentSize = 0;				// number of currently allocated blocks
	std::size_t _occupied = 0;					// number of currently occupied blocks
	std::vector<bool> _blockTable;				// table of occupied blocks
	IO & io;									// acio reference
	std::uint64_t _nextID = 1;					// authority giving blocks their IDs
	std::unordered_set<uint64_t> _dirtyBlocks;			// dirty blocks that need to be written
	std::vector<IO::req> _requestQueue;
	size_t _requestCount = 0;
	float _time;
	std::unordered_set<std::uint64_t> _lock;	// 
	

	// provides information after insertion
	struct InsertStruct {
		bool split = false;
		std::uint64_t leftBlockID;
		std::uint64_t rightBlockID;
		std::uint64_t pushedKey;
	};
	
	// provides information after deletion
	struct DeleteStruct {
		bool merge = false;
		size_t idx = 0;
	};

	// provides information after merging
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
		size_t newSize = (size_t)ceil((double)_currentSize * 1.5);
		return Resize(newSize);
	}

	// if there is less than 25% of allocated space occupied, storage is shrunk
	bool Shrink() {
		size_t size = (size_t)ceil((double)_currentSize * 0.25);
		if (_occupied < size) {
			for (auto i = _map.begin(); i != _map.end(); i++) {
				if (i->second >= size) {
					auto newIdx = FindFreeSpace();
					_blockTable[i->second] = false;
					_blockTable[newIdx] = true;
					i->second = newIdx;
					_dirtyBlocks.insert(i->first);
				}
			}
			WriteData();
			io.resize(size);
			_currentSize = size;
			return true;
		}
		return false;
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
		std::uint32_t * ptr;
		ptr = (std::uint32_t *) block.data();
		std::uint32_t  size = ptr[blockSize_32 - 2];
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

	// marks block as dirty - needs saving
	void TouchBlock(blockArray & b) {
		_dirtyBlocks.insert(GetBlockID(b));
	}

	// loads block from storage to cache
	bool LoadBlock(std::uint64_t id) {
		auto i = _map.find(id);
		if (i != _map.end()) {
			if (SyncRead(io, i->second, _cache[id])) {
				GetCacheSpace(id);
				return true;
			}
		}
		return false;
	}

	// disposes block of data
	bool RemoveBlock(blockArray & b) {
		auto id = GetBlockID(b);
		_cache.erase(id);
		auto i = _map.find(id); {
			if (i != _map.end()) {
				_blockTable[i->second] = false;
				_map.erase(id);
				_dirtyBlocks.erase(id);
				Unlock(id);
				_occupied--;
				Shrink();
			}
		}
		return true;
	};

	// alolocates new space in cache for block
	size_t GetCacheSpace(std::uint64_t id) {
		if ((cacheSize - _cache.size()) < 10) {
			FreeCache();
		}
		size_t rtc = _cachePtr++;
		_cachePtr %= cacheSize;
		//_cacheHistory.push_back(8);
		if (_cacheHistory.size() < rtc + 1) {
			_cacheHistory.push_back(id);
		}
		else {
			_cacheHistory[rtc] = id;
		}
		return rtc;
	}

	// searches in cache and eventually loads block to cache if it is not yet cached
	blockArray & Block(std::uint64_t id) {
		auto i = _cache.find(id);
		if (i != _cache.end()) {
			// block is cached, no need to load
			return _cache[id];
		}
		else {
			// block is not cached, we need to load it
			if (LoadBlock(id)) {
				return _cache[id];
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
			TouchBlock(b);
			_occupied++;
			return true;			
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

	// returns value for key from leaf node
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
		return 0;
	}

	// returns newly created empty initialized block 
	// TouchNewBlock method should be used after data is written to it
	blockArray & GetNewBlock(bool leaf) {		
		auto newId = GetNewID();
		auto && rtc = _cache[newId];
		InitNewBlock(rtc, newId, leaf);
		GetCacheSpace(newId);
		return rtc;
	}

	// inserts value to leaf node
	InsertStruct InsertToLeaf(std::uint64_t key, std::uint64_t value, blockArray & b) {
		Lock(GetBlockID(b));
		auto size = GetBlockSize(b);
		// find the spot
		size_t i;
		for (i = 0; i < size; i++)
		{
			if (b[2 * i] >= key) {
				if (b[2 * i] == key) {
					b[2 * i + 1] = value;
					Unlock(GetBlockID(b));
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
			TouchBlock(b);
			Unlock(GetBlockID(b));
			return InsertStruct{ false, 0,0,0 };
		}
	}

	// splits leaf node when it is full
	InsertStruct SplitLeaf(std::uint64_t key, std::uint64_t value, blockArray & b) {
		Lock(GetBlockID(b));
		auto && newBlock = GetNewBlock(true);
		Lock(GetBlockID(newBlock));
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
		while (j < maxRecords) {
			b[2 * j] = -1;
			b[2 * j + 1] = -1;
			j++;
		}
		// set sizes
		SetBlockSize(newBlock, minRecords);
		SetBlockSize(b, minRecords + 1);

		// Save new block
		SaveNewBlock(newBlock);
		// Save old block
		TouchBlock(b);
		// return information about splitting;
		Unlock(GetBlockID(b));
		Unlock(GetBlockID(newBlock));
		return InsertStruct{ true, GetBlockID(newBlock), GetBlockID(b), midVal };
	}

	// inserts key to inner node, propagates value towards leaves
	InsertStruct InsertToInnerNode(std::uint64_t key, std::uint64_t value, blockArray & b) {
		Lock(GetBlockID(b));
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
				TouchBlock(b);
				rtc.split = false;
			}
		}
		Unlock(GetBlockID(b));
		return rtc;
	}

	// splits inner node when it is full
	InsertStruct SplitInnerNode(std::uint64_t key, std::uint64_t leftNode, std::uint64_t rightNode, blockArray & b) {
		Lock(GetBlockID(b));
		auto && newBlock = GetNewBlock(false);
		Lock(GetBlockID(newBlock));
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
		TouchBlock(b);
		// return information about splitting;
		rtc.leftBlockID = GetBlockID(newBlock);
		rtc.rightBlockID = GetBlockID(b);
		Unlock(GetBlockID(b));
		Unlock(GetBlockID(newBlock));
		return rtc;
	}

	// deletes value from leaf node
	DeleteStruct DeleteFromLeaf(std::uint64_t key, blockArray & b) {
		Lock(GetBlockID(b));
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
				TouchBlock(b);
				Unlock(GetBlockID(b));
				return DeleteStruct{ false, 0 };
			}
			else {
				// too few keys, need some
				Unlock(GetBlockID(b));
				return DeleteStruct{ true, i };
			}
		}
		// key was not present, no job needs to be done
		Unlock(GetBlockID(b));
		return DeleteStruct{ false, 0 };
	}

	// if leaf has too few pairs after deleting, it merges or
	// borrows key from sibling. If merge occured, right node is deleted and left filled
	MergeStruct MergeLeaves(std::uint64_t leftID, std::uint64_t rightID, std::uint64_t key, size_t removedIdx, bool leftMerge) {
		Lock(leftID);
		Lock(rightID);		
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
				while (j < maxRecords) {
					left[2 * j] = -1;
					left[2 * j + 1] = -1;
					j++;
				}

				SetBlockSize(left, maxRecords - 1);
				Unlock(leftID);
				Unlock(rightID);
				TouchBlock(left);
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

				TouchBlock(left);
				TouchBlock(right);

				Unlock(leftID);
				Unlock(rightID);
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
				while (i < maxRecords)
				{
					left[2 * i] = -1;
					left[2 * i + 1] = -1;
					i++;
				}
				SetBlockSize(left, maxRecords - 1);
				Unlock(leftID);
				Unlock(rightID);
				TouchBlock(left);
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
				right[2 * i] = -1;
				right[2 * i + 1] = -1;

				SetBlockSize(right, rightSize - 1);

				TouchBlock(left);
				TouchBlock(right);

				Unlock(leftID);
				Unlock(rightID);

				return MergeStruct{ false, right[0] };
			}
		}
	}

	// if inner node has too few children after deleting, it merges or
	// borrows a child from sibling. If merge occured, right node is deleted and left filled
	MergeStruct MergeInnerNodes(std::uint64_t leftID, std::uint64_t rightID, std::uint64_t key, size_t removedIdx, bool leftMerge) {
		Lock(leftID);
		Lock(rightID);
		auto && left = Block(leftID);
		auto && right = Block(rightID);
		if (leftMerge) {
			auto leftSize = GetBlockSize(left);
			if (leftSize == minRecords) {
				// must merge blocks
				left[2 * leftSize + 1] = key;
				size_t i = 0;
				size_t j = leftSize + 1;
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
				SetBlockSize(left, maxRecords);

				Unlock(leftID);
				Unlock(rightID);

				TouchBlock(left);
				RemoveBlock(right);
				return MergeStruct{ true, 0 };
			}
			else {
				// borrow max child from left
				MergeStruct rtc = MergeStruct{ false, left[2 * leftSize - 1] };
				std::uint64_t tmpPtr = right[0];
				std::uint64_t tmpKey = right[1];
				right[0] = left[2 * leftSize];
				right[1] = key;
				left[2 * leftSize] = tmpKey;
				left[2 * leftSize - 1] = tmpPtr;
				size_t i = 1;
				for (; i <= removedIdx; i++)
				{
					tmpPtr = right[2 * i];
					tmpKey = right[2 * i + 1];
					right[2 * i] = left[2 * leftSize - 1];
					right[2 * i + 1] = left[2 * leftSize];
					left[2 * leftSize] = tmpKey;
					left[2 * leftSize - 1] = tmpPtr;
				}
				// delete ptr
				right[2 * i] = left[2 * leftSize - 1];
				left[2 * leftSize] = -1;
				left[2 * leftSize - 1] = -1;

				SetBlockSize(left, leftSize - 1);

				TouchBlock(left);
				TouchBlock(right);

				Unlock(leftID);
				Unlock(rightID);

				return rtc;
			}
		}
		else {
			auto rightSize = GetBlockSize(right);
			// delete key
			size_t i = removedIdx;
			for (; i < GetBlockSize(left) - 1; i++)
			{
				left[2 * i + 1] = left[2 * i + 3];
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
				// add end pointer
				left[2 * i] = right[2 * j];

				SetBlockSize(left, maxRecords);

				Unlock(leftID);
				Unlock(rightID);

				TouchBlock(left);
				RemoveBlock(right);
				return MergeStruct{ true, 0 };
			}
			else {
				// borrow min child from right
				MergeStruct rtc = MergeStruct{ false, right[1] };
				left[2 * i] = right[0];
				for (i = 0; i < rightSize; i++)
				{
					right[2 * i] = right[2 * i + 2];
					right[2 * i + 1] = right[2 * i + 3];
				}
				right[2 * i] = right[2 * i + 2];
				right[2 * i + 1] = -1;
				right[2 * i + 2] = -1;

				SetBlockSize(right, rightSize - 1);

				TouchBlock(left);
				TouchBlock(right);

				Unlock(leftID);
				Unlock(rightID);

				return rtc;
			}
		}
	}

	// deletes key from inner node or returns information that it needs merging
	DeleteStruct DeleteFromInnerNode(std::uint64_t key, blockArray & b) {
		Lock(GetBlockID(b));
		size_t i = 0;
		DeleteStruct rtc;
		auto size = GetBlockSize(b);
		auto id = GetBlockID(b);
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
					auto size2 = GetBlockSize(b);
					auto id2 = GetBlockID(b);
					if (GetBlockSize(b) <= minRecords) {
						// propagate upwards
						Unlock(GetBlockID(b));
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
						TouchBlock(b);
						Unlock(GetBlockID(b));
						return DeleteStruct{ false, 0 };
					}
				}
				else {
					//change the key
					b[2 * i + 1] = merged.newKey;
					TouchBlock(b);
					Unlock(GetBlockID(b));
					return DeleteStruct{ false, 0 };
				}
			}
			else {
				Unlock(GetBlockID(b));
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
					if (GetBlockSize(b) <= minRecords) {
						// propagate upwards
						Unlock(GetBlockID(b));
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
						TouchBlock(b);
						Unlock(GetBlockID(b));
						return DeleteStruct{ false, 0 };
					}
				}
				else {
					//change the key
					b[2 * i + 1] = merged.newKey;
					TouchBlock(b);
					Unlock(GetBlockID(b));
					return DeleteStruct{ false, 0 };
				}
			}
			else {
				Unlock(GetBlockID(b));
				return DeleteStruct{ false, 0 };
			}
		}
	}

	inline void Lock(std::uint64_t blockID) {
		_lock.insert(blockID);
	}

	inline void Unlock(std::uint64_t blockID) {
		_lock.erase(blockID);
	}

	inline bool IsLocked(std::uint64_t blockID) {
		return _lock.find(blockID) != _lock.end();
	}

	// erases oldest blocks from cache
	void FreeCache() {
		if ((cacheSize - _cache.size()) < 50) {
			const size_t newSpace = (size_t)(cacheSize * 0.1);
			std::unordered_map<IO::op*, std::uint64_t> writeOps;
			std::vector<IO::op> opsArray;
			opsArray.reserve(newSpace);
			auto idx = oldestCacheIndex;
			size_t j = 0;
			for (size_t i = 0; i < newSpace; i++)
			{
				if (_dirtyBlocks.find(_cacheHistory[idx]) != _dirtyBlocks.end()) {
					auto itr = _map.find(_cacheHistory[idx]);
					if (itr != _map.end() && !IsLocked(_cacheHistory[idx])) {
						IO::op o(itr->second, Block(itr->first).data());
						opsArray.push_back(o);
						IO::op * ptr = &opsArray[j++];
						io.write(ptr);
						writeOps[ptr] = _cacheHistory[idx];
					}
				}
				else {
					auto bz = _cache.erase(_cacheHistory[idx]);
				}
				idx++;
				idx %= cacheSize;
			}
			std::list<IO::op*> ops;
			while (io.poll(ops, 1, false)) {
				for (IO::op* i : ops)
				{
					auto op = writeOps.find(i);
					if (op != writeOps.end()) {
						bool done = io.finish(op->first);
						auto bz = _dirtyBlocks.erase(op->second);
						_cache.erase(op->second);
						writeOps.erase(op->first);
					}
				}
				if (writeOps.size() == 0) {
					break;
				}
			}
			oldestCacheIndex = idx;
		}
	}

	// returns reference to space for new request
	IO::req & NewRequest() {
		if (_requestQueue.size() <= _requestCount) {
			_requestQueue.push_back(IO::req());
		}
		return _requestQueue[_requestCount];
	}

	// Confirms new request, returns true if more requests should be waited for
	bool ConfirmNewRequest(IO::req & r, float timestamp) {
		++_requestCount;
		return _time + 0.1 > timestamp;
	}

	// Processes queued requests
	void Process() {
		for (size_t i = 0; i < _requestCount; i++) {
			auto && r = _requestQueue[i];
			switch (r.type) {
			case IO::req::write: {
				//handle client writing into the database
				Write(r.key, r.value);
				//std::cerr << "write " << r.key << " = " << r.value << std::endl;
				//send the confirmation
				//io.send_reply(&r);
				break;
			}
			case IO::req::read: {
				r.value = 0;
				//find the key
				r.value = Read(r.key);
				//std::cerr << "read " << r.key << " --> " << r.value << std::endl;
				//send the reply with the value
				//io.send_reply(&r);
				break;
			}
			case IO::req::erase:
				//in erasing case, find the key
				Erase(r.key);
				r.value = 0;
				//std::cerr << "erase " << r.key << std::endl;
				//and send the confirmation.
				//io.send_reply(&r);
				break;
			default:
				break;
			}
		}
	}

	// writes dirty blocks to storage
	void WriteData() {
		std::unordered_map<IO::op*, std::uint64_t> writeOps;
		std::vector<IO::op> opsArray;
		opsArray.reserve(_dirtyBlocks.size() * 1.2);
		// issue write ops for all dirty blocks
		size_t j = 0;
		for (auto i = _dirtyBlocks.begin(); i != _dirtyBlocks.end(); i++)
		{
			auto itr = _map.find(*i);
			if (itr != _map.end()) {
				IO::op o(itr->second, Block(itr->first).data());
				opsArray.push_back(o);
				IO::op * ptr = &opsArray[j++];
				io.write(ptr);
				writeOps[ptr] = *i;
			}
		}
		std::list<IO::op*> ops;
		while (io.poll(ops, 1, false)) {
			for (IO::op* i : ops)
			{
				auto op = writeOps.find(i);
				if (op != writeOps.end()) {
					bool done = io.finish(op->first);
					_dirtyBlocks.erase(op->second);
					writeOps.erase(op->first);
				}
			}
			if (writeOps.size() == 0) {
				break;
			}
		}
		_dirtyBlocks.clear();
	}

	// answers finished requests
	void Answer() {
		for (size_t i = 0; i < _requestCount; i++)
		{
			io.send_reply(&_requestQueue[i]);
		}
	}

	// resets server to waiting state
	void Reset(float timestamp) {
		_requestQueue.clear();
		_requestCount = 0;
		_time = timestamp;
	}

public:
	// Constructs default B+ tree server
	BTree(IO & io) : io(io)
	{
		Resize(1);
		auto && root = GetNewBlock(true);
		SaveNewBlock(root);
		_rootID = GetBlockID(root);
	}

	// Constructs B+ tree server with chosen internal cache size
	BTree(IO & io, int cacheSize) : io(io), cacheSize(cacheSize)
	{
		Resize(1);
		auto && root = GetNewBlock(true);
		SaveNewBlock(root);
		_rootID = GetBlockID(root);
	}

	// retrieves value from tree
	std::uint64_t Read(std::uint64_t key) {
		auto leafID = SeekLeaf(key, Block(_rootID));
		return GetValue(key, Block(leafID));
	}

	// inserts key- value pair into tree
	void Write(std::uint64_t key, std::uint64_t value) {
		InsertStruct rtc;
		auto && root = Block(_rootID);
		if (IsLeaf(root)) {
			rtc = InsertToLeaf(key, value, root);
		}
		else {
			rtc = InsertToInnerNode(key, value, root);
		}
		if (rtc.split) {
			// change root manually
			auto && newRoot = GetNewBlock(false);
			newRoot[0] = rtc.leftBlockID;
			newRoot[1] = rtc.pushedKey;
			newRoot[2] = rtc.rightBlockID;
			SetBlockSize(newRoot, 1);
			SaveNewBlock(newRoot);
			_rootID = GetBlockID(newRoot);
		}
	}

	// removes key and it's associated value from tree
	void Erase(std::uint64_t key) {
		DeleteStruct rtc;
		auto && root = Block(_rootID);
		if (IsLeaf(root)) {
			rtc = DeleteFromLeaf(key, root);
			if (rtc.merge) {
				// just remove key, root doesn't need merging
				size_t i = rtc.idx;
				for (; i < GetBlockSize(root); i++)
				{
					root[2 * i] = root[2 * i + 2];
					root[2 * i + 1] = root[2 * i + 3];
				}
				root[2 * i] = -1;
				root[2 * i + 1] = -1;

				SetBlockSize(root, GetBlockSize(root) - 1);
			}
		}
		else {
			rtc = DeleteFromInnerNode(key, root);
			if (rtc.merge) {
				// children merged, must remove key
				if (GetBlockSize(root) == 1) {
					// change root
					_rootID = root[0];
					RemoveBlock(root);
				}
				else {
					size_t i = rtc.idx;
					// just delete the key
					for (; i < GetBlockSize(root); i++)
					{
						root[2 * i + 1] = root[2 * i + 3];
						root[2 * i + 2] = root[2 * i + 4];
					}
					root[2 * i + 1] = -1;
					root[2 * i + 2] = -1;
					SetBlockSize(root, GetBlockSize(root) - 1);
					TouchBlock(root);
				}
			}
		}
	}	

	// Server starts listening for client requests
	void StartListening() {
		Reset(io.timestamp());
		std::list<IO::op*> ops;
		while (io.poll(ops, 0.1, true)) {
			auto k = ops.size();
			// fill task queue
			auto && r = NewRequest();
			if (!io.recv_request(&r) || !ConfirmNewRequest(r, io.timestamp())) {
				if (_requestCount > 0) {
					// make room in cache
					FreeCache();
					//	start processing
					Process();
					// write data
					WriteData();
					// answer requests
					Answer();					
				}
				// reset queue and start listening again
				Reset(io.timestamp());
			};			
		}
		// finish what's left in queue

		//	start processing
		Process();
		// write data
		WriteData();
		// answer requests
		Answer();
	}
};


#endif

