#include "Awl/Coro/UpdatePromise.h"
#include "Awl/Coro/UpdateTask.h"
#include "Awl/Logger.h"
#include "Awl/StaticChain.h"
#include "Awl/StringFormat.h"
#include "Awl/Exception.h"

using namespace awl;

UpdateTask UpdatePromise::get_return_object()
{
    return { std::coroutine_handle<UpdatePromise>::from_promise(*this) };
}

void UpdatePromise::unhandled_exception() noexcept
{
    const awl::StaticLink<awl::Logger&>* p_link = awl::static_chain<awl::Logger&>().find("Application");

    if (p_link != nullptr)
    {
        awl::Logger& logger = p_link->value();

        awl::ostringstream out;

        out << "Unhandled exception in UpdateTask ";

        try
        {
            std::rethrow_exception(std::current_exception());
        }
        catch (const awl::Exception& e)
        {
            out << "of type '" << e.what() << "', Message: " << e.What();
        }
        catch (const std::exception& e)
        {
            std::cout << "of type derived from std::exception, Message: '" << e.what();
        }

        logger.error(out.str());
    }

    std::terminate();
}
