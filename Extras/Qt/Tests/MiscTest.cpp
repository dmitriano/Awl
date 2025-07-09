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

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

struct Device
{
    std::string Version;
    int InventoryNumber;
    std::vector<std::shared_ptr<Device>> Children;
};

void f()
{
    auto getter = std::mem_fn(&Device::Version);

    using Getter = decltype(getter);

    using Field = std::decay_t<std::invoke_result_t<Getter, Device&>>;

    static_assert(std::is_same_v<Field, std::string>);
}

template <class Getter>
class Finder
{
public:

    using Field = std::decay_t<std::invoke_result_t<Getter, Device&>>;

    Finder(Getter func) : m_func(std::move(func)) {}

    std::vector<Field> FindDistinctValues(std::shared_ptr<Device> rootDevice)
    {
        FindDistinctValuesImpl(rootDevice);

        return std::vector<Field>(std::make_move_iterator(m_set.begin()), std::make_move_iterator(m_set.end()));
    }

private:

    void FindDistinctValuesImpl(const std::shared_ptr<Device>& parentDevice)
    {
        Field val = m_func(parentDevice);

        m_set.insert(val);

        for (const std::shared_ptr<Device>& device : parentDevice->Children)
        {
            FindDistinctValuesImpl(device);
        }
    }

    Getter m_func;

    std::unordered_set<Field> m_set;
};

AWL_TEST(Interview)
{
    AWL_UNUSED_CONTEXT;

    std::shared_ptr<Device> d = std::make_shared<Device>(Device{ "v1", 1, std::vector<std::shared_ptr<Device>>{
        std::make_shared<Device>(Device{"v2", 2, std::vector<std::shared_ptr<Device>>{}})} });

    auto versions = Finder{ std::mem_fn(&Device::Version) }.FindDistinctValues(d);

    auto numbers = Finder{ std::mem_fn(&Device::InventoryNumber) }.FindDistinctValues(d);
}
