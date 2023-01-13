#include "randomized_queue.h"

#include <cstdlib>
#include <iostream>

void assert (bool expr) {
    if (!expr) {
        std::cout << "Error, assert expected value" << std::endl;
        exit(30);
    }
}

int main()
{
    randomized_queue<int> q;
    for (int i = 0; i < 5; ++i) {
        q.enqueue(i);
    }
    auto b1 = q.begin();
    auto e1 = q.end();
    auto b2 = q.begin();
    auto e2 = q.end();

    std::vector<int> v11, v12;
    std::copy(b1, e1, std::back_inserter(v11));
    std::copy(b1, e1, std::back_inserter(v12));
    assert(v11 == v12); // equal vectors

    std::vector<int> v21, v22;
    std::copy(b2, e2, std::back_inserter(v21));
    std::copy(b2, e2, std::back_inserter(v22));
    assert(v21 == v22); // again equal vectors

    assert(v11 != v21); // vectors are not equal with high probability

    while (!q.empty()) {
        std::cout << q.dequeue() << ' ';
    }
    std::cout << std::endl;

    std::cout << "Visual test:" << std::endl;

    randomized_queue<int> q_v;

    q_v.enqueue(47);
    q_v.enqueue(42);
    q_v.enqueue(30);
    for (int i = 0; i < 10; ++i) {
        q_v.enqueue(i);
    }
    std::cout << "Preparations finished! Size of queue is: " << q_v.size() << std::endl;

    for (int t = 0; t < 7; ++t) {
        std::cout << "Test #" << t + 1 << ":" << std::endl;
        int ind = 0;
        for (auto i = q_v.begin(); i != q_v.end(); ++i, ++ind) {
//            printf("[%d]: %2d ", ind, *i);
            std::cout << "[" << ind << "]: ";
            std::cout << *i << ' ';
        }
        std::cout << std::endl;
    }
}
