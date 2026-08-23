#include "Awl/Coro/JobPromise.h"
#include "Awl/Coro/Job.h"
#include "Awl/ILogger.h"
#include "Awl/StaticChain.h"
#include "Awl/StringFormat.h"
#include "Awl/Exception.h"

#include <memory>

namespace awl::coro::detail
{
    Job JobPromise::get_return_object()
    {
        return { std::coroutine_handle<JobPromise>::from_promise(*this) };
    }

    void JobPromise::unhandled_exception() noexcept
    {
        const awl::StaticLink<std::shared_ptr<awl::ILogger>>* p_link = awl::static_chain<std::shared_ptr<awl::ILogger>>().find("Application");

        if (p_link != nullptr)
        {
            const std::shared_ptr<awl::ILogger>& logger = p_link->value();

            awl::ostringstream out;

            out << "Unhandled exception in Job ";

            try
            {
                std::rethrow_exception(std::current_exception());
            }
            catch (const awl::Exception& e)
            {
                out << "of type '" << e.what() << "', Message: " << e.message();
            }
            catch (const std::exception& e)
            {
                out << "of type derived from std::exception, Message: '" << e.what();
            }

            logger->error(out.str());
        }

        std::terminate();
    }
}
