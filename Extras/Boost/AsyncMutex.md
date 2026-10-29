# Asio coroutine mutex

`BoostExtras/AsyncMutex.h` provides `awl::AsyncMutex` through `AWL::Boost`.

```cpp
awl::AsyncMutex mutex(executor);

// Inside a Boost.Asio awaitable:
auto lock = co_await mutex.asyncLock();
co_await asyncWork();
// Leaving scope releases the lock, including during exception unwinding.
```

The mutex uses one token in a bounded `awl::Channel`. Waiting suspends the
coroutine without blocking its thread. Supply an externally owned
`boost::asio::any_io_executor`; no threads, pools or strands are created.
Different coroutines may acquire/release on different executors. The mutex does
not move their execution to its executor or replace the protected model's strand.
Keep all participating executors running until operations finish.

`Lock` is movable, not copyable. Moving transfers ownership; move assignment
releases the destination's previous lock. `ownsLock()` reports ownership and
`unlock()` releases early (an empty/moved-from lock is harmless).
Locks and already-created lazy waits keep the shared channel alive even if the
AsyncMutex wrapper is destroyed. Do not access the wrapper concurrently with its
destruction. A Lock object itself must not be accessed concurrently.

Acquisition uses normal Asio cancellation. Cancellation of one waiting coroutine
does not close the channel, unlock the owner or cancel other waiters. After
acquisition the mutex does not alter cancellation policy: an operation cancelled
inside the critical section unwinds and releases its lock. Components that must
finish committed SQL/model updates must still disable cancellation explicitly.

The mutex is not recursive. Awaiting another acquisition while holding the same
mutex can deadlock. No strict FIFO scheduling guarantee is added. This is
in-process mutual exclusion, not a database transaction or crash recovery.

Tests live in `Extras/Boost/Tests/AsyncMutexTest.cpp`.
