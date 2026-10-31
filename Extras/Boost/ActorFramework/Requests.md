# Requests between queued components

`BoostExtras/ActorFramework/Request.h` provides `awl::asyncRequest` and
`awl::RequestCompletion<T>` through `AWL::Boost`. Components keep their own bounded
`awl::Channel` inboxes and explicit consumer coroutines. Each request carries data
and a reply address backed by `boost::asio::any_completion_handler`, not another
channel or a numeric request ID.

```cpp
struct MarketRequest
{
    using Result = MarketState;

    std::string symbol;
    awl::RequestCompletion<Result> completion;
};

using Message = std::variant<MarketRequest, StopMessage>;
using Inbox = awl::Channel<void(boost::system::error_code, Message)>;

// Inside the requesting component's current message handler:
const MarketState market =
    co_await awl::asyncRequest(model_inbox, MarketRequest{"XRPFDUSD", {}});

// Inside the model's message handler, on its executor:
request.completion.complete(readMarketSnapshot(request.symbol));

// On a request failure:
request.completion.fail(std::current_exception());
```

A component can hide the message behind an ordinary method returning an awaitable:

```cpp
boost::asio::awaitable<MarketState> asyncGetMarket(std::string symbol)
{
    return awl::asyncRequest(_inbox, MarketRequest{std::move(symbol), {}});
}
```

The request must expose `Result` and a default-initialized
`RequestCompletion<Result> completion` member. Its inbox must accept the request
directly or as an alternative of its message variant. Results are owned values;
move-only requests/results and `void` results are supported. To acknowledge a
`void` request, call `completion.complete({})`.

## Execution and lifetime

`asyncRequest` owns its inbox and request in the coroutine frame and is lazy until
awaited/spawned. It suspends while the bounded inbox accepts the message, then
until the receiver supplies its answer. It uses `async_initiate` with
`use_awaitable`; no threads, pools or strands are created.

The answer is posted to the requesting coroutine's executor. It does not enter
the requesting component's inbox, so a sequential handler can await another
component without dispatching its own next message. Cyclic waits (including a
request to the same single-consumer component) can still deadlock.

Each reply state holds work guards for the caller and inbox executors until the
answer has been posted. Associated handler allocation is preserved. A reply
address is copyable; its completion handler remains unique. Exactly one
completion wins even if two threads race; subsequent `complete`/`fail` calls
return false. An initialized reply address must be used; the default empty value
is a placeholder populated by `asyncRequest`.

Destroying the last unanswered reply address posts `operation_aborted` to its
caller. A closed inbox reports its channel error; request exceptions are
re-thrown unchanged in the awaiting coroutine. `fail` requires a non-null
`exception_ptr`. Replies must not contain references to mutable receiver state.

## Cancellation and shutdown

Default Asio cancellation checks apply before initiation. Once initiated, a
request does not install a per-operation cancellation handler: it waits for
admission and an answer rather than abandoning work in the middle. A later
cancellation request remains in the caller's cancellation state; default checks
apply again at its next await. This policy does not roll back mutations.

The receiver owns its shutdown policy. Stop accepting new work and drain or fail
queued/deferred requests before stopping the consumer. Merely stopping the
consumer while retaining requests in its inbox does not complete those requests.
Keep both executors running until replies and consumer jobs finish. Releasing a
reply address during executor destruction is too late for orderly shutdown.

`RequestCompletion` is transport plumbing, not an actor runtime: it does not
serialize receiver state, supervise consumers, retry requests or create a durable
event log. The component's own consumer provides sequential processing and must
define which admitted operations finish without cancellation.

`Extras/Boost/Tests/ActorFramework/RequestTest.cpp` contains two small components (`MarketModel`
and `MarketBot`) with separate variant inboxes. Tests cover a shared strand,
different strands on multiple threads, concurrent callers, lazy initiation,
snapshot values, exception propagation, duplicate/abandoned replies, a closed
inbox, move-only payloads, void replies, admitted cancellation, and independent
caller/receiver execution contexts.
