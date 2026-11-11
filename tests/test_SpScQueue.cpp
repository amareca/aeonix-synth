#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "domain/SpScQueue.h"

using namespace aeonix;

TEST_CASE ("SPSC queue pushes and pops in order", "[queue]")
{
    SpScQueue<int, 4> q;
    int out = 0;

    REQUIRE_FALSE (q.tryPop (out));   // vacia al principio

    REQUIRE (q.tryPush (10));
    REQUIRE (q.tryPush (20));
    REQUIRE (q.tryPush (30));

    REQUIRE (q.tryPop (out));
    REQUIRE (out == 10);
    REQUIRE (q.tryPop (out));
    REQUIRE (out == 20);
    REQUIRE (q.tryPop (out));
    REQUIRE (out == 30);
    REQUIRE_FALSE (q.tryPop (out));
}

TEST_CASE ("SPSC queue rejects push when full", "[queue]")
{
    SpScQueue<int, 4> q;

    REQUIRE (q.tryPush (1));
    REQUIRE (q.tryPush (2));
    REQUIRE (q.tryPush (3));
    REQUIRE (q.tryPush (4));
    REQUIRE_FALSE (q.tryPush (5));   // llena
}

TEST_CASE ("SPSC queue size tracks the element count", "[queue]")
{
    SpScQueue<int, 8> q;

    REQUIRE (q.size() == 0);

    q.tryPush (1);
    q.tryPush (2);
    q.tryPush (3);
    REQUIRE (q.size() == 3);

    int out = 0;
    q.tryPop (out);
    REQUIRE (q.size() == 2);
}
