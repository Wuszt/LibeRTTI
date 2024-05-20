<p align="center">
  <img src="Logo.png" height = 300>
</p>


<h3 align="center"> Liberate your C++ with reflection! </h3>
<br>
<br>

LibeRTTI is a header only and dependency-free library which provides type data for your (and selected built-in) c++ types. The minimal requirement is C++14.

# Features :statue_of_liberty:

| Feature                          | Description   |
| :---                             | :---          |
| **Hierarchy based** | Every registered type gets its unique nested type class, e.g if you have a `class A` then it's type class will be `A::Type`. These type classes create a hierarchy based on their original classes hierarchy. So for instance, if `class A : public B`, then `class A::Type : class B::Type`. Also, all type classes are virtual, even if their original classes are not. It allows you to take advantage of polymorphic features like having a function parameter of type `const A::Type&` and passing the children types of A as arguments. `rtti:Type` is the root class for all types classes. |
| **"Super" keyword** | A well known feature from other languages. The `Super` keyword refers to the parent class of your class |
| **Instantiating/Destroying/Copying without knowing true type** | Type classes are able to instantiate, destroy or copy raw memory which contains the types they represent. |
| **Move without knowing true type (Optional)** | Same as above but with the move operation. It forces all registered types to be movable. It might be disabled by defining `RTTI_REQUIRE_MOVE_CTOR 0` before including the **LibeRTTI** header. |
| **Recognizing object's true type** | You can get the true type of your polymorphic class instance. |
| **Types register** | **LibeRTTI** gives you access to all registered types. |
| **Properties and Methods** | Types might keep data about the their member variables and methods to make them accessible in runtime. |
| **Unique and persistent IDs** | All registered types and their properties get unique IDs which persist between executions unless the name of the type/property changes. |
| **Primitive Types** | All primitive types are registered out of the box. |
| **Pointer Types** | Pointer types are registered lazily at runtime when the need for them arrises. You're not limited by the amount of indirections (properties like `float*****` are allowed). Pointer types follow their original classes hierarchy. I.e `PointerType<B>` inherits from `PointerType<A>` if `B` also inherits from `A`. |
| **Runtime Types** | You can compose completely new type with selected properties in runtime. Such types can still inherit from other types and preserve hierarchy of classes.
| **std:: <br /> shared_ptr <br /> unique_ptr <br /> vector <br />unordered_set <br />unordered_map <br />pair <br /> Types (Optional)** | All these types are registered out of the box and might be disabled using config defines: <br />`RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE 0`, <br />`RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE 0`, <br /> `RTTI_CFG_CREATE_STD_VECTOR_TYPE 0`, <br /> `RTTI_CFG_CREATE_STD_SET_TYPE 0`,<br /> `RTTI_CFG_CREATE_STD_MAP_TYPE 0`,<br /> `RTTI_CFG_CREATE_STD_PAIR_TYPE 0` <br />  &#8205;  |

# Demo :statue_of_liberty:
```cpp
// .h
struct BaseStruct
{
  RTTI_DECLARE_STRUCT( BaseStruct );
};

struct MyStruct : public BaseStruct
{
  RTTI_DECLARE_STRUCT( MyStruct, BaseStruct );

  float myFloat;
  MyStruct* myPtr;
  BaseStruct myCustomInstance;

  MyStruct* Foo( Float input )
  {
    myFloat = input;
    return this;
  }
}

// .cpp
RTTI_IMPLEMENT_TYPE( BaseStruct );
RTTI_IMPLEMENT_TYPE( MyStruct,
  RTTI_REGISTER_PROPERTY( myFloat );
  RTTI_REGISTER_PROPERTY( myPtr );
  RTTI_REGISTER_PROPERTY( myCustomInstance );
  RTTI_REGISTER_METHOD( Foo );
);

static_assert( MyStruct::GetTypeStatic().InheritsFrom< BaseStruct::Type > );

void Func( void* rawInstance, const rtti::Type& type )
{
  if( type.IsA< MyType >() )
  {
    const MyStruct::Type& myType = static_cast< const MyStruct::Type& >( type );

    // Accessing properties
    const rtti::Property* floatProperty = myType.FindProperty( "myFloat" );
    if ( floatProperty->GetType().IsA< float >() )
    {
      floatProperty->GetValue< float >( rawInstance ) = 3.14f;
    }
    myType.FindProperty( "myCustomInstance" )->GetValue< BaseStruct >( rawInstance ) = BaseStruct();

    // Calling methods
    MyType* fooResult = nullptr;
    float arg = 3.14f;
    myType.FindMethod( "Foo" )->Call( /*instance*/ rawInstance, /*args*/ &arg, /*outcome*/ &fooResult );
  }
}
```

# Usage :statue_of_liberty:
Registering your type requires 2 steps.
## 1. Declare type
Put one of the following macros into your type's body.
| Type                             | Macro   |
| :---                             | :---          |
| Regular, non virtual class | <pre lang=cpp> RTTI_DECLARE_CLASS( <class_name>, <parent_name_with_namespace> (optional) ) </pre>
|                             |        |
| Virtual class | <pre lang=cpp> RTTI_DECLARE_POLYMORPHIC_CLASS( <class_name>, <parent_name_with_namespace> (optional) ) </pre>
|                            |          |
| Abstract class | <pre lang=cpp> RTTI_DECLARE_ABSTRACT_CLASS( <class_name>, <parent_name_with_namespace> (optional) ) </pre>
|                            |        |
| Struct |  <pre lang=cpp> RTTI_DECLARE_STRUCT( <struct_name>, <parent_name_with_namespace> (optional) ) </pre>
## 2. Implement type
Put the following macro in .cpp file.
```cpp
RTTI_IMPLEMENT_TYPE( <class_name_with_namespace>, <properties_registration_macros>... (optional> )
```
To register property of your type use:
```cpp
RTTI_REGISTER_PROPERTY( <property_name> )
```
For methods use:
RTTI_REGISTER_METHOD( <method_name> )
and put it in `RTTI_IMPLEMENT_TYPE` macro.

---
Examples can be found in [**Tests**](https://github.com/Wuszt/LibeRTTI/tree/main/Tests) or [**Forge project**](https://github.com/Wuszt/Forge) source code.
