/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <Awl/Testing/UnitTest.h>

#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <string>
#include <concepts>

#include <memory>
#include <stdexcept>

namespace
{
    struct Device
    {
        std::string Version;
        int InventoryNumber;
        std::vector<std::shared_ptr<Device>> Children;
    };

    template <class Getter>
        requires std::invocable<Getter, const Device&>
    class Finder
    {
    public:

        // std::invoke_result_t is std::string& so we need std::decay_t.
        using Field = std::decay_t<std::invoke_result_t<Getter, const Device&>>;

        Finder(Getter func) : m_func(std::move(func)) {}

        std::vector<Field> FindDistinctValues(std::shared_ptr<Device> rootDevice)
        {
            FindDistinctValuesImpl(rootDevice);

            return MoveElements();
        }

    private:

        void FindDistinctValuesImpl(const std::shared_ptr<Device>& parentDevice)
        {
            // Pointer to member can be passed directly, without std::mem_fn.
            Field val = std::invoke(m_func, *parentDevice);

            m_set.emplace(val);

            for (const std::shared_ptr<Device>& device : parentDevice->Children)
            {
                FindDistinctValuesImpl(device);
            }
        }

        std::vector<Field> MoveElements()
        {
            std::vector<Field> v;
            v.reserve(m_set.size());

            while (!m_set.empty())
            {
                v.push_back(std::move(m_set.extract(m_set.begin()).value()));
            }

            return v;
        }

        Getter m_func;

        std::unordered_set<Field> m_set;
    };
}

AWL_TEST(InterviewTask)
{
    AWL_UNUSED_CONTEXT;

    std::shared_ptr<Device> d = std::make_shared<Device>(Device{ "v1", 1, std::vector<std::shared_ptr<Device>>{
        std::make_shared<Device>(Device{"v2", 2, std::vector<std::shared_ptr<Device>>{}})} });

    // Pointer to member is invocable, so we can use it without std::mem_fn.
    {
        auto versions = Finder{ &Device::Version }.FindDistinctValues(d);

        auto numbers = Finder{ &Device::InventoryNumber }.FindDistinctValues(d);
    }

    // An example of using std::mem_fn.
    {
        auto versions = Finder{ std::mem_fn(&Device::Version) }.FindDistinctValues(d);

        auto numbers = Finder{ std::mem_fn(&Device::InventoryNumber) }.FindDistinctValues(d);
    }

    // std::function is also invocable.
    {
        auto versions = Finder{ std::function<std::string(const Device&)>(std::mem_fn(&Device::Version)) }.FindDistinctValues(d);

        auto numbers = Finder{ std::function<int (const Device&)>(std::mem_fn(&Device::InventoryNumber)) }.FindDistinctValues(d);
    }
}

AWL_TEST(InterviewMoveSet)
{
    AWL_UNUSED_CONTEXT;

    std::unordered_set<std::string> set;

    set.insert("a long string number 0000000000000000000000000000001");
    set.insert("a long string number 0000000000000000000000000000002");
    set.insert("a long string number 0000000000000000000000000000003");

    {
        // The code below does not move `unordered_set` elements to `vector`, but it copies them because `unordered_set` elements are const.
        std::vector<std::string> v(std::make_move_iterator(set.begin()), std::make_move_iterator(set.end()));
    }

    {
        std::vector<std::string> v;
        v.reserve(set.size());

        while (!set.empty())
        {
            v.push_back(std::move(set.extract(set.begin()).value()));
        }
    }
}
