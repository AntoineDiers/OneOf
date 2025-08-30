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
    (KEY_4, ClassName::Empty)   // a variant with the key KEY_4 will contain no data
) // Notice that two keys can be associated with the same type
```
You can instanciate your class with the static member function **create** :

```cpp
// The compiler will yell at you if you try to call create with a type that doesn't match the specified key
MyOneOf my_oneof = MyOneOf::create<MyOneOf::Keys::KEY_2> ("My value"); 
```

In order to handle an instance of your class you have to use the visitor pattern

```cpp
// the Visitor<T> class constructor takes in a fallback function that is called if 
// no specific callback is defined for the visited key. T is the return type of the visit, 
// all visitor functions must return this type
MyOneOf::Visitor<std::optional<std::string>> stringify_visitor([](const Thing::Keys& ) { return std::nullopt; });

// Add visitor functions for the keys that we know how to stringify, the rest of the keys will be handled by the fallback
stringify_visitor.KEY_1 = [](const float& val) { return std::to_string(val); }
stringify_visitor.KEY_2 = [](const std::string& val) { return val; }
stringify_visitor.KEY_3 = [](const std::string& val) { return val; }

// Visit our MyOneOf instance
std::optional<std::string> stringified = my_oneof.visit(stringify_visitor);
```


> The class **MyOneOf::VisitorMut** can be used with the member function **MyOneOf::visit_mut** to define visitors that take non-const reference arguments.

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
    Thing::Visitor<void> visitor([](const Thing::Keys&) { std::cout << "Unhandled Thing type" << std::endl; });
    visitor.ANIMAL  = [](const AnimalProperties& props) { std::cout << "It's an animal!" << std::endl; };
    visitor.PLANT   = [](const PlantProperties& props)  { std::cout << "It's a plant!" << std::endl; };
    visitor.ROCK    = [](const RockProperties& props)   { std::cout << "It's a rock!" << std::endl; };
    visitor.UNKNOWN = [](const Thing::Empty&)           { std::cout << "It's an unknown thing!" << std::endl; };

    // Visit each Thing instance
    animal.visit(visitor);
    plant.visit(visitor);
    rock.visit(visitor);
    unknown.visit(visitor);

    // Create a visitor that checks if a Thing is a living thing (animal or plant)
    // bool is the return type of the visit, all visitor functions must return bool
    // We only define visitors for ANIMAL and PLANT, the others will be handled by the fallback
    Thing::Visitor<bool> is_living_thing_visitor([](const Thing::Keys&){ return false; });
    is_living_thing_visitor.ANIMAL = [](const AnimalProperties&) { return true; };
    is_living_thing_visitor.PLANT  = [](const PlantProperties&)  { return true; };

    // Use the visitor to check if each Thing is a living thing
    // Here, visit returns a bool as we gave it a Visitor<bool>
    bool is_animal_living  = animal.visit(is_living_thing_visitor);     // true
    bool is_plant_living   = plant.visit(is_living_thing_visitor);      // true
    bool is_rock_living    = rock.visit(is_living_thing_visitor);       // false
    bool is_unknown_living = unknown.visit(is_living_thing_visitor);    // false

    std::cout << std::boolalpha;
    std::cout << "Is animal a living thing? " << is_animal_living << std::endl;
    std::cout << "Is plant a living thing? " << is_plant_living << std::endl;
    std::cout << "Is rock a living thing? " << is_rock_living << std::endl;
    std::cout << "Is unknown a living thing? " << is_unknown_living << std::endl;
}
```
