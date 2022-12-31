#pragma once

#include "temp_Chunk.h"
#include "EASTL/list.h"
namespace sge {

template<class T> using List = eastl::list<T>;

namespace temp {

class LinearAllocator : public NonCopyable
{
public:
	LinearAllocator() = default;
	LinearAllocator(u64 nBytes_);
	~LinearAllocator() = default;

	void init(u64 nBytes_ = 4 * 1024) { _init(nBytes_); }

	void* allocate(u64 nBytes_);
	bool deallocate(u64 nBytes_);
	void clear();
	template<class T> void destruct();
	template<class T> void destructAndClear();

	u64 getChunkCount();
	u64 getTotalByteSize();

	List<Chunk>& getChunkList();
	const List<Chunk>& getChunkList() const;

	List<Chunk>::iterator& getCurrentIterator();
	const List<Chunk>::iterator& getCurrentIterator() const;
protected:
	void _init(u64 nBytes_ = 4 * 1024);

private:
	List<Chunk> _chunkList;
	List<Chunk>::iterator _itCurrentChunk;			// it only work for list as it do not resize

	//bool isCleared = false;
};

template<class T> inline 
void LinearAllocator::destruct()
{
	for (auto& chunk : _chunkList)
	{
		chunk.destruct<T>();
	}
}

template<class T> inline 
void LinearAllocator::destructAndClear()
{
	for (auto& chunk : _chunkList)
	{
		chunk.destructAndClear<T>();
	}
}

}



}