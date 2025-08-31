# OneOf

OneOf is a headers-only C++ library that aims to bring Rust-like enums to C++.
- No dependencies (thanks to William R Swanson for his copyright free implementation of a map macro though)
- C++ min version : c++17 (because of the internal use of std::variant)

## Getting started

The macro **GENERATE_ONE_OF** can be used to generate a class that contains one of the options given to the macro

 ```cpp
GENERATE_ONE_OF(MyOneOf,        // the name of the class
    (KEY_1, float),             // a variant with the key KEY_1 will contain a float value
    (KEY_2, std::string),       // a variant with the key KEY_2 will contain a std::string value
    (KEY_3, std::string),       // a variant with the key KEY_3 will contain a std::string value
    (KEY_4, MyOneOf::Empty)     // a variant with the key KEY_4 will contain no data
) // Notice that two keys can be associated with the same type
```
You can instanciate your class with the static member function **create** :

```cpp
// The compiler will yell at you if you try to call create with a type that doesn't match the specified key
MyOneOf my_oneof = MyOneOf::create<MyOneOf::Keys::KEY_2> ("My value"); 
```

In order to handle an instance of your class you have to use the visitor pattern via the **Visitor** or the **VisitorMut** class
(the **VisitorMut** class's callbacks that take non-const reference arguments)

```cpp
// Create our visitor

// The visitor constructor takes the visit return type as a template parameter,
// this is the type that will be returned by the visit method.
// All visitor callbacks must return this type
auto stringify_visitor = MyOneOf::Visitor<std::optional<std::string>>()
.match<MyOneOf::Keys::KEY_1>([](const float& val)       { return std::to_string(val); })
.match<MyOneOf::Keys::KEY_2>([](const std::string& val) { return val; })
.match<MyOneOf::Keys::KEY_3>([](const std::string& val) { return val; })
.fallback([](const MyOneOf::Keys& key){ return std::nullopt; }); // Called if none of the keys above match   

// Visit our MyOneOf instance

// NOTE : We could have done the visitor declaration and the visit in the same line
// but it can be useful to keep the visitor if you want to apply it to multiple OneOfs  

// IMPORTANT : The compiler will yell at you if all keys are not handled by the visitor,
// so you must either match all keys or define a fallback function before calling visit

std::optional<std::string> stringified = stringify_visitor.visit(my_oneof); // "My value"
```

## Basic example

```cpp
#include <one_of/one_of.h>

#include <iostream>

struct AnimalProperties { /* ... */ };
struct PlantProperties  { /* ... */ };
struct RockProperties   { /* ... */ };

// Define my "OneOf" class
GENERATE_ONE_OF(Thing,              // the name of the class
    (ANIMAL,   AnimalProperties),   // a Thing can be an ANIMAL that has AnimalProperties
    (PLANT,    PlantProperties),    // a Thing can be a PLANT that has PlantProperties
    (ROCK,     RockProperties),     // a Thing can be a ROCK that has RockProperties
    (UNKNOWN,  Thing::Empty)        // a Thing can be UNKNOWN, it then has no properties

    // ... you can add more variants as needed, two variants can even have the same type
)              

int main()
{
    // Create instances of Thing
    // The compiler will yell at you if you try to create a Thing with a type that doesn't match the specified key
    Thing animal  = Thing::create<Thing::Keys::ANIMAL>  (AnimalProperties{}); 
    Thing plant   = Thing::create<Thing::Keys::PLANT>   (PlantProperties{});
    Thing rock    = Thing::create<Thing::Keys::ROCK>    (RockProperties{});
    Thing unknown = Thing::create<Thing::Keys::UNKNOWN> (Thing::Empty{});

    // Create a visitor to handle each variant
    // void is the return type of the visit, all visitor functions must return void
    // The constructor takes a fallback function that is called if no specific visitor is defined for a key
    auto visitor = Thing::Visitor<void>()
    .match<Thing::Keys::ANIMAL> ([](const AnimalProperties& props)  { std::cout << "It's an animal!" << std::endl; })
    .match<Thing::Keys::PLANT>  ([](const PlantProperties& props)   { std::cout << "It's a plant!" << std::endl; })
    .match<Thing::Keys::ROCK>   ([](const RockProperties& props)    { std::cout << "It's a rock!" << std::endl; })
    .match<Thing::Keys::UNKNOWN>([](const Thing::Empty& props)      { std::cout << "What is it?" << std::endl; });

    // Visit each Thing instance
    visitor.visit(animal);
    visitor.visit(plant);
    visitor.visit(rock);
    visitor.visit(unknown);

    // Create a visitor that checks if a Thing is a living thing (animal or plant)
    // bool is the return type of the visit, all visitor functions must return bool
    // We only define visitors for ANIMAL and PLANT, the others will be handled by the fallback
    auto is_living_thing_visitor = Thing::Visitor<bool>()
    .match<Thing::Keys::ANIMAL>([](const AnimalProperties&) { return true; })
    .match<Thing::Keys::PLANT>([](const PlantProperties&) { return true; })
    .defaultFallback(); // The fallback will return false (bool default constructor gives false)
    
    // Use the visitor to check if each Thing is a living thing
    // Here, visit returns a bool as we gave it a Visitor<bool>
    bool is_animal_living  = is_living_thing_visitor.visit(animal);     // true
    bool is_plant_living   = is_living_thing_visitor.visit(plant);      // true
    bool is_rock_living    = is_living_thing_visitor.visit(rock);       // false
    bool is_unknown_living = is_living_thing_visitor.visit(unknown);    // false

    std::cout << std::boolalpha;
    std::cout << "Is animal a living thing? " << is_animal_living << std::endl;
    std::cout << "Is plant a living thing? " << is_plant_living << std::endl;
    std::cout << "Is rock a living thing? " << is_rock_living << std::endl;
    std::cout << "Is unknown a living thing? " << is_unknown_living << std::endl;
}
```
