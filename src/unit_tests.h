#pragma once

namespace chc
{

#define Test(x) \
	do { \
		if (!(x)) { \
			std::cout << "TEST_FAILED: " << #x << std::endl; \
			std::abort(); \
		} \
	} while (0)

namespace unit_tests
{
	void testAll();
}

}
