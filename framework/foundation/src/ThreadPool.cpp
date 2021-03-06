#include <bdn/init.h>
#include <bdn/ThreadPool.h>

#include <bdn/entry.h>

#if BDN_HAVE_THREADS

namespace bdn
{

    ThreadPool::ThreadPool(int minThreadCount, int maxThreadCount)
        : _minThreadCount(minThreadCount), _maxThreadCount(maxThreadCount)
    {
        if (_minThreadCount < 0)
            throw InvalidArgumentError("ThreadPool constructor parameter minThreadCount must be >=0");

        if (_maxThreadCount <= 0)
            throw InvalidArgumentError("ThreadPool constructor parameter maxThreadCount must be >0");

        if (_maxThreadCount < _minThreadCount)
            throw InvalidArgumentError("ThreadPool constructor parameter maxThreadCount must be "
                                       ">=minThreadCount");
    }

    ThreadPool::~ThreadPool()
    {
        Mutex::Lock lock(_mutex);

        // signal the idle runners to stop.
        // They are currently waiting for new jobs - this will make them wake up
        // and end. Thus their thread will end as well.
        for (auto &runner : _idleRunners)
            runner->signalStop();

        // release the idle runners. Note that the runner object will remain
        // alive until their thread has actually finished.
        _idleRunners.clear();

        // should we stop busy runners or detach them (i.e. let them finish
        // their current job)? We have to consider two cases: 1) the thread pool
        // is deleted when the program exits. 2) the thread pool is deleted at
        // some other time In the case of 1 then it does not really matter.
        // There is a strong likelihood that the app exits before the runner
        // finishes anyway, whether we signal stop or not. In the case of 2 the
        // choice is basically between having the job finish normally and having
        // it be aborted. Since the pool is deleted then it is likely that the
        // caller wants the job to be aborted. If the job is not intended to be
        // aborted then the job could simply keep a reference to the pool and
        // keep it alive until it is finished. So the correct action here is to
        // abort.

        // stop busy runners
        for (auto &runner : _busyRunners)
            runner->signalStop();

        // and then release them. The runner object will remain alive until the
        // abort is complete and their thread exits.
        _busyRunners.clear();
    }

    bool ThreadPool::runnerFinishedJob(PoolRunner *runner)
    {
        Mutex::Lock lock(_mutex);

        if (!_queuedJobs.empty()) {
            P<IThreadRunnable> job = _queuedJobs.front();
            _queuedJobs.pop_front();

            // give the runner a new job right away
            runner->startJob(job);

            // Note that the runner remains in _busyRunners

            // runner should continue
            return true;
        } else {
            // we do not currently have anything queued.

            _busyRunners.erase(runner);

            if (_busyRunners.size() >= (size_t)_minThreadCount) {
                // we have more threads than necessary. Let this one die.
                return false;
            } else {
                // add the runner to the idle list and let it go to sleep
                _idleRunners.push_back(runner);

                // runner should not end, but wait for the next job
                return true;
            }
        }
    }

    // PoolRunner

    void ThreadPool::addJob(IThreadRunnable *runnable)
    {
        Mutex::Lock lock(_mutex);

        if (_idleRunners.empty()) {
            // we have no idle runner waiting.

            if (_busyRunners.size() >= (size_t)_maxThreadCount) {
                // we cannot start a new thread. Add the job to the queue
                _queuedJobs.push_back(runnable);
            } else {
                // start another thread.
                P<PoolRunner> runner = newObj<PoolRunner>(this);

                runner->startJob(runnable);

                _busyRunners.insert(runner);

                try {
                    P<Thread> thread = newObj<Thread>(runner);
                    thread->detach();
                }
                catch (...) {
                    // if there is an error starting the thread then we remove
                    // the runner again.
                    _busyRunners.erase(runner);
                }
            }
        } else {
            // we have an idle runner waiting. Give it a new job
            P<PoolRunner> runner = _idleRunners.front();
            _idleRunners.pop_front();

            _busyRunners.insert(runner);

            runner->startJob(runnable);
        }
    }

    int ThreadPool::getBusyThreadCount() const
    {
        Mutex::Lock lock(_mutex);

        return (int)_busyRunners.size();
    }

    int ThreadPool::getIdleThreadCount() const
    {
        Mutex::Lock lock(_mutex);

        return (int)_idleRunners.size();
    }

    void ThreadPool::PoolRunner::signalStop()
    {
        // this is called when the thread pool shuts down.

        {
            Mutex::Lock lock(_mutex);

            _shouldStop = true;

            // signal the active job to abort (if we have one).
            if (_job != nullptr)
                _job->signalStop();
        }

        // make sure that we wake up if we are waiting for a new job
        _wakeSignal.set();
    }

    void ThreadPool::PoolRunner::startJob(IThreadRunnable *job)
    {
        Mutex::Lock lock(_mutex);

        if (_job != nullptr)
            throw ProgrammingError("ThreadPool::PoolRunnable::startJob was "
                                   "called while the thread was still busy.");

        _job = job;

        _wakeSignal.set();
    }

    void ThreadPool::PoolRunner::run()
    {
        while (true) {
            _wakeSignal.wait();

            {
                Mutex::Lock lock(_mutex);

                _wakeSignal.clear();

                if (_shouldStop)
                    break;

                if (_job == nullptr) {
                    // this should never happen (note that Signals have no
                    // sporadic wake ups).
                    programmingError("ThreadPool PoolRunner was woken up, but it has no job "
                                     "and was also not asked to stop.");
                }
            }

            try {
                _job->run();
            }
            catch (...) {
                // we treat this just like a top-level exception that happens
                // in a normal Thread. So we cann unhandledException.
                if (!bdn::unhandledException(true))
                    std::terminate();

                // ignore exception and continue.
            }

            try {
                _job = nullptr;
            }
            catch (...) {
                // exception in runnable destructor.

                if (!bdn::unhandledException(true))
                    std::terminate();

                // continue and do not delete the runnable object
                _job.detachPtr();
            }

            P<ThreadPool> pool = _poolWeak.toStrong();
            if (pool == nullptr)
                break; // pool has been deleted

            if (!pool->runnerFinishedJob(this)) {
                // the pool wants us to end to reduce the total number of
                // threads.
                break;
            }
        }
    }
}

#endif
