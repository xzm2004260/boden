#include "testStringImpl.h"

// Note: we split the tests for the StringImpl implementation for the individual
// encoding into multiple CPP files. Some C++ compilers can otherwise choke on
// the huge amounts of template variations generated in a single file.

TEST_CASE("StringImpl-utf16", "[.][long][String]") { testStringImpl<Utf16StringData>(); }
