#include "temp_Chunk.h"

#include <sge_core/log/Log.h>

namespace sge {

void Chunk::init(u64 nBytes_)
{
	_buffer.resize(nBytes_);

	//SGE_LOG("init! buffer size: {}", _buffer.size());

	memset(_buffer.data(), 0, _buffer.size());
	_offset = 0;
}

void* Chunk::allocate(u64 nBytes_)
{
	//SGE_ASSERT(_offset + nBytes_ < _buffer.capacity());
	/*if (_offset + nBytes_ > _buffer.capacity())
		return nullptr;*/

	if (_offset + nBytes_ > _buffer.size())
		return nullptr;

	u8* p = _buffer.data() + _offset;
	_offset += nBytes_;
	return p;
}

bool  Chunk::deallocate(u64 nBytes_)
{
	SGE_ASSERT(_offset - nBytes_ >= 0);
	if (_offset - nBytes_ >= 0)
	{
		_offset -= nBytes_;
		return true;
	}
	return false;
}

void Chunk::clear()
{
	//_buffer.clear();		// gg
	memset(_buffer.data(), 0, _buffer.size());
	_offset = 0;
}

void* Chunk::getData()
{
	return _buffer.data();
}

u64 Chunk::getRemainingByteSize()
{
	return _buffer.size() - getByteSize();
}

u64 Chunk::getByteSize()
{
	return (_offset == 0) ? 0 : _offset + 1;
}

u64 Chunk::getTotalByteSize()
{
	return  _buffer.size();
}

}