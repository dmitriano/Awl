/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
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

        const qsizetype n = i - s.rbegin();
        if (n != 0)
        {
            const qsizetype pos = s.rend() - i;

            s.remove(pos, n);
        }

        //Add one zero back if the string ends with "." (like "365.")
        if (s.endsWith('.'))
        {
            s.append('0');
        }
    }
}

