# OneOf

OneOf is a no-dependencies headers-only C++11 library that aims to bring Rust-like enums to C++.

Just like in rust : 
- Your "enum" Alternatives can contain any type
- A match pattern allows you to visit the Alternatives
- You can ensure at compile-time that all Alternatives are matched
### Step 1 : Create your Alternatives

```cpp
#include <one_of/one_of.h>
#include <string>

// Alternatives :         NAME           TYPE
ONE_OF_CREATE_ALTERNATIVE(ALTERNATIVE_1, int)
ONE_OF_CREATE_ALTERNATIVE(ALTERNATIVE_2, std::string)
ONE_OF_CREATE_ALTERNATIVE(ALTERNATIVE_3, int)
```

### Step 2 : Define your OneOf

```cpp
typedef one_of::OneOf<
    ALTERNATIVE_1, 
    ALTERNATIVE_2, 
    ALTERNATIVE_3> MyOneOf;
```

### Step 3 : Instanciate your OneOf

```cpp
int main()
{
    MyOneOf instance(ALTERNATIVE_2{}, "Hello World!"); 
```

### Step 4 : Match !

```cpp
    instance.match<ALTERNATIVE_1>([](int& val)
    {
        //...
    })
    .match<ALTERNATIVE_2>([](std::string& val)
    {
        // This will be executed !
    })
    .match<ALTERNATIVE_3>([](int& val)
    {
        //...
    })
```

### Step 4 : Finalise

```cpp
    .assertMatchIsExhaustive();
```

Alternatively if you do not want to match all Alternatives : 

```cpp
    .fallback([]()
    {
        //...
    });
```

You can also do nothing but this is not advised. 

## Basic example

> A more exhaustive example is available [here](./example/src/main.cpp)

```cpp
#include <one_of/one_of.h>
#include <iostream>
#include <string>

struct Empty{};
ONE_OF_CREATE_ALTERNATIVE(FOUND,     size_t)
ONE_OF_CREATE_ALTERNATIVE(NOT_FOUND, Empty)
typedef one_of::OneOf<FOUND, NOT_FOUND> FindResult;

FindResult find_first(const std::string& str, char character)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == character) { return FindResult(FOUND{}, i); }
    }
    return FindResult(NOT_FOUND{});
}

int main()
{
    find_first("abcdefghijk", 'h')
    .match<FOUND>([](const size_t& index)
    {
        std::cout << "Found character at index " << index << std::endl;
    })
    .match<NOT_FOUND>([](const Empty&)
    {
        std::cout << "Character not found" << std::endl;
    })
    .assertMatchIsExhaustive();
}
```

