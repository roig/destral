# Destral

PROGRAMMING INFORMATION
# C++ ONLY Allowed list:

Headers allowed:
	#include <memory>
	#include <string>
	#include <vector>
	#include <unordered_map>
	#include <functional>

- Just use structs
- Use struct initialization struct example { int i = 0; }
- Try to use static arrays when possible. avoid std::vector
- Try no overuse std::vector / std::unordered_map
- Try to not use std::string if it's not needed at all.


# C++ forbidden list:
- NO CLASSES 
- NO OOP
- NO inheritance
- no const in functions
- no const references in functions (just pointers like C)
- no functions inside structs (make functions like in C) (struct only data)
- no C++ unnamed initializer  { "hjideidjei", 788, 89.f} (this sucks..) use designated initialization { .name="Hello", .index = 0} etc...

