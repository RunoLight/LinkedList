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
#include <ctime>
#include <activation.h>

#include "LinkedList.h"

#define CATCH_CONFIG_MAIN 
#include "catch.hpp"

using namespace std;


TEST_CASE("Setup") {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
}

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

    SECTION("Many pushes and erases") {
        int tries = 1;
        while (tries--) {
            auto pushes = 10000;
            //auto pushes = 1000000;
            LinkedList<int>* list = new LinkedList<int>();
            for (int i = 0; i < pushes; i++) {
                list->push_back(i);
                if (i % (pushes / 100) == 0) {
                    cout << i << " pushes, " << i / (pushes / 100) << " %" << endl;
                }
            }
            REQUIRE(list->Size() == pushes);
            cout << "done pushes\n";

            int i = 0;
            auto it = list->begin();
            for (int j = 0; j < 1000000; j++) {
                if (j % (pushes / 100) == 0) {
                    cout << i << " pushes, " << j / (pushes / 100) << " %" << endl;
                }
                it = list->erase(it);
            }
            cout << "LIST SIZE " << list->Size() << endl;
            delete list;
        }
    }
}

void do_join(thread& t) { t.join(); }
void join_all(vector<thread>& v) { for_each(v.begin(), v.end(), do_join); }

auto pusherFunction = [](LinkedList<int>* list, ListIterator<int> it, const int& numberOfPushes, const int& value)
{
    for (auto i = 0; i < numberOfPushes; i++) {
        it = list->insert_after(it, value);
    }
    printf("Pusher %i ended with %i pushes\n", value, numberOfPushes);
};

auto deleterFunction = [](LinkedList<int>* list, ListIterator<int> it, const int& numberOfErases)
{
    for (auto i = 0; i < numberOfErases; i++) {
        //it++;
        //list->erase(it);
        it = list->erase(it);
    }
    printf("Eraser ended with %i erases\n", numberOfErases);
};

void ThreadedTest1(const int threads, int tries, const int totalPushes) {
    const int pushesPerThread = totalPushes / threads;
    while (tries--) {

        LinkedList<int>* list = new LinkedList<int>();
        vector<ListIterator<int>> iterators;
        ListIterator<int>* it = new ListIterator<int>(list->begin());
        for (int i = 0; i < threads; i++) {
            *it = list->insert_after(*it, i);
            iterators.push_back(*it);
            *it = list->insert_after(*it, i);
        }
        delete it;

        cout << "Size after iterators reserved " << list->Size() << endl;

        // Total size of list ready for pushing is okay
        REQUIRE(list->Size() == threads * 2);

        // Pushers
        {
            vector<thread> pushers;
            pushers.reserve(threads);
            for (int i = 0; i < threads; i++) {
                pushers.push_back(
                    thread(pusherFunction, list, iterators[i], pushesPerThread, i)
                );
            }
            join_all(pushers);
            //for (int i = 0; i < threads; i++) {
            //    auto iterat = iterators[i];
            //    for (auto j = 0; j < pushesPerThread; j++) {
            //        iterat = list->insert_after(iterat, i);
            //    }
            //    printf("Pusher %i ended with %i pushes\n", i, pushesPerThread);
            //}
        }

        std::cout << "done pushes\n";
        REQUIRE(list->Size() == threads * (pushesPerThread + 1));
        cout << "Size after pushers done " << list->Size() << endl;
        {
            // TODO DELETE
            auto z = 0;
            auto iter = list->begin();
            while (iter != list->end()) {
                iter++;
                z++;
            }
            std::cout << z << endl;
        }

        cout << "Size after itering done " << list->Size() << endl;

        {
            vector<thread> deleters;
            deleters.reserve(threads);
            for (int i = 0; i < threads; i++) {
                deleters.push_back(
                    thread(deleterFunction, list, iterators[i], pushesPerThread - 1)
                );
            }
            join_all(deleters);

            //for (int i = 0; i < threads; i++) {
            //    auto iterat = iterators[i];
            //    for (auto j = 0; j < pushesPerThread + 1; j++) {
            //        iterat = list->erase(iterat);
            //    }
            //    printf("Eraser ended with %i erases\n", pushesPerThread);
            //}
        }


        cout << "Size after deleters done " << list->Size() << endl;

        REQUIRE(list->Size() == threads);
        
        //auto deleter = thread(deleterFunction, list, list->begin(), totalPushes);
        //deleter.join();

        std::cout << "done erases\n";
        //REQUIRE(list->Size() == threads);
        delete list;

    //    // Total size of list ready for pushing is okay
    //    CHECK(list.Size() == threadsAmount);
    //    cout << "\nIterators reserved, inital elements added";
    //    cout << list.debug();

    //    vector<thread> pushers;
    //    pushers.reserve(threadsAmount);
    //    for (int i = 0; i < threadsAmount; i++) {
    //        pushers.push_back(
    //            thread(pusherFunction, &list, iterators[i], pushesPerThread, i)
    //        );
    //    }
    //    join_all(pushers);


        // PUSH IN ONE THREAD
        //auto pusher = thread(pusherFunction, list, list->begin(), totalPushes, 0);
        //pusher.join();

        //REQUIRE(list->Size() == totalPushes);


        //REQUIRE(list->Size() == totalPushes);


        //int i = 0;
        //auto it = list->begin();
        //for (int j = 0; j < totalPushes; j++) {
        //    if (j % (totalPushes / 100) == 0) {
        //        cout << j << " erases, " << j / (totalPushes / 100) << " %" << endl;
        //    }
        //    it = list->erase(it);
        //}

    }
}


TEST_CASE("Threaded tests", "[threads]") {
    auto hardwareThreadsAmount = thread::hardware_concurrency();

    SECTION("Threaded pushing, correct size of list, erase after threaded pushing") {
        auto repeatEveryTest = 5;
        auto totalPushes = 10000000;
        //int threadAmount = 1;
        int threadAmount = 4;
        int pushesPerThread;

        while (threadAmount <= hardwareThreadsAmount) {
            std::cout << "Multithreaded test, " << threadAmount << " threads\n\n";
            pushesPerThread = totalPushes / threadAmount;

            ThreadedTest1(threadAmount, repeatEveryTest, totalPushes);

            threadAmount *= 2;
        }

        //int count = 100000000;
        //LinkedList<int> list;
        //for (auto i = 0; i < count; i++) {
        //    list.push_back(i);
        //};

        //auto it = list.begin();
        //while (it != list.end()) {
        //    it = list.erase(it);
        //}

        //thread deleter(deleterFunction, &list, it, count);
        //deleter.join();

        //REQUIRE(list.Size() == 0);
        //cout << "WOW\n";

        //SIGNLE THREAD UP



        //LinkedList<int> list;
        //int threadsAmount = 1;
        //int pushes = 1000;

        //ListIterator<int>* it = new ListIterator<int>(list.begin());
        //for (unsigned long i = 0l; i < threadsAmount * pushes; i++) {
        //    list.push_back(i);
        //}

        //REQUIRE(list.Size() == threadsAmount * pushes);
        //cout << "\nSignle thread pushed";
        //cout << list.debug();

        //*it = list.begin();

        ////thread deleter(deleterFunction, &list, *it, pushes);
        ////deleter.join();

        ////for (auto i = 0; i < pushes-5; i++) {
        ////    cout << "\n" << i << " erase";
        ////    list.erase(*it);
        ////    cout << "\nErased, iterating";
        ////    (*it)++;
        ////    cout << "\nIterated";
        ////}
        ////printf("Eraser ended with %i erases\n", pushes);

        //while (*it != --(--(--(--(--(list.end())))))) {
        //    list.erase(*it);
        //    (*it)++;
        //}

        //delete it;

        //cout << "\nSignle thread erased";
        //cout << list.debug();

        ////auto iter2 = list.begin();
        ////list.erase(iter2);
        ////iter2++;
        ////list.erase(iter2);
        ////iter2++;
        ////list.erase(iter2);
        ////iter2++;
        ////// TODO INSERT AFTER DELETED NODE FIX
        ////list.insert_after(iter2, 11);
        ////list.insert_after(iter2, 22);
        ////list.insert_after(iter2, 33);
        ////iter2++;
        ////iter2++;
        ////list.erase(iter2);
        ////iter2++;
        ////iter2++;
        //
        //cout << "\nErased few elements";
        //cout << list.debug();
        //cout << "----------------\n";

        //REQUIRE(true);
    }



    //SECTION("Threaded pushing, correct size of list, erase after threaded pushing") {
    //    clock_t start = clock();

    //    int threadsAmount = 8;
    //    int pushesPerThread = 100000;

    //    LinkedList<int> list;
    //    vector<ListIterator<int>> iterators;

    //    ListIterator<int>* it = new ListIterator<int>(list.begin());

    //    for (int i = 0; i < threadsAmount; i++) {
    //        *it = list.insert_after(*it, i);
    //        iterators.push_back(*it);
    //    }

    //    delete it;

    //    // Total size of list ready for pushing is okay
    //    CHECK(list.Size() == threadsAmount);
    //    cout << "\nIterators reserved, inital elements added";
    //    cout << list.debug();

    //    vector<thread> pushers;
    //    pushers.reserve(threadsAmount);
    //    for (int i = 0; i < threadsAmount; i++) {
    //        pushers.push_back(
    //            thread(pusherFunction, &list, iterators[i], pushesPerThread, i)
    //        );
    //    }

    //    join_all(pushers);

    //    CHECK(list.Size() == threadsAmount + threadsAmount * pushesPerThread);
    //    cout << "\nThreads pushers finished";
    //    //cout << list.debug();

    //    vector<thread> deleters;
    //    deleters.reserve(threadsAmount);
    //    for (int i = 0; i < threadsAmount; i++) {
    //        deleters.push_back(
    //            thread(deleterFunction, &list, iterators[i], pushesPerThread)
    //        );
    //    }
    //    join_all(deleters);

    //    

    //    // Verification

    //    REQUIRE(list.Size() == threadsAmount);

    //    ListIterator<int>* validationIterator = new ListIterator<int>(list.begin());
    //    int value = 0;
    //    while (*validationIterator != list.end()) {
    //        auto listValue = **validationIterator;
    //        auto neededValue = value++;
    //        REQUIRE(listValue == neededValue);

    //        (*validationIterator)++;
    //    }

    //    cout << "\Threads deleters finished";
    //    cout << list.debug();
    //    cout << "----------------\n";

    //    cout << "\n\n\n" << list.Size() << "\n\n\n";

    //    cout << "\nDone within " << float(clock() - start) / CLOCKS_PER_SEC * 1000 << " msec." << endl;
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