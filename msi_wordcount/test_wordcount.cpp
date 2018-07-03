#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <string>
#include <sstream>
#include "libmsi_wordcount.h"

TEST_CASE( "Word count input stream", "[wordcount]" )
{
  WordCounter wc;
  std::istringstream buffer("   word1 \t word2 \n word3 \r\nword4");
  REQUIRE(wc.read(buffer) == 4u);
}


TEST_CASE( "Word count chunk wise", "[wordcount]" )
{
  WordCounter wc;
  wc.read(" 23", 3);
  wc.read("4  ", 3);
  wc.read("   ", 3);
  wc.read(" 1 ", 3);
  wc.read("1  ", 3);
  REQUIRE(wc.getNumWords() == 3u);
}
