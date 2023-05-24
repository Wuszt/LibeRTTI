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
#include <vector>
#include <memory>
#include <type_traits>

#define RTTI_REQUIRE_MOVE_CTOR 1

namespace rtti
{
	class RTTI
	{
		friend class Type;

	public:
		const std::vector< const Type* >& GetTypes() const
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

		bool IsTypeRegistered( const Type& testType );

		void RegisterType( const Type& instance )
		{
			if ( IsTypeRegistered( instance ) )
			{
				// Your types have to be unique! You probably declared 2 classes with the same name in 2 different TUs.
				throw;
			}

			m_types.emplace_back( &instance );
		}

		std::vector< const Type* > m_types;
	};

	static auto Get = RTTI::Get;

	template< class >
	class Property;

	class IProperty
	{
	public:
		virtual ~IProperty() = default;

		virtual const char* GetName() const = 0;
		virtual const Type& GetType() const = 0;
		virtual size_t GetOffset() const = 0;
		virtual size_t GetID() const = 0;

		template< class T >
		const Property< T >* Cast() const
		{
			return dynamic_cast< const Property< T >* >( this );
		}
	};

	template< class T >
	class Property : public IProperty
	{
		friend class Type;

	public:
		virtual const char* GetName() const override
		{
			return m_name;
		}

		virtual const Type& GetType() const override
		{
			return m_type;
		}

		virtual size_t GetOffset() const override
		{
			return m_offset;
		}

		virtual size_t GetID() const override 
		{
			return m_id;
		}

		void* GetAddress( void* owner ) const
		{
			return static_cast< char* >( owner ) + GetOffset();
		}

		T& GetValue( void* owner ) const
		{
			return *( static_cast< T* >( GetAddress( owner ) ) );
		}

		void SetValue( void* owner, const T& value ) const
		{
			 GetValue( owner ) = value;
		}

	private:
		Property( const char* name, size_t offset, size_t id )
			: m_name( name )
			, m_type( GetTypeOf< T >() )
			, m_offset( offset )
			, m_id( id )
		{}

		size_t m_id = 0u;
		const char* m_name = nullptr;
		const Type& m_type;
		size_t m_offset = 0u;
	};

	template< typename T, std::enable_if_t< !std::is_arithmetic_v< T >, bool > = true >
	const Type& GetTypeOf()
	{
		return T::Type::GetInstance();
	}

	template< typename T, std::enable_if_t< std::is_arithmetic_v< T >, bool > = true >
	const Type& GetTypeOf()
	{
		return PrimitiveType< T >::GetInstance();
	}

	class Type
	{
	public:
		virtual ~Type() = default;
		virtual const char* GetName() const = 0;

		bool IsA( const Type& type ) const
		{
			return GetID() == type.GetID();
		}

		template< class T >
		bool IsA() const
		{
			return IsA( T::GetTypeStatic() );
		}

		virtual bool InheritsFrom( const Type& type ) const = 0;

		template< class T >
		bool InheritsFrom() const
		{
			return InheritsFrom( T::GetTypeStatic() );
		}

		template< class T >
		static bool InheritsFromStatic()
		{
			return false;
		}

		bool operator==( const Type& rhl ) const
		{
			return IsA( rhl );
		}

		bool operator!=( const Type& rhl ) const
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

		virtual bool IsAbstract() const
		{
			return false;
		}

		virtual bool IsVirtual() const
		{
			return false;
		}

		size_t GetID() const
		{
			return m_id;
		}

		virtual size_t GetSize() const = 0;

		virtual size_t GetPropertiesAmount() const { return 0u; }
		virtual const IProperty* GetProperty( size_t index ) const { return nullptr; }

		const IProperty* FindProperty( size_t wantedId ) const
		{
			for ( size_t i = 0u; i < GetPropertiesAmount(); ++i )
			{
				const IProperty* property = GetProperty( i );
				if ( property->GetID() == wantedId )
				{
					return property;
				}
			}

			return nullptr;
		}

		const IProperty* FindProperty( const char* name ) const
		{
			return FindProperty( CalcHash( name ) );
		}

	protected:
		Type( const char* name )
			: m_id( CalcHash( name ) )
		{
			RTTI::GetMutable().RegisterType( *this );
		}

		static size_t GetPropertiesAmountStatic() { return 0u; }
		static const ::rtti::IProperty* GetPropertyStatic( size_t index ) { return nullptr; }

		// Java's hashCode for String
		static size_t CalcHash( const char* name )
		{
			size_t result = 0u;

			for ( size_t i = 0u; name[ i ] != 0; ++i )
			{
				result = name[ i ] + result * 31;
			}

			return result;
		}

		template< class T >
		static std::unique_ptr< ::rtti::IProperty > CreateProperty( const char* name, size_t offset )
		{
			return std::unique_ptr< ::rtti::Property< T > >( new ::rtti::Property< T >( name, offset, CalcHash( name ) ) );
		}

	private:
		size_t m_id = 0u;
	};

	inline bool RTTI::IsTypeRegistered( const Type& testType )
	{
		for ( const Type* type : m_types )
		{
			if ( *type == testType )
			{
				return true;
			}
		}

		return false;
	}

	template< class T >
	class PrimitiveType : public Type
	{
		static_assert( std::is_arithmetic_v< T >, "That's not a primitive type!" );

	public:
		virtual const char* GetName() const;
		virtual bool InheritsFrom( const Type& type ) const { return false; }
		virtual void ConstructInPlace( void* dest ) const { *static_cast< T* >( dest ) = T(); }
#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const { std::memcpy( dest, src, sizeof( T ) ); }
#endif
		virtual void Destroy( void* ptr ) const {}
		virtual size_t GetSize() const { return sizeof( T ); }
		virtual bool IsPrimitive() const { return true; }

		static const rtti::PrimitiveType< T >& GetInstance()
		{
			static rtti::PrimitiveType< T > s_typeInstance;
			return s_typeInstance;
		}

	private:
		PrimitiveType< T >() : rtti::Type( GetName() ) {}
	};
}

#define INHERITS_FROM_BODY_true( ParentClassName ) return ParentClassName##::GetTypeStatic().IsA( type ) || ParentClassName##::GetTypeStatic().InheritsFrom( type );
#define INHERITS_FROM_BODY_false( ParentClassName ) return false;

#define VIRTUAL_true virtual
#define VIRTUAL_false

#define INHERITS_FROM_STATIC_BODY_true return Super::GetTypeStatic().IsA< T >() || Super::InheritsFromStatic< T >();
#define INHERITS_FROM_STATIC_BODY_false return false;

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
class Type : public ParentClassName##::Type \
{ \
public: \
	virtual const char* GetName() const override; \
	virtual bool InheritsFrom( const ::rtti::Type& type ) const override \
	{ \
		INHERITS_FROM_BODY_##Inherits##( ParentClassName ) \
	} \
	template< class T > \
	bool InheritsFrom() const \
	{ \
		return ::rtti::Type::InheritsFrom< T >(); \
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
	virtual bool IsAbstract() const override \
	{ \
		return Abstract; \
	} \
	virtual bool IsVirtual() const override \
	{ \
		return Virtual; \
	} \
	virtual size_t GetSize() const override \
	{ \
		return sizeof( ClassName ); \
	} \
	static const Type& GetInstance() \
	{ \
		static Type s_typeInstance; \
		return s_typeInstance; \
	} \
	virtual size_t GetPropertiesAmount() const override { return ParentClassName##::Type::GetPropertiesAmountStatic() + m_properties.size(); } \
	virtual const ::rtti::IProperty* GetProperty( size_t index ) const override \
	{ \
		if( index < ParentClassName##::Type::GetPropertiesAmountStatic() ) \
		{ \
			return ParentClassName##::Type::GetPropertyStatic( index ); \
		} \
		else \
		{ \
			return m_properties[ index - ParentClassName##::Type::GetPropertiesAmountStatic() ].get(); \
		} \
	} \
protected: \
	VIRTUAL_##Virtual ClassName##* Construct_Internal() const \
	{ \
		CONSTRUCT_INTERNAL_BODY_##Abstract##( ClassName ) \
	} \
	Type(); \
	Type( const char* name ) : ParentClassName##::Type ( name ) {} \
	static size_t GetPropertiesAmountStatic() { return GetInstance().GetPropertiesAmount(); } \
	static const ::rtti::IProperty* GetPropertyStatic( size_t index ) { return GetInstance().GetProperty( index ); } \
private: \
	std::vector< std::unique_ptr< ::rtti::IProperty > > m_properties; \
}; \
	static const Type& GetTypeStatic() \
	{ \
		return Type::GetInstance(); \
	} \
	template< class T > \
	bool IsA() const \
	{ \
		return static_cast<const ::rtti::Type&>( GetType() ).IsA< T >(); \
	} \
	template< class T > \
	bool InheritsFrom() const \
	{ \
		return static_cast<const ::rtti::Type&>( GetType() ).InheritsFrom< T >(); \
	} \
	template< class T > \
	bool InheritsFromOrIsA() const \
	{ \
		return IsA< T >() || InheritsFrom< T >(); \
	} \
	VIRTUAL_##Virtual const Type& GetType() const \
	{ \
		return GetTypeStatic(); \
	} \
	template< class T > \
	static bool InheritsFromOrIsAStatic() \
	{ \
		return GetTypeStatic().IsA< T >() || InheritsFromStatic< T >(); \
	} \
	template< class T > \
	static bool InheritsFromStatic() \
	{ \
		INHERITS_FROM_STATIC_BODY_##Inherits; \
	} \
	using ClassType = Type; \
private:

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

#define IMPLEMENT_TYPE( NamespaceClassName, ... ) \
NamespaceClassName##::Type::Type() : NamespaceClassName##::Type::Type( GetName() ) \
{ \
	using CurrentlyImplementedType = NamespaceClassName##; \
	__VA_ARGS__ \
} \
const char* NamespaceClassName##::Type::GetName() const \
{ \
	return #NamespaceClassName; \
} \
CREATE_TYPE_INSTANCE( NamespaceClassName##::Type )

#define REGISTER_PROPERTY( PropertyName ) m_properties.emplace_back( CreateProperty< decltype( CurrentlyImplementedType::##PropertyName ) >( #PropertyName, offsetof( CurrentlyImplementedType, PropertyName ) ) );

#define DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( type ) \
template<> \
inline const char* ::rtti::PrimitiveType< type >::GetName() const \
{ \
	return #type; \
} \
template<> \
inline bool rtti::Type::IsA< type >() const \
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