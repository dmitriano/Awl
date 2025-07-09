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

struct Device
{
    std::string Version;
    int InventoryNumber;
    std::vector<std::shared_ptr<Device>> Children;
};

template <class Getter>
    requires std::invocable<Getter, const std::shared_ptr<Device>&>
class Finder
{
public:

    using Field = std::decay_t<std::invoke_result_t<Getter, const std::shared_ptr<Device>&>>;

    Finder(Getter func) : m_func(std::move(func)) {}

    std::vector<Field> FindDistinctValues(std::shared_ptr<Device> rootDevice)
    {
        FindDistinctValuesImpl(rootDevice);

        return std::vector<Field>(std::make_move_iterator(m_set.begin()), std::make_move_iterator(m_set.end()));
    }

private:

    void FindDistinctValuesImpl(const std::shared_ptr<Device>& parentDevice)
    {
        // Pointer to member can be passed directly, without std::mem_fn.
        Field val = std::invoke(m_func, parentDevice);

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

    // An example of using std::function.
    {
        auto versions = Finder{ std::function<std::string(const std::shared_ptr<Device>&)>(std::mem_fn(&Device::Version)) }.FindDistinctValues(d);

        auto numbers = Finder{ std::function<int (const std::shared_ptr<Device>&)>(std::mem_fn(&Device::InventoryNumber)) }.FindDistinctValues(d);
    }
}
