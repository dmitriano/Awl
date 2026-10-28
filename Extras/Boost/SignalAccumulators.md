# Asio signal accumulators

Include `BoostExtras/SignalAccumulator.h` or `BoostExtras/SignalRangeAccumulator.h`.
The types live in `awl`, and `asyncWait()` returns a Boost.Asio awaitable. The old
`Awl/Coro` accumulator headers and `awl::coro` accumulator types have been removed.

```cpp
awl::SignalAccumulator<int> values(executor, signal);
int value = co_await values.asyncWait();

auto changes = awl::accumulate_signal<&Order::changed>(executor, orders);
auto [sender, filled, cost] = co_await changes.asyncWait();
```

Supply a caller-owned strand executor. Construct/destroy accumulators, deliver signals,
mutate observable sets, and consume results on that same strand. These types create no
threads, pools or strands. Signals and observable sets must outlive their accumulator;
the range accumulator retains shared handles to its current objects and queued senders.

Every event is retained in FIFO order, including emissions before waiting and during
consumer processing. Arguments are stored by decayed value. A single argument returns
that value; multiple arguments return a tuple; a no-argument signal returns void.
Range events prepend the sender (a shared pointer); a no-argument range event returns
only the sender. Observable sets subscribe/unsubscribe on insertion/removal/clearing.
Already queued events survive removal of their sender from the set.

One active wait per accumulator is supported; another concurrent wait throws logic_error.
Asio cancellation aborts the wait without consuming queued events or removing signal
subscriptions. A fresh coroutine can resume consumption. Destruction removes subscriptions,
discards pending events and wakes outstanding waits with operation_aborted; returned lazy
awaitables also remain safe to await after accumulator destruction, while executors live.

The FIFO is unbounded: synchronous signal callbacks cannot await backpressure. Consumers
must keep up or use a separate coalescing notification mechanism when payload history
is unnecessary. Timer cancellation is only the wake mechanism; no signal coroutine is
resumed inline. No persistent storage or cross-process recovery is provided.

Tests are in `Extras/Boost/Tests/SignalAccumulatorTest.cpp` and run with AWL Boost tests.

## One-shot signal waits

Include `BoostExtras/SignalAwaitable.h` and use
`co_await awl::asyncWaitSignal(executor, signal)`. This returns a Boost.Asio
awaitable with the same value/tuple/void result rules as SignalAccumulator.
The old `Awl/Coro/SignalAwaitable.h`, `SignalAwaitable` class and
`awl::coro::wait_signal` helper have been removed without compatibility wrappers.

A one-shot wait subscribes only when awaited. Earlier emissions are ignored.
It unsubscribes on the first signal, before the awaiting coroutine resumes;
subsequent emissions cannot replace the result. Cancellation also unsubscribes.
Independent waits have independent subscriptions/cancellation. As with accumulators,
the signal must outlive the operation and delivery/subscription lifetime must stay
on the supplied strand. Tests live in `Extras/Boost/Tests/SignalAwaitableTest.cpp`.
