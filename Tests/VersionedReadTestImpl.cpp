#include "Awl/Io/SequentialStream.h"
#include <memory>

namespace VtsTest
{
    class FakeStream : public awl::io::SequentialOutputStream
    {
    public:

        void Write(const uint8_t * buffer, size_t count) override
        {
            static_cast<void>(buffer);
            static_cast<void>(count);
        }
    };

    std::unique_ptr<awl::io::SequentialOutputStream> CreateFakeStream()
    {
        return std::unique_ptr<awl::io::SequentialOutputStream>(new FakeStream());
    }
}
