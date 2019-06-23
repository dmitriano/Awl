#pragma once

#include "Awl/Stringizable.h"

#include <vector>
#include <assert.h>

namespace awl
{
    class Prototype
    {
    public:

        virtual const awl::helpers::MemberList & GetMemberNames() const = 0;

        virtual std::vector<size_t> GetMemberTypes() const = 0;
    };

    //V is std::variant, S is a Stringizable
    template <class V, class S>
    class AttachedPrototype : public Prototype
    {
    public:

        const awl::helpers::MemberList & GetMemberNames() const override
        {
            return S::get_member_names();
        }

        std::vector<size_t> GetMemberTypes() const override
        {
            auto a = map_types_t2v<decltype(S{}.as_tuple()), V>();
            return std::vector<size_t>(a.begin(), a.end());
        }
    };

    class DetachedPrototype : public Prototype
    {
    public:

        explicit DetachedPrototype(const Prototype & ap) : 
            memberNames(ap.GetMemberNames()), memberTypes(ap.GetMemberTypes())
        {
        }

        const awl::helpers::MemberList & GetMemberNames() const override
        {
            return memberNames;
        }

        std::vector<size_t> GetMemberTypes() const override
        {
            return memberTypes;
        }

        AWL_SERIALIZABLE(memberNames, memberTypes)

    private:

        helpers::MemberList memberNames;
        std::vector<size_t> memberTypes;
    };
}
