# Mini-RTTI
Mini-RTTI is a small, header only library which provides type data for your classes and structs.

# Usage
First you have to put one of four macros in your class body
* For regular, non virtual classes use:
```cpp
DECLARE_CLASS( <class_name>, <parent_name_with_namespace> (optional) )
```

* If your class is virtual:
```cpp
DECLARE_POLYMORPHIC_CLASS( <class_name>, <parent_name_with_namespace> (optional) )
```

* If your class is abstract use:
```cpp
DECLARE_ABSTRACT_CLASS( <class_name>, <parent_name_with_namespace> (optional) )
```

* For structs use:
```cpp
DECLARE_STRUCT( <struct_name>, <parent_name_with_namespace> (optional) )
```
<br/>
Whatever your class is, eventually you need to put:

```cpp
IMPLEMENT_TYPE( <class_name_with_namespace> )
```
in your .cpp file.

Your type is required to have default constructor! <br/>
It's also required to have move constructor, but this is an optional feature which you can turn of switching `RTTI_REQUIRE_MOVE_CTOR` to `0` <br/>
And that's it, you can now get some info about your type in the runtime.

# What can I do with that?
### Hierarchy <br/>
Every registered type gets it's unique nested type class, e.g if you have a `class A` then it's type class is will be `A::Type`. These type classes creates a hierarchy based on their original classes hierarchy. `rtti:IType` is the base class for all types classes.
So for instance, if `class A : public B`, then `class A::Type : class B::Type`. Also, all type classes are virtual, even if their original classes are not. It allows you to take advantage of polymorphic features like having a parameter of type `const B::Type&` and passing B's children types as arguments.

### Access parent class without literally typing it <br/>
Very often you might need to invoke some parent class method, but C++ doesn't provide out of the box way to know what your parent is.
In such cases you can use `Super` in your class which you likely know from other languages.

### Instantiate class instance based on type class
Type classes are able to instantiate it's original classes instances in runtime.

### Move class instance
You can invoke move constructor for some place in memory. Useful if you're working on raw memory and want to move class instance between buffers.
This is an optional feature since it forces all classes to have move constructor. To enable it set `RTTI_REQUIRE_MOVE_CTOR` to `1`

### Recognizing object true type <br/>
You can get the true type of your polymorphic class and get some additional information about it, compare it etc.
Have a look on code examples to see what data about your type you can get.

### Types register
You have access to the list of all registered types.

### Unique and persistent IDs
All registered types get unique IDs which persist between executions until the name of the type changes. Might be used for serialization.

# Primitive Types
To get primitive's type use `rtti::PrimitiveType< <primitive_type_name> >::GetInstance()`.
All primitive types are registered out of the box. However, if I skipped something, you can add new one by adding `DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( <primitive_type_name> )` to your header.

# Code Examples
* Non-virtual classes
```cpp
// .h
class A
{
  DECLARE_CLASS( A )
};

// .cpp
IMPLEMENT_TYPE( A )
```

* Virtual classes
```cpp
// .h
class A
{
  DECLARE_POLYMORPHIC_CLASS( A )
  virtual ~A() = default;
};

// .cpp
IMPLEMENT_TYPE( A )
```

* Structs
```cpp
// .h
struct A
{
  DECLARE_STRUCT( A )
};

// .cpp
IMPLEMENT_TYPE( A )
```

* Abstract class
```cpp
// .h
class I
{
DECLARE_ABSTRACT_CLASS( I );
public:
    virtual ~I() = 0;
}

// .cpp
IMPLEMENT_TYPE( I );
```

* Class in namespace
```cpp
// .h
namespace a
{
  class A
  {
    DECLARE_CLASS( A )
  };
}

// .cpp
IMPLEMENT_TYPE( a::A )
```

* Inheritance
```cpp
// .h
namespace a
{
  class A;
}

class B : public A
{
  DECLARE_CLASS( B, a::A )
}

namespace c
{
  class C : public B
  {
    DECLARE_POLYMORPHIC_CLASS( C, B )
    virtual ~C() = default;
  };
}

class D : public C
{
    DECLARE_POLYMORPHIC_CLASS( D, c::C )
}

// .cpp
IMPLEMENT_TYPE( a::A );
IMPLEMENT_TYPE( B );
IMPLEMENT_TYPE( c::C );
IMPLEMENT_TYPE( D );
```

* Type information
```cpp
class A
{
  DECLARE_CLASS( A )
};
namespace b
{
  class B : public A 
  {
    DECLARE_CLASS( B, A )
  };
}

class C
{
  DECLARE_POLYMORPHIC_CLASS( C )
};
namespace d
{
  class D : public C 
  {
    DECLARE_POLYMORPHIC_CLASS( D, C )
  };
}

A* b = new b::B();
C* d = new d::D();

const rtti::IType& aType = A::GetTypeStatic();
const rtti::IType& bType = b->GetType();
bType == aType; // true, because b is A* and both A and B are not virtual classes

const rtti::IType& cType = C::GetTypeStatic();
const rtti::IType& dType = d->GetType();
cType == dType; // false, d is C*, but both C and D are virtual classes

aType.GetName(); // "A"
bType.GetName(); // "A"
cType.GetName(); // "C"
dType.GetName(); // "D"

bType.GetHash(); // unique hash for each type
bType.IsAbstract(); // false
bType.IsVirtual(); // false
dType.IsVirtual(); // true

dType.IsA< d::D >(); // true
dType.IsA< C >(); // false
dType.IsA( cType ); // false

dType.InheritsFrom< d::D >(); // false
dType.InheritsFrom< C >(); // true
dType.InheritsFrom( cType ); //true

d->IsA< d::D >(); // true
d->IsA< C >(); // false

d->InheritsFrom< d::D >(); // false
d->InheritsFrom< C >(); // true

b.InheritsFromOrIsA< d::D >(); // true
b.InheritsFromOrIsA< C >(); // true
```

* Instantiating class default object
``` cpp
const C::Type* baseType = &C::GetStaticType();

std::unique_ptr< C > c = baseType->Construct(); // c is instance of C

baseType = &d::D::GetStaticType();
std::unique_ptr< C > d = baseType->Construct(); // d is instance of D

void* buffer = new char[baseType->GetSize()]; // GetSize returns the size of the type
baseType->ConstructInPlace( buffer ); // Constructs object in selected place
baseType->Destroy( buffer ); // Destroys object in selected place
```

* Moving class instance between buffers (optional feature)
``` cpp
#define RTTI_REQUIRE_MOVE_CTOR 1

const C::Type* baseType = &C::GetStaticType();

void* buffer0 = new char[baseType->GetSize()]; // GetSize returns the size of the type
void* buffer1 = new char[baseType->GetSize()]; // GetSize returns the size of the type
baseType->ConstructInPlace( buffer0 ); // Constructs object in selected place
baseType->MoveInPlace( buffer1, buffer0 ); // Moves object from buffer0 to buffer1

baseType->Destroy( buffer0 ); // Destroys object in selected place
baseType->Destroy( buffer1 ); // Destroys object in selected place
```

* Types register
``` cpp
const auto& allTypes = rtti::Get().GetTypes();
```

* Primitive types
``` cpp
const rtti::IType& floatType = ::rtti::PrimitiveType< float >::GetInstance();
const rtti::IType& boolType = ::rtti::PrimitiveType< bool >::GetInstance();
```

* IDs
``` cpp
const rtti::IType& type = A::GetTypeStatic();
size_t uniqueAndPersistentID = type.GetID();
```
