#include <sge_core.h>

#include <sge_core/multi_thread/job_system/src/job_system.h>

#include <iostream>
#include <conio.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <string>
#include <mutex>
#include <execution>

namespace sge {

bool s_isQuit = false;

class PrimeNumberSolver
{
	static constexpr size_t s_kJobCount		= 16 * 4;

public:
	struct Request
	{
		size_t id		= 0;
		u64 current		= 0;
		u64 start		= 0;
		u64 count		= 0;

		PrimeNumberSolver* solver = nullptr;
	};
	
	struct Result
	{
		Vector<u64> _buffer;
	};

	static bool s_isPrimeNumber(i64 v)
	{
		for (i64 i = 2; i < v; i++)
		{
			if (v % i == 0)
				return false;
		}
		return true;
	}

	bool isFinished() const { return _finishCounter.load() == s_kJobCount; }

	void print()
	{
		{
			MyTimer timer;

			while (_finishCounter.load() != s_kJobCount)
			{
				//atomicLog("waiting finish");
			}
		}
		auto res = _result.scopedLock();

		atomicLog("========= finished");
		atomicLog("========= result count: {}", res->_buffer.size());

		for (auto& e : res->_buffer)
		{
			atomicLog("res: {}", e);
		}
		atomicLog("=== end print()");

		//atomicLog("========= jobRemainCount: {}", _mainJob->jobRemainCount());
	}

	void combineResult(const Span<u64>& result)
	{
		if (!result.size())
		{
			_finishCounter++;
			return;
		}
		auto res = _result.scopedLock();
		res->_buffer.appendRange(result);

		_finishCounter++;
	}
public:

	struct test_JobSystem
	{
		static constexpr size_t s_kBatchSize	= 1;
		static constexpr u64	s_kPrimeStart	= 1000000000LL;
		//static constexpr u64	s_kPrimeStart	= 2;

		struct Data
		{
			struct Storage
			{
				Request req;
			};

			Data(PrimeNumberSolver& solver)
			{
				storage = new Storage[s_kJobCount];
				::memset(storage, 0, sizeof(storage) * s_kJobCount);
				for (size_t i = 0; i < s_kJobCount; i++)
				{
					auto& req	= storage[i].req;
					req.id		= i;
					req.start	= s_kPrimeStart + i * s_kBatchSize;
					req.current = req.start;
					req.count	= s_kBatchSize;
					req.solver	= &solver;
				}
			}
			~Data()
			{
				delete storage;
			}
			Storage* storage = nullptr;
		};
		using Storage = Data::Storage;

		static void test()
		{
			PrimeNumberSolver solver0;  Data data0{ solver0 }; { JobSystem jsys; test_sge_JobSystem(data0.storage); }
		}

		static void test_sge_JobSystem(Storage* storage)
		{
			Job* mainJob = nullptr; (void)mainJob;

			auto mainTask = [](void* param)
			{
				auto* solver = static_cast<PrimeNumberSolver*>(param);
				solver->print();
			};

			auto task = [](void* param)
			{
				Request* req = static_cast<Request*>(param);
				auto index = req->id; (void)index;

				MyRequest myReq(req);
				req->solver->combineResult(myReq._result._buffer);
			};

#if 0
			mainJob = JobSystem::createJob(mainTask, storage->req.solver);
			JobSystem::submit(mainJob);

			for (size_t i = 0; i < s_kJobCount; i++)
			{
				auto* job = JobSystem::createSubJob(mainJob, task, &storage[i]);
				JobSystem::submit(job);
			}
			JobSystem::waitForComplete(mainJob);

#else
			auto* job = JobSystem::createAndRunNJobs(task, storage, s_kJobCount);
			JobSystem::waitForComplete(job);
#endif // 0

			storage->req.solver->print();

			DependencyManager::printRunAfter();
			DependencyManager::printRunBefore();
		}

	};

	struct test_parallel_for
	{
		static constexpr size_t s_kCount = 100000;
		static constexpr size_t s_kStart = 100000;
		static constexpr size_t s_kBatch = 10;

		struct Data
		{
			struct Storage
			{
				Request req;
				size_t count = 0;
			};

			Data()
			{
				storage = new Storage[s_kCount];
				::memset(storage, 0, sizeof(storage) * s_kCount);
				for (size_t i = 0; i < s_kCount; i++)
				{
					storage[i].req.id = 0;
					storage[i].req.current = 0;
					storage[i].req.start = s_kStart;
					storage[i].req.count = s_kBatch;
				}
			}
			~Data()
			{
				delete storage;
			}
			Storage* storage = nullptr;
		};
		using Storage = Data::Storage;

		static void update(void* param, size_t size)
		{
			auto* storage = static_cast<Storage*>(param); (void)storage;
			for (size_t i = 0; i < size; i++, storage++)
			{
				update_impl(*storage);
			}
		}

		static void update_impl(Storage& storage)
		{
			Request& val = storage.req;
			//val = test_val;
			for (size_t startIndex = val.start; startIndex < val.start + val.count; startIndex++)
			{
				if (PrimeNumberSolver::s_isPrimeNumber(startIndex))
				{
					val.current += startIndex;
				}
			}
		}

		static void test()
		{
			Data data0; { JobSystem jsys;	test_sge_JobSystem	(data0.storage); }
			Data data1; {					test_std_par_for	(data1.storage); }

			check(data0.storage, data1.storage);
		}

		static void test_sge_JobSystem(Storage* storage)
		{
			{
				MyTimer timer;
				auto* job = JobSystem::parallel_for(storage, s_kCount, test_parallel_for::update);

				JobSystem::submit(job);
				JobSystem::waitForComplete(job);

				DependencyManager::printRunAfter();
				DependencyManager::printRunBefore();
			}
		}

		static void test_std_par_for(Storage* storage)
		{
			{
				MyTimer timer;
				auto request_task = std::async(std::launch::async,
					[&]() {
						std::for_each(std::execution::par,
							storage,
							storage + s_kCount,
							[](Storage& storage)
							{
								update_impl(storage);
							}
						);
					});
				request_task.wait();
			}
		}

		static void check(Storage * storage0, Storage * storage1)
		{
			for (size_t i = 0; i < s_kCount; i++)
			{
				if (storage0[i].req.current != storage1[i].req.current)
				{
					SGE_LOG("incorrect");
					return;
				}
			}
		}

	};

	struct test_dependent
	{
		static void test()
		{

			JobSystem jsys;

			auto task = [](void* param)
			{
				SGE_LOG((char*)param);
			};

#define JOB(x) auto* job ## x = JobSystem::createJob(task, "Task " #x)
#define JOB_NO_DECL(x) JobSystem::createJob(task, "Task " #x)->setName("Task " #x)

			{
				MyTimer timer;

#if 1
				DependencyManager::beginCapture();

				JobFlow jf;
				auto [A, B, C, D, E, F] = jf.emplace(
					JOB_NO_DECL(A), 
					JOB_NO_DECL(B),
					JOB_NO_DECL(C),
					JOB_NO_DECL(D),
					JOB_NO_DECL(E),
					JOB_NO_DECL(F));

				A->runAfter(B, F);
				C->runBefore(B);
				D->runBefore(C);
				DependencyManager::endCapture();

				//A->runAfter(B, F);

				jf.runAndWait();

				DependencyManager::printRunAfter();
				DependencyManager::printRunBefore();
				
#else
				JOB(A);
				JOB(B);
				JOB(C);
				JOB(D);
				JOB(E);
				JOB(F);

				jobD->addRunBefore(jobC);
				jobE->addRunBefore(jobC);
				jobA->addRunBefore(jobB);

				jobC->addRunBefore(jobB);
				jobF->addRunBefore(jobA);

				JobSystem::submit(jobF);
				JobSystem::submit(jobD);
				JobSystem::submit(jobE);
				JobSystem::waitForComplete(jobA);   
#endif // 0

			}
#undef JOB
#undef JOB_NO_DECL
		}
	};
private:
	class MyRequest
	{
		friend class PrimeNumberSolver;
	public:
		MyRequest(Request* request)
			:
			_request(request)
		{
			auto* req = request;

			//atomicLog("Task id : {}, start: {}, count: {}", id, start, count);

			for (size_t i = 0; i < req->count; i++)
			{
				auto v = getNext();
				if (v == 0)
					break;
				if (s_isPrimeNumber(v))
				{
					addResult(v);
				}
			}
		}

		void addResult(u64 v)
		{
			auto* res = &_result;
			res->_buffer.emplace_back(v);
			//atomicLog("Task id: {}, added result: {}", id, v);
		}

		u64 getNext()
		{
			auto* req = _request;
			
			if (req->current >= req->start + req->count)
				return 0;

			auto next = req->current;
			req->current++;
			return next;
		}

	private:
		Request* _request = nullptr;
		Result   _result;
	};

private:
	MutexProtected<Result> _result;
	Atomic<size_t> _finishCounter = 0;
};

class Test_Multi_thread
{
public:
	Test_Multi_thread()
	{
		atomicLog("============= Test_Multi_thread(), start thread pool");
	}
	~Test_Multi_thread()
	{
		atomicLog("============= ~Test_Multi_thread(), end thread pool");
	}

	void main()
	{
		SGE_DUMP_VAR(hardwareThreadCount());
		atomicLog("start thread pool");
		{
			for (;;)
			{
				poll();

				static bool isStart = false;
				static bool isDone = false;

				if (!isStart)
				{
					
					//PrimeNumberSolver::test_JobSystem::test();
					//PrimeNumberSolver::test_parallel_for::test();
					PrimeNumberSolver::test_dependent::test();
					isStart = true;
					isDone = true;
				}

				//primeNumberSolver.print();


				if (s_isQuit || isDone)
				{
					atomicLog("=== try quit");
					//threadPool.terminate();
					return;
				}
			}
		}
	}

	void poll()
	{
		if (_kbhit()) {
			int ch = 0;
			// Stores the pressed key in ch
			ch = _getch();
			s_isQuit = true;
		}
	}

private:
};

class Test_Data
{
public:
	static constexpr int kCount = 4;
	Test_Data()
	{
		::memset(_test_data, 0, sizeof(int) * kCount);

		intptr_t p_val = reinterpret_cast<intptr_t>(_test_data); (void)p_val;
		{
			int* p = _test_data; (void)p;
			*p = 1;
			p++;
			*p = 2;
		}


		SGE_DUMP_VAR(p_val);
		//SGE_DUMP_VAR(*p);

	}
	int _test_data[kCount];
	int _test_raw1 = 0;
	int _test_raw2 = 0;
	int _test_raw3 = 0;
	int _test_raw4 = 0;

	int _test_data1[1];
	int _test_data2[1];
	int _test_data3[1];
	int _test_data4[1];
};

}


void test_Multi_thread() {
	using namespace sge;

	//signal(SIGTERM, my_singal_handler); // kill process
	//signal(SIGINT,  my_singal_handler); // Ctrl + C

	atomicLog("=== sizeof(Job) {}", sizeof(Job));

#if 1
	SGE_TEST_CASE(Test_Multi_thread, main());

#else


#endif // 0

	atomicLog("=== end main()");

	return;
}