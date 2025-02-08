#include "StringUtil.h"

namespace awl
{
    void removeTrailingZeros(QString & s)
    {
        auto i = s.rbegin();

        for (; i != s.rend(); ++i)
        {
            if (*i != '0')
            {
                break;
            }
        }

        int n = i - s.rbegin();
        if (n != 0)
        {
            int pos = s.rend() - i;

            s.remove(pos, n);
        }

        //Add one zero back if the string ends with "." (like "365.")
        if (s.endsWith('.'))
        {
            s.append('0');
        }
    }
}

