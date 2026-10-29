# Process shutdown signals

`AWL::Boost` provides `BoostExtras/ShutdownSignals.h`:

```cpp
awl::ShutdownSignals signals(executor);
const awl::ShutdownSignal signal = co_await signals.asyncWait();
```

The executor must wrap a caller-owned strand. Register the object before signals
can arrive; perform waits, cancellation and destruction on that strand (or while
its context is idle). The helper creates no threads, pools or strands. It does
not request application shutdown or log messages itself.

SIGINT maps to `ShutdownSignal::Interrupt`, SIGTERM to `Terminate`, and Windows
SIGBREAK to `Break`. `Break` remains a portable enum value, but its native signal
is registered only on Windows. Include `BoostExtras/ShutdownSignalFormatter.h`
for narrow/wide `std::format` and AWL lazy logging with the names
`Ctrl+C (SIGINT)`, `SIGTERM` and `Ctrl+Break (SIGBREAK)`.

Asio errors propagate through the awaitable. Both coroutine cancellation and
`cancel()` abort active waits with `operation_aborted`. Cancellation is a wakeup,
not a shutdown signal. A later wait can reuse the object; cancel is not a sticky
stop flag. Normal Asio signal queuing rules apply, including signals received
after registration but before waiting. Do not mix these registrations with
unrelated native handlers for the same process signals.

The object must outlive the coroutines using it. On EOF, RPC stop, timeout or other
shutdown paths, cancel and join its wait before destroying the object or stopping
the executor; otherwise an outstanding signal wait keeps the event loop alive.
The application remains responsible for logging, stop_source, error reporting and
joining its other tasks.

Tests are in `Extras/Boost/Tests/ShutdownSignalsTest.cpp`. They raise process
signals in the test executable; do not run them concurrently in that process
with other tests that manipulate native signal handlers. Windows Ctrl+Break is
also exercised by TradeClient's separate-process MCP signal integration test.
