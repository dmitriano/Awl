#include "Awl/Ring.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

#include <sstream>

AWT_TEST(Ring)
{
    awl::ring<int> mybuf(10);

    std::ostringstream cout;

    for (size_t i = 0; i < 20; ++i) {
        mybuf.push(i);
        for (auto i = mybuf.begin(); i != mybuf.end(); ++i) cout << *i << ": ";
        if (mybuf.full()) cout << "full";
        cout << '\n';
    }
    cout << "Buffer Size: " << mybuf.size() << '\n';
    for (size_t i = 0; i < mybuf.size() + 1; ++i) {
        try
        {
            cout << mybuf.at(i) << ": ";
        }
        catch (std::exception e)
        {
            cout << e.what() << '\n';
            continue;
        }
    }
    cout << '\n';
    auto start = mybuf.begin();
    start += 1;
    cout << "start++: " << *start << '\n';
    awl::ring<int>::const_iterator cstart(start);
    cout << "cstart(start)++: " << *(++cstart) << '\n';
    cout << "--start: " << *(--start) << '\n';
    if (start == mybuf.begin()) cout << "Start is mybuf.begin\n";
    else cout << "Lost!\n";
    cout << "Push!\n";
    mybuf.push(100);
    if (start == mybuf.begin()) cout << "In the begining :-)\n";
    else cout << "Start is no longer mybuf.begin\n";
    start = mybuf.begin();
    cout << "after push, start: " << *start << '\n';
    cout << "forwards:  ";
    for (auto i = mybuf.begin(); i < mybuf.end(); i += 2) cout << *i << ": ";
    cout << '\n';
    cout << "backwards: ";
    for (auto i = mybuf.rbegin(); i < mybuf.rend(); i += 2) cout << *i << ": ";
    cout << '\n';
    cout << "mybuf[0]: " << mybuf[0] << " " << "\nPush!\n\n";
    mybuf.push(20);
    for (size_t i = 0; i < mybuf.size(); ++i) cout << mybuf[i] << ": ";
    cout << '\n';
    cout << "pop: " << mybuf.top() << '\n';
    mybuf.pop();
    cout << "new front: " << mybuf[0] << " new size: ";
    cout << mybuf.size() << '\n';
    cstart = mybuf.end();
    cout << "last: " << *(--cstart) << '\n';
    for (auto i = mybuf.begin(); i != mybuf.end(); ++i) cout << *i << ": ";
    cout << '\n';
    cout << "pop again: " << mybuf.front() << '\n';
    mybuf.pop();
    cstart = mybuf.rbegin();
    cout << "last: " << *cstart << '\n';
    for (auto i = mybuf.begin(); i != mybuf.end(); ++i) cout << *i << ": ";
    cout << "\n\nclone: ";
    awl::ring<int> cbuf(mybuf);
    
    //std::find reqires 'operator-=' in GCC.
    //for (auto i = std::find(mybuf.begin(), mybuf.end(), 100); i != cbuf.end(); ++i) cout << *i << ": ";

    auto iter = cbuf.cbegin();
    cout << "\nbegin[3] = " << iter[3];
    cout << '\n' << '\n';
    cout << "Hello World!\n";

    context.out << awl::FromAString(cout.str());
}
