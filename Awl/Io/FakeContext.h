#pragma once

namespace awl::io
{
    template <class Stream>
    class ReadContext
    {
    public:

        template<class Struct>
        void ReadV(Stream & s, Struct & val) const
        {
            static_cast<void>(s);
            static_cast<void>(val);
        }
    };

    template <class Stream>
    class WriteContext
    {
    public:
        
        template<class Struct>
        void WriteV(Stream & s, const Struct & val) const
        {
            static_cast<void>(s);
            static_cast<void>(val);
        }
    };
}
