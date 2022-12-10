#pragma once

namespace sge {

template<size_t COUNT = 256>
class CountBatcher
{
public:
	static constexpr size_t s_kValue = COUNT;

	template<class T>
	static inline bool shouldBatch(size_t count)
	{
		return count > COUNT;
	}
};

template<size_t BYTE = 32 * 1024> // L1 cache
class ByteBatcher
{
public:
	static constexpr size_t s_kValue = BYTE;

	template<class T>
	static inline bool shouldBatch(size_t count)
	{
		return sizeof(T) * count > BYTE;
	}
};

class _parallel_for_impl
{
public:
	template<class T>
	struct Data
	{
		using Task = void (*)(void*, size_t);
		Data() = default;
		Data(Job* parent, T* data, size_t count, Task task)
			:
			parent(parent), data(data), count(count), task(task)
		{
		}
		Job* parent = nullptr;
		T* data = nullptr;
		size_t count = 0;
		Task task;
	};
	template<class T> using Task = typename Data<T>::Task;

	template<class T, class BATCHER>
	static void invoke(void* par_data)
	{
		auto* jsys = JobSystem::instance();
		jsys->_checkError();
		//SGE_ASSERT(data_->task);

		Data<T>* data = static_cast<Data<T>*>(par_data);

		T* beg = data->data; (void)beg;
		T* end = data->data + data->count; (void)end;

		//atomicLog("BATCHER::s_kValue {}", BATCHER::s_kValue);

		if (BATCHER::shouldBatch<T>(data->count))
		{
			size_t total_Count = data->count;

			// split to left part
			size_t left_count = total_Count / 2;
			if (left_count)
			{
				Job* left_job = jsys->allocateJob();

				auto* left_buf = left_job->_allocate(sizeof(Data<T>));

				// data::parent change from left_job to data->parent (the root)
				auto* left_data = new(left_buf) Data<T>(data->parent, data->data, left_count, data->task); 

				left_job->init(_parallel_for_impl::invoke<T, BATCHER>, left_data, data->parent);
				jsys->submit(left_job);
			}

			SGE_ASSERT((data->data >= beg && data->data < end) && (data->data + left_count >= beg && data->data + left_count < end));
			/*if (!(data->data >= beg && data->data < end) || !(data->data + left_count >= beg && data->data + left_count < end))
			{
			atomicLog("================ _parallel_for_impl out of bound");
			}*/

			// split to right part
			size_t right_count = total_Count - left_count;
			if (right_count)
			{
				Job* right_job = jsys->allocateJob();

				auto* right_buf = right_job->_allocate(sizeof(Data<T>));

				// data::parent change from right_job to data->parent (the root)
				auto* right_data = new(right_buf) Data<T>(data->parent, data->data + left_count, right_count, data->task);

				right_job->init(_parallel_for_impl::invoke<T, BATCHER>, right_data, data->parent);
				jsys->submit(right_job);
			}

		}
		else
		{
			data->task(data->data, data->count);
		}
	}
};


}