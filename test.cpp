#include <bits/stdc++.h>
#include <algorithm>
int main()
{
    std::deque<int> d = {0, 5, 7, 2, 1, -14};
    auto greater = [](const int A, const int B)
    {
        return A > B;
    };
    std::sort(d.begin(), d.end(), greater);

    for(const auto &i : d)

    std::cout<< i;
    return 0;
}