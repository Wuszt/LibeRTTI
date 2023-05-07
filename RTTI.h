/**
 * MIT License
 *
 * Copyright (c) 2022 Jakub Kuznik
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#define RTTI_REQUIRE_MOVE_CTOR 1

namespace rtti
{
	class RTTI
	{
	friend class IType;

	public:
		const std::vector< const IType* >& GetTypes() const
		{
			return m_types;
		}

		static const RTTI& Get()
		{
			return GetMutable();
		}

	private:
		static RTTI& GetMutable()
		{
			static RTTI s_rtti;
			return s_rtti;
		}

		void RegisterType( const IType& instance )
		{
			m_types.emplace_back( &instance );
		}

		std::vector< const IType* > m_types;
	};

	static auto Get = RTTI::Get;

	class IType
	{
	public:
		virtual ~IType() = default;
		virtual const char* GetName() const = 0;

		Bool IsA( const IType& type ) const
		{
			return this == &type;
		}

		template< class T >
		Bool IsA() const
		{
			return IsA( T::GetTypeStatic() );
		}

		virtual Bool InheritsFrom( const IType& type ) const = 0;

		template< class T >
		Bool InheritsFrom() const
		{
			return InheritsFrom( T::GetTypeStatic() );
		}

		template< class T >
		static Bool InheritsFromStatic()
		{
			return false;
		}

		Bool operator==( const IType& rhl ) const
		{
			return IsA( rhl );
		}

		Bool operator!=( const IType& rhl ) const
		{
			return !IsA( rhl );
		}

		virtual bool IsPrimitive() const
		{
			return false;
		}

		virtual void ConstructInPlace( void* dest ) const = 0;

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const = 0;
#endif
		virtual void Destroy( void* ptr ) const = 0;

		virtual Bool IsAbstract() const
		{
			return false;
		}

		virtual Bool IsVirtual() const
		{
			return false;
		}

		Uint64 GetHash() const
		{
			return reinterpret_cast< Uint64 >( this );
		}

		virtual Uint32 GetSize() const = 0;

	protected:
		IType()
		{
			RTTI::GetMutable().RegisterType( *this );
		}
	};

	template< class T >
	class PrimitiveType : public IType
	{
	public:
		virtual const char* GetName() const;
		virtual Bool InheritsFrom( const IType& type ) const { return false; }
		virtual void ConstructInPlace( void* dest ) const { *static_cast< T* >( dest ) = T(); }
#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const { std::memcpy( dest, src, sizeof( T ) ); }
#endif
		virtual void Destroy( void* ptr ) const {}
		virtual Uint32 GetSize() const { return sizeof( T ); }
		virtual bool IsPrimitive() const { return true; }

		static const rtti::PrimitiveType< T >& GetInstance()
		{
			static rtti::PrimitiveType< T > s_typeInstance;
			return s_typeInstance;
		}

	private:
		PrimitiveType< T >() = default;
	};
}

#define INHERITS_FROM_BODY_true( ParentClassName ) return ParentClassName##::GetTypeStatic().IsA( type ) || ParentClassName##::GetTypeStatic().InheritsFrom( type );
#define INHERITS_FROM_BODY_false( ParentClassName ) return false;

#define VIRTUAL_true virtual
#define VIRTUAL_false

#define INHERITS_FROM_STATIC_BODY_true return Super::GetTypeStatic().IsA< T >() || Super::InheritsFromStatic< T >();
#define INHERITS_FROM_STATIC_BODY_false return false;

#define TYPE_CLASS_NAME_true Type
#define TYPE_CLASS_NAME_false IType

#define CONSTRUCT_INTERNAL_BODY_true( ClassName ) return nullptr;
#define CONSTRUCT_INTERNAL_BODY_false( ClassName ) return new ClassName##();

#define CONSTRUCT_IN_PLACE_INTERNAL_BODY_true( ClassName, dest )
#define CONSTRUCT_IN_PLACE_INTERNAL_BODY_false( ClassName, dest ) new (##dest##) ClassName##();

#if RTTI_REQUIRE_MOVE_CTOR
	#define MOVE_IN_PLACE_INTERNAL_true( ClassName ) virtual void MoveInPlace( void* dest, void* src  ) const override {}
	#define MOVE_IN_PLACE_INTERNAL_false( ClassName ) virtual void MoveInPlace( void* dest, void* src ) const override { new (dest) ClassName##(std::move( *static_cast< ClassName##* >( src ) ) ); }
#else
	#define MOVE_IN_PLACE_INTERNAL_true( ClassName )
	#define MOVE_IN_PLACE_INTERNAL_false( ClassName )
#endif

#define DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, Inherits, Virtual, Abstract ) \
public: \
class Type : public ParentClassName##::##TYPE_CLASS_NAME_##Inherits## \
{ \
public: \
	virtual const char* GetName() const override; \
	virtual Bool InheritsFrom( const ::rtti::IType& type ) const override \
	{ \
		INHERITS_FROM_BODY_##Inherits##( ParentClassName ) \
	} \
	template< class T > \
	Bool InheritsFrom() const \
	{ \
		return ::rtti::IType::InheritsFrom< T >(); \
	} \
	std::unique_ptr< ClassName > Construct() const \
	{ \
		return std::unique_ptr< ClassName >( Construct_Internal() ); \
	} \
	virtual void ConstructInPlace( void* dest ) const override \
	{ \
		CONSTRUCT_IN_PLACE_INTERNAL_BODY_##Abstract##( ClassName, dest )\
	} \
	MOVE_IN_PLACE_INTERNAL_##Abstract##( ClassName ) \
	virtual void Destroy( void* ptr ) const override \
	{ \
		static_cast< ClassName##* >( ptr )->~##ClassName##(); \
	} \
	virtual Bool IsAbstract() const override \
	{ \
		return Abstract; \
	} \
	virtual Bool IsVirtual() const override \
	{ \
		return Virtual; \
	} \
	virtual Uint32 GetSize() const override \
	{ \
		return sizeof( ClassName ); \
	} \
	static const Type& GetInstance() \
	{ \
		static Type s_typeInstance; \
		return s_typeInstance; \
	} \
protected: \
	VIRTUAL_##Virtual ClassName##* Construct_Internal() const \
	{ \
		CONSTRUCT_INTERNAL_BODY_##Abstract##( ClassName ) \
	} \
	Type() = default; \
}; \
	static const Type& GetTypeStatic() \
	{ \
		return Type::GetInstance(); \
	} \
	template< class T > \
	Bool IsA() const \
	{ \
		return static_cast<const ::rtti::IType&>( GetType() ).IsA< T >(); \
	} \
	template< class T > \
	Bool InheritsFrom() const \
	{ \
		return static_cast<const ::rtti::IType&>( GetType() ).InheritsFrom< T >(); \
	} \
	template< class T > \
	Bool InheritsFromOrIsA() const \
	{ \
		return IsA< T >() || InheritsFrom< T >(); \
	} \
	VIRTUAL_##Virtual const Type& GetType() const \
	{ \
		return GetTypeStatic(); \
	} \
	template< class T > \
	static Bool InheritsFromOrIsAStatic() \
	{ \
		return GetTypeStatic().IsA< T >() || InheritsFromStatic< T >(); \
	} \
	template< class T > \
	static Bool InheritsFromStatic() \
	{ \
		INHERITS_FROM_STATIC_BODY_##Inherits; \
	} \
	using ClassType = Type; \
private: \

#define DECLARE_TYPE_INTERNAL( ClassName, Virtual ) \
DECLARE_TYPE_INTERNAL_PARENT( ClassName, ::rtti, false, Virtual, false)

#define DECLARE_TYPE_INTERNAL_PARENT_DIRECT( ClassName, ParentClassName, Virtual ) \
DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, true, Virtual, false) \
	using Super = ParentClassName;

#define EXPAND( x ) x

#define GET_DECLARE_TYPE_MACRO(_1,_2,NAME,...) NAME

#define DECLARE_POLYMORPHIC_CLASS(...) EXPAND(GET_DECLARE_TYPE_MACRO(__VA_ARGS__, DECLARE_TYPE_INTERNAL_PARENT_DIRECT, DECLARE_TYPE_INTERNAL)(__VA_ARGS__, true))

#define DECLARE_CLASS(...) EXPAND(GET_DECLARE_TYPE_MACRO(__VA_ARGS__, DECLARE_TYPE_INTERNAL_PARENT_DIRECT, DECLARE_TYPE_INTERNAL)(__VA_ARGS__, false))

#define DECLARE_STRUCT(...) DECLARE_CLASS(__VA_ARGS__) \
public:

#define DECLARE_ABSTRACT_TYPE_INTERNAL( ClassName ) \
DECLARE_TYPE_INTERNAL_PARENT( ClassName, ::rtti, false, true, true)

#define DECLARE_ABSTRACT_TYPE_INTERNAL_PARENT_DIRECT( ClassName, ParentClassName ) \
DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, true, true, true) \
	using Super = ParentClassName;

#define DECLARE_ABSTRACT_CLASS( ... ) EXPAND( GET_DECLARE_TYPE_MACRO( __VA_ARGS__, DECLARE_ABSTRACT_TYPE_INTERNAL_PARENT_DIRECT, DECLARE_ABSTRACT_TYPE_INTERNAL )( __VA_ARGS__ ) )

#define CONCAT(X, Y) CONCAT2(X, Y)
#define CONCAT2(X,Y) X##Y

#define CREATE_TYPE_INSTANCE( TypeClass ) \
namespace rtti_internal \
{ \
static const void* CONCAT( s_typeInstance_, __COUNTER__ ) = &TypeClass::GetInstance(); \
}

#define IMPLEMENT_TYPE( NamespaceClassName ) \
const char* NamespaceClassName##::Type::GetName() const \
{ \
	return #NamespaceClassName; \
} \
CREATE_TYPE_INSTANCE( NamespaceClassName##::Type )

#define DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( type ) \
template<> \
inline const char* ::rtti::PrimitiveType< type >::GetName() const \
{ \
	return #type; \
} \
template<> \
inline Bool rtti::IType::IsA< type >() const \
{ \
	return IsA( ::rtti::PrimitiveType< type >::GetInstance() ); \
} \
CREATE_TYPE_INSTANCE( ::rtti::PrimitiveType<##type##> )

DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( float )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int32 )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int64 )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( bool )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned short )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned long long )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( double )
DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int8 )