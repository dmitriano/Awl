/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <Awl/Testing/UnitTest.h>

#include <vector>
#include <map>
#include <set>
#include <string>

class Solution
{
public:

    int rob(std::vector<int>& nums)
    {
        std::vector<int> sums;

        sums.reserve(nums.size());

        if (nums.size() >= 1u)
        {
            sums.push_back(nums[0]);
        }
        else
        {
            return 0;
        }

        if (nums.size() >= 2u)
        {
            sums.push_back(std::max(nums[0], nums[1]));
        }

        for (size_t n = 2; n < nums.size(); ++n)
        {
            int s1 = nums[n] + sums[n - 2];

            int s2 = sums[n - 1];

            sums.push_back(std::max(s1, s2));
        }

        return sums.back();
    }
};

AWL_TEST(Interview)
{
    AWL_UNUSED_CONTEXT;

    AWL_VARIABLE_ATTRIBUTE(std::vector<int>, nums, {});
    AWL_ATTRIBUTE(int, expected_sum, 0);

    Solution sol;

    AWL_ASSERT(sol.rob(nums) == expected_sum);
}
