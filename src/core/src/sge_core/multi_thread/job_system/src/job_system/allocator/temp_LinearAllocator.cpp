#include "temp_LinearAllocator.h"

namespace sge {
namespace temp
{


LinearAllocator::LinearAllocator(u64 nBytes)
{
	_init(nBytes);
}

void LinearAllocator::_init(u64 nBytes_)
{
	_chunkList.push_front();
	_chunkList.front().init(nBytes_);
	_itCurrentChunk = _chunkList.begin();
}

void* LinearAllocator::allocate(u64 nBytes_)
{
#if old_impl_has_bug

#endif // old_impl_has_bug

	SGE_ASSERT(!_chunkList.begin().mpNode, "not yet init");

	void* p = nullptr;

	p = _itCurrentChunk->allocate(nBytes_);

	//SGE_LOG("Current Chunk size: {}", _itCurrentChunk->getByteSize());

	if (!p)
	{
		//SGE_LOG("ChunkList Prev size: {}", _chunkList.size());

		auto prev_byteSize = _itCurrentChunk->getTotalByteSize();

		if (_itCurrentChunk.next() == _chunkList.end())
		{
			_chunkList.push_back();
			_chunkList.back().init(prev_byteSize);
		}

		//SGE_LOG("ChunkList Current size: {}", _chunkList.size());
		SGE_ASSERT(nBytes_ <= prev_byteSize);

		_itCurrentChunk = _itCurrentChunk.next();
		p = _itCurrentChunk->allocate(nBytes_);
		return p;
	}
	return p;

}

bool LinearAllocator::deallocate(u64 nBytes_)
{
#if old_impl_has_bug
	auto itBack = _chunkList.end().prev();
	bool isSuccessful = false;

	for (auto it = _chunkList.rend(); it != _chunkList.rbegin(); it++)
	{
		isSuccessful = it->deallocate(nBytes_);
		if (isSuccessful)
			return true;
	}
	return false;
#endif // old_impl_has_bug

	bool isSuccessful = false;
	isSuccessful = _itCurrentChunk->deallocate(nBytes_);
	if (!isSuccessful)
	{
		_itCurrentChunk = _itCurrentChunk.prev();
	}
	return isSuccessful;
}

void LinearAllocator::clear()
{
	for (auto it = _chunkList.begin(); it != _chunkList.end(); it++)
	{
		it->clear();
	}
	_itCurrentChunk = _chunkList.begin();
}

u64 LinearAllocator::getChunkCount()
{
	return _chunkList.size();
}

u64 LinearAllocator::getTotalByteSize()
{
	u64 totalByteSize = 0;
	for (auto it = _chunkList.begin(); it != _chunkList.end(); it++)
	{
		totalByteSize += it->getTotalByteSize();
	}
	return totalByteSize;
}

List<Chunk>& LinearAllocator::getChunkList()
{
	return _chunkList;
}

const List<Chunk>& LinearAllocator::getChunkList() const
{
	return _chunkList;
}
List<Chunk>::iterator& LinearAllocator::getCurrentIterator()				{ return _itCurrentChunk; }
const List<Chunk>::iterator& LinearAllocator::getCurrentIterator() const	{ return _itCurrentChunk; }


}

}