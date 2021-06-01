#include <functional>
#include <utility>
#include <type_traits>
#include <vector>
#include <limits>
#include <numeric>
#include <cmath>
#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <utility>
#include <type_traits>
#include <vector>
#include <limits>
#include <numeric>
#include <cmath>
#include <memory>
#include <iostream>
#include <string>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <iostream>
#include <atomic>
#include <cstdlib>
#include <activation.h>

#include "LinkedList.h"

#define CATCH_CONFIG_MAIN 
#include "catch.hpp"

using namespace std;

TEST_CASE("Core mechanics tests", "[Base]") {
    SECTION("Constructor, destructor, iterating, push") {
        LinkedList<int> l;
        l.push_back(1);
        l.push_front(2);
        l.push_back(3);

        auto it = l.begin();

        REQUIRE(*it == 2);
        it++;
        REQUIRE(*it == 1);
        it++;
        REQUIRE(*it == 3);
        it--;
        REQUIRE(*it == 1);
        l.insert_after(it, 10);
        it++;
        REQUIRE(*it == 10);
    }

    SECTION("Initializer list constructor, destructor") {
        LinkedList<int> l2{ 1,2,3,4,5 };
        auto it = l2.begin();
        REQUIRE(*it++ == 1);
        REQUIRE(*it++ == 2);
        REQUIRE(*it == 3);

        LinkedList<bool>* l3 = new LinkedList<bool>{};
        l3->push_back(true);
        REQUIRE(*(l3->begin()) == true);
        delete l3;
    }

    SECTION("Checking ref_count working") {
        LinkedList<int> list{ 1,2,3,4,5 };

        auto it = list.begin();
        REQUIRE(*it == 1);
        it++;
        ++it;

        REQUIRE(*it == 3);

        it--;
        --it;
        REQUIRE(*it == 1);

        ++it;
        auto it_2 = it;
        auto it_3 = it_2;
        it = list.erase(it);
        it = list.erase(it);

        it_2++;
        REQUIRE(*it_2 == 4);
        ++it_3;
        REQUIRE(*it == 4);

        list.insert_after(it_3, 2);
        list.insert_after(it_3, 3);
        it_3 = list.erase(++it_3);
        //it_3--;
        it_3 = list.erase(it_3);

        it_3--;

        it_3 = it_2;

        it_3--;
        --it_2;
        REQUIRE(*it_2 == 1);
        REQUIRE(*it_3 == 1);
    }

    SECTION("==, !=") {
        LinkedList<int> list{ 1,2,3,4,5 };

        auto it = list.begin();
        auto it2 = list.begin();
        REQUIRE(it == it2);

        it2++;
        REQUIRE(it != it2);
        REQUIRE(it.debugRefCount() == 3);
        REQUIRE(it2.debugRefCount() == 3);
        it++;
        REQUIRE(it.debugRefCount() == 4);
    }

    SECTION("push back/push front/insert") {
        LinkedList<int> list;

        list.push_back(3);
        REQUIRE(*--list.end() == 3);

        int a = 4;
        list.push_back(a);
        REQUIRE(*--list.end() == 4);

        list.push_front(2);
        REQUIRE(*list.begin() == 2);

        a = 1;
        list.push_front(a);
        REQUIRE(*list.begin() == 1);

        auto it = list.begin();
        for (int i = 1; i < 4; i++) {
            REQUIRE(*it == i);
            it++;
        }

        it = list.insert_after(it, 5);
        //REQUIRE(*it == 5);

        //int abc = 2;
        //REQUIRE(abc == 2);

    }

    SECTION("erase") {
        LinkedList<int> list{ 1,2,3 };

        auto it = list.begin();
        it = list.erase(it);
        REQUIRE(*it == 2);

        it = list.erase(--list.end());
        REQUIRE(it == list.end());
    }

    SECTION("size") {
        LinkedList<int> list{ 1,2,3 };

        REQUIRE(list.Size() == 3);

        list.push_back(4);
        REQUIRE(list.Size() == 4);

        list.push_front(1);
        REQUIRE(list.Size() == 5);
        
        auto it = list.begin();
        list.erase(it);
        it++;
        REQUIRE(list.Size() == 4);

        list.erase(list.begin());
        REQUIRE(list.Size() == 3);
    }

    SECTION("->") {
        LinkedList<pair<int, int>> list;
        list.push_back(make_pair(1, 2));
        auto it = list.begin();
        REQUIRE(it->first == 1);
    }

    SECTION("Is empty") {
        LinkedList<int> list;
        REQUIRE(list.Size() == 0);
        list.push_back(4);
        REQUIRE(list.Size() == 1);
    }

    SECTION("Many pushes and clear") {
        LinkedList<int> arr;
        for (int i = 0; i < 1000; i++) {
            arr.push_back(i);
        }
        cout << endl << arr.Size();
        REQUIRE(arr.Size() == 1000);
        arr.clear();
        REQUIRE(arr.Size() == 0);
    }
}

void PusherBody(LinkedList<int> list, ListIterator<int> it, int numberOfPushes, int value) {
    while (numberOfPushes--) {
        it = list.insert_after(it, value);
    }
    printf("Pusher %i ended with %i pushes\n", value, numberOfPushes);
}

TEST_CASE("Threaded tests", "[threads]") {
    //SECTION("Threaded pushing, correct size of list, erase after threaded pushing") {
    //    LinkedList<int> list;
    //    int threadsAmount = 100;
    //    int pushes = 100;

    //    std::vector<std::thread> vec;
    //    for (int z = 0; z < threadsAmount; z++) {
    //        vec.push_back(std::thread([&]() {
    //            auto it = list.begin();
    //            for (unsigned long i = 0l; i < pushes; i++) {
    //                list.push_back(i);
    //            }
    //            cout << "\nAdds complete\n";
    //            }));
    //    }

    //    for (int z = 0; z < threadsAmount; z++)
    //    {
    //        vec[z].join();
    //    }

    //    REQUIRE(list.Size() == (threadsAmount * pushes));
    //    cout << "\nPASS SIZE CHECK\n";

    //    std::thread t2([&]() {
    //        cout << "\nDeleter-thread started\n";
    //        auto it = list.begin();
    //        while (it != list.end()) {
    //            it = list.erase(it);
    //        }
    //        cout << "\nDeleter-thread completed\n";
    //    });

    //    t2.join();

    //    REQUIRE(list.Size() == 0);
    //}
    
    //// TODO CHECK TIME
    //SECTION("Threaded pushing, correct size of list, erase after threaded pushing") {
    //    int threadsAmount = 100;
    //    int pushes = 100;
    //    
    //    LinkedList<int> list;
    //    vector<int> iterators;
    //    
    //    ListIterator<int> it = list.begin();
    //    iterators.push_back(it);

    //    for (int i = 0; i < threadsAmount - 1; i++) {
    //        it = list.insert_after(it, i);
    //        it = list.insert_after(it, i);
    //        iterators.push_back(it);
    //    }

    //    

    //    

    //    vector<thread> pushers;
    //    pushers.reserve(threadsAmount);
    //    for (auto i = 0; i < threadsAmount; i++) {
    //        pushers.push_back(
    //            thread(PusherBody, list, it, pushes, i)
    //        );
    //    }

    //    std::vector<std::thread> vec;
    //    for (int z = 0; z < threadsAmount; z++) {
    //        vec.push_back(std::thread([&]() {
    //            auto it = list.begin();
    //            for (unsigned long i = 0l; i < pushes; i++) {
    //                list.push_back(i);
    //            }
    //            cout << "\nAdds complete\n";
    //            }));
    //    }

    //    for (int z = 0; z < threadsAmount; z++)
    //    {
    //        vec[z].join();
    //    }

    //    REQUIRE(list.Size() == (threadsAmount * pushes));
    //    cout << "\nPASS SIZE CHECK\n";

    //    std::thread t2([&]() {
    //        cout << "\nDeleter-thread started\n";
    //        auto it = list.begin();
    //        while (it != list.end()) {
    //            it = list.erase(it);
    //        }
    //        cout << "\nDeleter-thread completed\n";
    //        });

    //    t2.join();

    //    REQUIRE(list.Size() == 0);
    //}
}

/*
TEST_CASE("LinkedList sample", "[CLinkedList]") {        

    


    
    
    SECTION("Checks if read operations works in parallel") {
        {
            std::cout << "\n Begin test that require some time...\n";
            CLinkedList<int> list{ 1, 2, 3, 4, 5};
            int threadsAmount = 2;
            int totalOperations = 5000000;

            CHECK(totalOperations % threadsAmount == 0);

            // Multi-threaded operations
            auto startThreaded = std::chrono::high_resolution_clock::now();
            std::vector<std::thread> vec;
            for (int z = 0; z < threadsAmount; z++) {
                vec.push_back(std::thread([&]() {
                    for (unsigned long i = 0l; i < totalOperations / threadsAmount; i++) {
                        list.size(); list.size(); list.size(); list.size(); list.size(); list.size();
                        list.empty(); list.empty(); list.empty(); list.empty(); list.empty(); list.empty();
                    }
                }));
            }
            for (int z = 0; z < threadsAmount; z++)
            {
                vec[z].join();
            }
            auto endThreaded = std::chrono::high_resolution_clock::now();

            // Single-thread operations
            auto startSingle = std::chrono::high_resolution_clock::now();
            for (unsigned long i = 0l; i < totalOperations; i++) {
                list.size(); list.size(); list.size(); list.size(); list.size(); list.size(); list.size();
                list.empty(); list.empty(); list.empty(); list.empty(); list.empty(); list.empty(); list.empty();
            }
            auto endSingle = std::chrono::high_resolution_clock::now();

            // Check if multi-thread is faster
            auto durationThreaded = std::chrono::duration_cast<std::chrono::microseconds>(endThreaded - startThreaded);
            auto durationSingle = std::chrono::duration_cast<std::chrono::microseconds>(endSingle - startSingle);

            CHECK(durationSingle > durationThreaded);
        }
    }
    
    SECTION("Erasing, iterating attack, checks end(), begin()") {
        {
            std::shared_mutex m;
            std::condition_variable_any cv;

            int threads = 10;
            int elements = 1000;

            CLinkedList<int> list;
            std::vector<int> savedValues;
            std::atomic<bool> dataReady{ false };
            std::atomic<int> consumed{ 0 };
            std::vector<int> wrongValues;

            std::vector<std::thread> checkers;
            for (int i = 0; i < threads; i++)
            {
                checkers.push_back(std::thread([&]() {
                    std::cout << "Checker waiting\n";
                    std::shared_lock<std::shared_mutex> lck(m);
                    cv.wait(lck, [&] { return dataReady.load(); });

                    std::cout << "Running checks [ERASE, it++]\n";
                    auto it = list.begin();
                    for (int i = 0; i < elements / (threads * 3); i++)
                    {
                        it++;
                        it = list.erase(it);
                    }

                    dataReady = false;
                    consumed++;
                    std::cout << "Checks complete\n";
                    cv.notify_all();
                    cv.wait(lck, [&]() {return dataReady == true; });

                    std::cout << "Running checks [END, it++, GET, BEGIN]\n";
                    it = list.begin();
                    while (it != list.end())
                    {
                        it++;
                    }
                    auto val = it.get() + it.get() % 10;
                    savedValues.push_back(val);

                    dataReady = false;
                    consumed++;
                    std::cout << "Checks complete\n";
                    cv.notify_all();
                }));
            }

            std::thread producer([&]() {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::unique_lock<std::shared_mutex> lck(m);

                for (int i = 0; i < elements; i++) {
                    list.push_front(rand());
                }
                dataReady = true;

                std::cout << "Data prepared\n";
                cv.notify_all();
                cv.wait(lck, [&]() { return consumed.load() == threads; });

                std::cout << "Data preparing\n";
                list.push_front(rand());
                dataReady = true;
                consumed = 0;


                std::cout << "Data prepared\n";
                cv.notify_all();
                cv.wait(lck, [&]() { return consumed.load() == threads; });

                std::cout << "Checking results\n";
                auto correctValue = savedValues[0];
                std::copy_if(savedValues.begin(), savedValues.end(), std::back_inserter(wrongValues), [&](int i) {return i != correctValue; });
            });

            for (auto& th : checkers)
                th.join();
            producer.join();

            REQUIRE(wrongValues.size() == 0);
        }
    }
    
}
*/