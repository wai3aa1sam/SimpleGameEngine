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

#define SGE_IS_OLD_JOB_SYSTEM_TEST_CASE 0

#define SGE_IS_JOB_SYSTEM_DISPATCH_DEPEND 0

#if SGE_IS_OLD_JOB_SYSTEM_TEST_CASE

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


#else

class PrimeNumberSolver
{
public:
	static constexpr size_t s_kBatchSize	= 4;

	static constexpr size_t	s_kPrimeStart	= 1000000000LL;
	//static constexpr size_t	s_kPrimeStart	= 2;

	static constexpr size_t	s_kLoopCount    = 40;

private:
	static bool s_isPrimeNumber(i64 v)
	{
		for (i64 i = 2; i < v; i++)
		{
			if (v % i == 0)
				return false;
		}
		return true;
	}

public:
	
	template<class... ARGS>
	class SolverJobT
	{
	public:
		SolverJobT(size_t primeStart_)
		{
			auto primeStart = primeStart_;

			_result.resize(s_kLoopCount);

			_numbers.resize(s_kLoopCount);
			for (size_t i = primeStart; i < primeStart + s_kLoopCount; i++)
			{
				_numbers[i - primeStart] = i;
			}
		}

		void execute()
		{
			SGE_PROFILE_SCOPED;

			//SGE_LOG("=== i: {}", i);

			for (size_t i = 0; i < s_kLoopCount; i++)
			{
				bool isPrime = s_isPrimeNumber(_numbers[i]);
				_result[i] = isPrime;
			}
		}

		void execute(const JobArgs& args)
		{
			SGE_PROFILE_SCOPED;

			auto i = args.loopIndex;
			//SGE_LOG("=== i: {}", i);

			bool isPrime = s_isPrimeNumber(_numbers[i]);
			_result[i] = isPrime;
		}

		void execute_submit()
		{
			SGE_PROFILE_SCOPED;

			bool isPrime = s_isPrimeNumber(_numbers[i]);
			_result[i] = isPrime;
		}

		void print() const
		{
			SGE_LOG("=== result");
			size_t resultCount = 0;
			for (size_t i = 0; i < _result.size(); i++)
			{
				if (_result[i] == 1)
				{
					SGE_LOG("prime: {}", _numbers[i]);
					resultCount++;
				}
			}
			SGE_LOG("=== result count: {}", resultCount);
		}

	private:
		Vector<size_t> _numbers;
		Vector<int>   _result;
	};

	template<class... ARGS>
	class SolverJobT_testParFor : public JobParFor_Base<SolverJobT_testParFor<ARGS...>>, public SolverJobT<ARGS...>
	{
		using Base = SolverJobT<ARGS...>;
	public:
		SolverJobT_testParFor(size_t primeStart_)
			: Base(primeStart_)
		{
		}
	};

	template<class... ARGS>
	class SolverJobT_testFor : public JobFor_Base<SolverJobT_testFor<ARGS...>>, public SolverJobT<ARGS...>
	{
		using Base = SolverJobT<ARGS...>;
	public:
		SolverJobT_testFor(size_t primeStart_)
			: Base(primeStart_)
		{
		}
	};

	template<class... ARGS>
	class SolverJobT_test : public Job_Base<SolverJobT_test<ARGS...>>, public SolverJobT<ARGS...>
	{
		using Base = SolverJobT<ARGS...>;
	public:
		SolverJobT_test(size_t primeStart_)
			: Base(primeStart_)
		{
		}
	};

	using SolverJob_ParFor	= SolverJobT_testParFor<void>;
	using SolverJob_ParFor2 = SolverJobT_testParFor<void, void>;

	using SolverJob_For		= SolverJobT_testFor<void>;
	using SolverJob_For2	= SolverJobT_testFor<void, void>;

	using SolverJob			= SolverJobT_test<void>;
	using SolverJob2		= SolverJobT_test<void, void>;

	class Test_Dispatch
	{
	public:

		void test()
		{
			JobSystem _jsys;

			{
				SGE_PROFILE_SCOPED;

				SMutexProtected<int> a;
				//auto& data = a.scopedULock();
				auto data = a.scopedSLock(); (void)data;
				SGE_DUMP_VAR(*data);
				
				MutexProtected<int> aa;
				auto data2 = aa.scopedULock();
				SGE_DUMP_VAR(*data2);
				
				CondVarProtected<int> aaa;
				auto data3 = aaa.scopedULock();
				SGE_DUMP_VAR(*data3);

				struct test2
				{
					int a = 10;
				};
				
				SMtxCondVarProtected<int> aaaa;
				auto data4 = aaaa.scopedSLock();
				SGE_DUMP_VAR(*data4);
				
				auto handle = solverJob_ParFor.dispatch(s_kLoopCount, s_kBatchSize);
				//auto handle = solverJob_For.dispatch(s_kLoopCount);
				//auto handle = solverJob.dispatch();
				handle->waitForComplete();
			}
			
			solverJob_ParFor.print();
			solverJob_For.print();
			solverJob.print();
		}

	private:
		PrimeNumberSolver::SolverJob_ParFor	solverJob_ParFor{PrimeNumberSolver::s_kPrimeStart};
		PrimeNumberSolver::SolverJob_For	solverJob_For	{PrimeNumberSolver::s_kPrimeStart};
		PrimeNumberSolver::SolverJob		solverJob		{PrimeNumberSolver::s_kPrimeStart};
	};

private:

};

#endif // 0

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
			#if SGE_IS_JOB_SYSTEM_DISPATCH_DEPEND

			JobSystem _jsys;

			auto* jsys = JobSystem::instance();

			Job* handle0 = nullptr;
			Job* handle1 = nullptr;

			PrimeNumberSolver::SolverJob_ParFor		solverJob_ParFor0(PrimeNumberSolver::s_kPrimeStart);
			PrimeNumberSolver::SolverJob_ParFor2	solverJob_ParFor1(PrimeNumberSolver::s_kPrimeStart + PrimeNumberSolver::s_kLoopCount * 1);

			PrimeNumberSolver::SolverJob_For		solverJob_For0(PrimeNumberSolver::s_kPrimeStart);
			PrimeNumberSolver::SolverJob_For2		solverJob_For1(PrimeNumberSolver::s_kPrimeStart + PrimeNumberSolver::s_kLoopCount * 1);

			PrimeNumberSolver::SolverJob			solverJob0(PrimeNumberSolver::s_kPrimeStart);
			PrimeNumberSolver::SolverJob2			solverJob1(PrimeNumberSolver::s_kPrimeStart + PrimeNumberSolver::s_kLoopCount * 1);

			size_t startCount = 0;
			Job* empty = jsys->createEmptyJob(); (void)empty;

			#endif // 0

			for (;;)
			{
				poll();

				static bool isStart = false;
				static bool isDone = false;

				if (!isStart)
				{
					#if SGE_IS_OLD_JOB_SYSTEM_TEST_CASE
					//PrimeNumberSolver::test_JobSystem::test();
					//PrimeNumberSolver::test_parallel_for::test();
					//PrimeNumberSolver::test_dependent::test();

					#else

					#if SGE_IS_JOB_SYSTEM_DISPATCH_DEPEND

					{
						SGE_PROFILE_SCOPED;

						handle0 = solverJob_ParFor0.delayDispatch(PrimeNumberSolver::s_kLoopCount, PrimeNumberSolver::s_kBatchSize);
						handle1 = solverJob_ParFor1.delayDispatch(PrimeNumberSolver::s_kLoopCount, PrimeNumberSolver::s_kBatchSize, handle0, handle0, handle0);

						handle0 = solverJob_For0.delayDispatch(PrimeNumberSolver::s_kLoopCount);
						handle1 = solverJob_For1.delayDispatch(PrimeNumberSolver::s_kLoopCount, handle0, handle0, handle0);

						handle0 = solverJob0.delayDispatch();
						handle1 = solverJob1.delayDispatch(handle0, handle0, handle0);
					}

					#else

					isStart = true;
					isDone = true;

					SGE_TEST_CASE(PrimeNumberSolver::Test_Dispatch, test());

					#endif // SGE_IS_JOB_SYSTEM_DISPATCH_DEPEND

					#endif // SGE_IS_OLD_JOB_SYSTEM_TEST_CASE

				}
				#if SGE_IS_JOB_SYSTEM_DISPATCH_DEPEND

				if (startCount > 2)
				{
					SGE_PROFILE_SCOPED;

					jsys->submit(handle0);

					sleep_ms(500);

					jsys->waitForComplete(handle1);

					solverJob_ParFor0.print();
					solverJob_ParFor1.print();

					solverJob_For0.print();
					solverJob_For1.print();

					solverJob0.print();
					solverJob1.print();


					isDone = true;
				}

				SGE_LOG("waiting...");
				startCount++;
				sleep(1);

				#endif // SGE_IS_JOB_SYSTEM_DISPATCH_DEPEND

				SGE_PROFILE_FRAME;

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

}

void test_Multi_thread() {
	using namespace sge;

	//signal(SIGTERM, my_singal_handler); // kill process
	//signal(SIGINT,  my_singal_handler); // Ctrl + C

	atomicLog("=== sizeof(Job) {}", sizeof(Job));

	SGE_TEST_CASE(Test_Multi_thread, main());

	atomicLog("=== end main()");

	return;
}