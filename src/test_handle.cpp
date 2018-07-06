// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

TEST_CASE( "Inital Test", "[Handle" ) {
    REQUIRE( 1 == 1 );
}

