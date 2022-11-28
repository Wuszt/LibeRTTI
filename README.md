# Mini-RTTI
Mini-RTTI is a small, header only way to provide type data for your classes and structs.

# Usage
First you have to put one of four macros in your class body
* For regular classes use: `DECLARE_CLASS( <class_name>, <parent_namespaces> (optional), <parent_name> (optional) )`
* If your class is abstract use: `DECLARE_ABSTRACT_CLASS( <class_name>, <parent_namespaces> (optional), <parent_name> (optional) )`
* If your class is a polymorphic base for other class, but is not inheritting itself: `DECLARE_POLYMORPHIC_BASE_CLASS( <class_name> )`
* For structs use `DECLARE_STRUCT( <struct_name>, <parent_namespaces> (optional), <parent_name> (optional) )`

Then you need to put `IMPLEMENT_TYPE( <type_namespaces>(optional), <class_name> )` in your .cpp file.


That's it, you can now get some info about your type in the runtime.

# What can I do with that?
### Hierarchy <br/>
Every registered type gets it's unique type class with the name of it's owner class + "Type" postfix, e.g if you have a `class A` then it's type class is `class AType`. These type classes creates a hierarchy based on their original classes hierarchy.
So for instance, if `class A : public B`, then `class AType : class BType`. It allows you to take advantage of polymorphic features like parameters of a base class type.

### Access parent class without literally typing it <br/>
Very often you might need to invoke some parent class method, but C++ doesn't provide out of the box way to know what your parent is.
In such cases you can use `Super` in your class which you likely know from other languages.

### Recognizing object true type <br/>
You can get the true type of your polymorphic class and get some additional information about it, compare it etc.
Have a look on code examples to see what data about your type you can get.


# Code Examples
* Classes
```
// .h
class A
{
  DECLARE_CLASS( A )
};

// .cpp
IMPLEMENT_TYPE( A )
```

* Structs
```
// .h
struct A
{
  DECLARE_STRUCT( A )
};

// .cpp
IMPLEMENT_TYPE( A )
```

* Abstract class
```
// .h
class I
{
DECLARE_ABSTRACT_CLASS( IAsset );
public:
    virtual ~I() = 0;
}

// .cpp
IMPLEMENT_TYPE( IAsset );
```

* Class in namespace
```
// .h
namespace a
{
  class A
  {
    DECLARE_CLASS( A )
  };
}

// .cpp
IMPLEMENT_TYPE( a, A )
```

* Polymorphic base class
```
// .h
class A
{
  DECLARE_POLYMORPHIC_BASE_CLASS( A )
  public:
    	virtual ~A() = default;
};

// .cpp
IMPLEMENT_TYPE( A )
```

* Inheritance
```
// .h
namespace a
{
  class A;
}

class B : public A
{
  DECLARE_CLASS( B, a, A )
}

namespace c
{
  class C : public B
  {
    DECLARE_CLASS( C, B )
  };
}

// .cpp
IMPLEMENT_TYPE( a, A )
IMPLEMENT_TYPE( B )
IMPLEMENT_TYPE( c, C )
```

* Type information
```
namespace b
{
  class B : public A;
}

A* b = new B();

const IType& aType = A::GetTypeStatic();
const IType& bType = b->GetType();
bType.GetName( /*withNamespaces*/ true ); // "b::B"
bType.GetName( /*withNamespaces*/ false ); // "B"
bType.GetHash(); // unique hash for each type
bType.IsAbstract();

bType.IsA< B >(); // true
bType.IsA< A >(); // false
bType.IsA( aType ); // false

bType.InheritsFrom< B >(); // false
bType.InheritsFrom< A >(); // true
bType.InheritsFrom( aType ); //true

b.IsA< B >(); // true
b.IsA< A >(); // false

b.InheritsFrom< B >(); // false
b.InheritsFrom< A >(); // true

b.InheritsFromOrIsA< B >(); // true
b.InheritsFromOrIsA< A >(); // true

```
