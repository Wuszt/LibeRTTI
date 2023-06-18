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

#ifndef RTTI_REQUIRE_MOVE_CTOR
#define RTTI_REQUIRE_MOVE_CTOR 1
#endif

#include <vector>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <string>

#pragma region ForwardDeclarations
namespace rtti
{
class Type;
template< class T, class T2 = void > class PointerType;
template< class T > class VectorType;
template< class T, size_t Count > class ArrayType;
template< class T > class PrimitiveType;
using TypeId = size_t;
}
#pragma endregion

#pragma region TypeOf
namespace rtti
{
	namespace internal
	{
		template <typename> struct is_template : std::false_type {};

		template <template <typename...> class Tmpl, typename ...Args>
		struct is_template<Tmpl<Args...>> : std::true_type {};

		template< class T >
		struct is_vector : std::false_type {};

		template< class T >
		struct is_vector< std::vector< T > > : std::true_type {};

		template<class T>
		struct is_array : std::false_type {};

		template<class T, std::size_t N>
		struct is_array<T[ N ]> : std::true_type {};

		template<class T, std::size_t N>
		struct is_array< std::array< T, N > > : std::true_type {};
	}

	template< class T, class T2 = void >
	struct type_of {};

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_class_v< T > && !internal::is_template< T >::value && !internal::is_array< T >::value > > { using type = typename T::Type; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_fundamental_v< T > > > { using type = PrimitiveType< T >; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_pointer_v< T > > > { using type = PointerType< std::remove_const_t< std::remove_pointer_t< T > > >; };

	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_vector< T >::value > > { using type = VectorType< typename T::value_type >; };

	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_array< T >::value > >{ private: using internalType = std::remove_reference_t< decltype( *std::begin( std::declval< T& >() ) ) >; public: using type = ArrayType< internalType, sizeof( T ) / sizeof( internalType ) >; };

	template< class T >
	const typename type_of< T >::type& GetTypeInstanceOf()
	{
		return type_of< T >::type::GetInstance();
	}
}
#pragma endregion

#pragma region InternalHelpers
namespace rtti
{
	namespace internal
	{
		// Java's hashCode for String
		static constexpr size_t CalcHash( const char* name, size_t seed = 0u )
		{
			for ( size_t i = 0u; name[ i ] != 0; ++i )
			{
				seed = name[ i ] + seed * 31;
			}

			return seed;
		}

		template< class T >
		class has_CalcId
		{
			using yes = char;
			using no = char[ 2 ];

			template <typename U, U> struct really_has;

			template<typename C> static yes& Test( really_has < TypeId(), &C::CalcId >* );
			template<typename> static no& Test( ... );

		public:
			static constexpr bool value = sizeof( Test<T>( 0 ) ) == sizeof( yes );
		};
	}
}
#pragma endregion

#pragma region RTTIClass
namespace rtti
{
	class RTTI
	{
	public:
		void GetTypes( std::vector< const Type* >& outTypes ) const
		{
			outTypes.reserve( m_types.size() );
			for ( const auto& entry : m_types )
			{
				outTypes.emplace_back( entry.second.get() );
			}
		}

		static const RTTI& Get()
		{
			return GetMutable();
		}

		static RTTI& GetMutable()
		{
			static RTTI s_rtti;
			return s_rtti;
		}

		template< class T, class... TArgs, std::enable_if_t< internal::has_CalcId< T >::value, bool > = true >
		const T& GetOrRegisterType( const TArgs& ... args )
		{
			TypeId id = T::CalcId( args... );
			auto found = m_types.find( id );

			if ( found != m_types.end() )
			{
				return static_cast< const T& >( *found->second );
			}

			T* instance = new T( std::forward< TArgs >( args )... );
			m_types.emplace( id, instance );

			instance->OnRegistered();

			return *instance;
		}

		template< class T, class... TArgs, std::enable_if_t< !internal::has_CalcId< T >::value, bool > = true >
		const T& GetOrRegisterType( const TArgs& ... args )
		{
			std::unique_ptr< T > instance( new T( std::forward< TArgs >( args )... ) );

			auto currentInstance = m_types.find( instance->GetID() );

			if ( currentInstance == m_types.end() )
			{
				T& result = *instance;
				currentInstance = m_types.emplace( instance->GetID(), std::move( instance ) ).first;

				result.OnRegistered();
				return result;
			}

			return static_cast< const T& >( *currentInstance->second );
		}

	private:
		std::unordered_map< TypeId, std::unique_ptr< const Type > > m_types;
	};

	static auto Get = RTTI::Get;
}
#pragma endregion

#pragma region Properties
namespace rtti
{
	class Property
	{
		friend class Type;
	public:
		virtual ~Property() = default;

		const char* GetName() const 
		{ 
			return m_name;
		}

		virtual size_t GetOffset() const
		{
			return m_offset;
		}

		virtual TypeId GetID() const
		{
			return m_id;
		}

		void* GetAddress( void* owner ) const
		{
			return static_cast< char* >( owner ) + GetOffset();
		}

		template< class T >
		T& GetValue( void* owner ) const
		{
			return *( static_cast< T* >( GetAddress( owner ) ) );
		}

		template< class T >
		void SetValue( void* owner, const T& value ) const
		{
			GetValue< T >( owner ) = value;
		}

		const Type& GetType() const
		{
			return m_type;
		}

	protected:
		Property( const char* name, size_t offset, const Type& type )
			: m_name( name )
			, m_id( internal::CalcHash( name ) )
			, m_offset( offset )
			, m_type( type )
		{}

	private:
		const char* m_name = nullptr;
		TypeId m_id = 0u;
		size_t m_offset = 0u;
		const Type& m_type;
	};
}

#define RTTI_REGISTER_PROPERTY( PropertyName ) m_properties.emplace_back( CreateProperty< decltype( CurrentlyImplementedType::##PropertyName ) >( #PropertyName, offsetof( CurrentlyImplementedType, PropertyName ) ) );

#pragma endregion

#pragma region TypeClass
namespace rtti
{
	class Type
	{
	public:
		Type() = delete;
		Type( const Type& ) = delete;
		Type( Type&& ) = delete;
		Type& operator=( const Type& ) = delete;
		Type& operator=( Type&& ) = delete;

		virtual ~Type() = default;
		virtual const char* GetName() const = 0;

		template< class T >
		bool IsA() const
		{
			return *this == GetTypeInstanceOf< T >();
		}

		virtual bool InheritsFrom( const Type& type ) const 
		{ 
			return false;
		}

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
			return GetID() == rhl.GetID();
		}

		bool operator!=( const Type& rhl ) const
		{
			return !(*this == rhl);
		}

		virtual bool IsPrimitive() const
		{
			return false;
		}

		virtual const PointerType< Type >& GetPointerType() const = 0;

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

		TypeId GetID() const
		{
			return m_id;
		}

		virtual size_t GetSize() const = 0;

		virtual size_t GetPropertiesAmount() const { return 0u; }
		virtual const Property* GetProperty( size_t index ) const { return nullptr; }

		const Property* FindProperty( size_t wantedId ) const
		{
			for ( size_t i = 0u; i < GetPropertiesAmount(); ++i )
			{
				const Property* property = GetProperty( i );
				if ( property->GetID() == wantedId )
				{
					return property;
				}
			}

			return nullptr;
		}

		const Property* FindProperty( const char* name ) const
		{
			return FindProperty( internal::CalcHash( name ) );
		}

	protected:
		Type( const char* name )
			: Type( internal::CalcHash( name ) )
		{}

		Type( TypeId id )
			: m_id( id )
		{}

		virtual void OnRegistered() {}

		static size_t GetPropertiesAmountStatic() { return 0u; }
		static const ::rtti::Property* GetPropertyStatic( size_t index ) { return nullptr; }

		template< class T >
		static std::unique_ptr< ::rtti::Property > CreateProperty( const char* name, size_t offset )
		{
			return std::unique_ptr< ::rtti::Property >( new ::rtti::Property( name, offset, GetTypeInstanceOf< std::remove_const_t< T > >() ) );
		}

	private:
		TypeId m_id = 0u;
	};
}
#pragma endregion

#pragma region TypeDefining
#define RTTI_INTERNAL_INHERITS_FROM_BODY_true( ParentClassName ) return ParentClassName##::GetTypeStatic() == type || ParentClassName##::GetTypeStatic().InheritsFrom( type );
#define RTTI_INTERNAL_INHERITS_FROM_BODY_false( ParentClassName ) return false;

#define RTTI_INTERNAL_VIRTUAL_true virtual
#define RTTI_INTERNAL_VIRTUAL_false

#define RTTI_INTERNAL_INHERITS_FROM_STATIC_BODY_true return Super::GetTypeStatic().IsA< T >() || Super::InheritsFromStatic< T >();
#define RTTI_INTERNAL_INHERITS_FROM_STATIC_BODY_false return false;

#define RTTI_INTERNAL_CONSTRUCT_BODY_true( ClassName ) return nullptr;
#define RTTI_INTERNAL_CONSTRUCT_BODY_false( ClassName ) return new ClassName##();

#define RTTI_INTERNAL_CONSTRUCT_IN_PLACE_BODY_true( ClassName, dest )
#define RTTI_INTERNAL_CONSTRUCT_IN_PLACE_BODY_false( ClassName, dest ) new (##dest##) ClassName##();

#if RTTI_REQUIRE_MOVE_CTOR
#define RTTI_INTERNAL_MOVE_IN_PLACE_true( ClassName ) virtual void MoveInPlace( void* dest, void* src  ) const override {}
#define RTTI_INTERNAL_MOVE_IN_PLACE_false( ClassName ) virtual void MoveInPlace( void* dest, void* src ) const override { new (dest) ClassName##(std::move( *static_cast< ClassName##* >( src ) ) ); }
#else
#define RTTI_INTERNAL_MOVE_IN_PLACE_true( ClassName )
#define RTTI_INTERNAL_MOVE_IN_PLACE_false( ClassName )
#endif

#define RTTI_INTERNAL_PARENT_CLASS_TYPE_true( ParentClassName ) ParentClassName::Type
#define RTTI_INTERNAL_PARENT_CLASS_TYPE_false( ParentClassName ) ::rtti::Type

#define RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, Inherits, Virtual, Abstract ) \
public: \
	using ParentClassType = RTTI_INTERNAL_PARENT_CLASS_TYPE_##Inherits##( ParentClassName ) ; \
	class Type : public ParentClassType \
	{ \
	friend class ::rtti::RTTI; \
	public: \
		virtual const char* GetName() const override; \
		virtual bool InheritsFrom( const ::rtti::Type& type ) const override \
		{ \
			RTTI_INTERNAL_INHERITS_FROM_BODY_##Inherits##( ParentClassName ) \
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
		virtual const ::rtti::PointerType< ClassName >& GetPointerType() const override \
		{ \
			return ::rtti::GetTypeInstanceOf< std::add_pointer_t< ClassName > >(); \
		} \
		virtual void ConstructInPlace( void* dest ) const override \
		{ \
			RTTI_INTERNAL_CONSTRUCT_IN_PLACE_BODY_##Abstract##( ClassName, dest )\
		} \
		RTTI_INTERNAL_MOVE_IN_PLACE_##Abstract##( ClassName ) \
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
			static const Type& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< Type >(); \
			return s_typeInstance; \
		} \
		virtual size_t GetPropertiesAmount() const override { return ParentClassType::GetPropertiesAmountStatic() + m_properties.size(); } \
		virtual const ::rtti::Property* GetProperty( size_t index ) const override \
		{ \
			if( index < ParentClassType::GetPropertiesAmountStatic() ) \
			{ \
				return ParentClassType::GetPropertyStatic( index ); \
			} \
			else \
			{ \
				return m_properties[ index - ParentClassType::GetPropertiesAmountStatic() ].get(); \
			} \
		} \
	protected: \
		RTTI_INTERNAL_VIRTUAL_##Virtual ClassName##* Construct_Internal() const \
		{ \
			RTTI_INTERNAL_CONSTRUCT_BODY_##Abstract##( ClassName ) \
		} \
		Type(); \
		Type( const char* name ) : ParentClassType ( name ) {} \
		static size_t GetPropertiesAmountStatic() { return GetInstance().GetPropertiesAmount(); } \
		static const ::rtti::Property* GetPropertyStatic( size_t index ) { return GetInstance().GetProperty( index ); } \
		virtual void OnRegistered() override; \
	private: \
		std::vector< std::unique_ptr< ::rtti::Property > > m_properties; \
	}; \
	static const Type& GetTypeStatic() \
	{ \
		return Type::GetInstance(); \
	} \
	template< class T > \
	bool IsA() const \
	{ \
		return GetType().IsA< T >(); \
	} \
	template< class T > \
	bool InheritsFrom() const \
	{ \
		return GetType().InheritsFrom< T >(); \
	} \
	template< class T > \
	bool InheritsFromOrIsA() const \
	{ \
		return IsA< T >() || InheritsFrom< T >(); \
	} \
	RTTI_INTERNAL_VIRTUAL_##Virtual const Type& GetType() const \
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
		RTTI_INTERNAL_INHERITS_FROM_STATIC_BODY_##Inherits; \
	} \
	using ClassType = Type; \
private:

#define RTTI_INTERNAL_DECLARE_TYPE( ClassName, Virtual ) \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ::rtti, false, Virtual, false)

#define RTTI_INTERNAL_DECLARE_TYPE_PARENT_DIRECT( ClassName, ParentClassName, Virtual ) \
public: \
	using Super = ParentClassName; \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, true, Virtual, false) \

#define RTTI_INTERNAL_EXPAND( x ) x

#define RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO(_1,_2,NAME,...) NAME

#define RTTI_DECLARE_POLYMORPHIC_CLASS(...) RTTI_INTERNAL_EXPAND(RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO(__VA_ARGS__, RTTI_INTERNAL_DECLARE_TYPE_PARENT_DIRECT, RTTI_INTERNAL_DECLARE_TYPE)(__VA_ARGS__, true))

#define RTTI_DECLARE_CLASS(...) RTTI_INTERNAL_EXPAND(RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO(__VA_ARGS__, RTTI_INTERNAL_DECLARE_TYPE_PARENT_DIRECT, RTTI_INTERNAL_DECLARE_TYPE)(__VA_ARGS__, false))

#define RTTI_DECLARE_STRUCT(...) RTTI_DECLARE_CLASS(__VA_ARGS__) \
public:

#define RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE( ClassName ) \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ::rtti, false, true, true)

#define RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE_PARENT_DIRECT( ClassName, ParentClassName ) \
public: \
	using Super = ParentClassName; \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, true, true, true) \

#define RTTI_DECLARE_ABSTRACT_CLASS( ... ) RTTI_INTERNAL_EXPAND( RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO( __VA_ARGS__, RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE_PARENT_DIRECT, RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE )( __VA_ARGS__ ) )
#pragma endregion

#pragma region TypeImplementing
#define RTTI_INTERNAL_CONCAT(X, Y) RTTI_INTERNAL_CONCAT2(X, Y)
#define RTTI_INTERNAL_CONCAT2(X,Y) X##Y

#define RTTI_INTERNAL_REGISTER_TYPE( TypeClass ) \
namespace RTTI_INTERNAL_CONCAT( rtti_impl, __COUNTER__ ) \
{ \
namespace \
{ \
struct Initializer { Initializer() { ::rtti::RTTI::GetMutable().GetOrRegisterType< TypeClass >(); } }; \
static Initializer s_initializer; \
} \
}

#define RTTI_IMPLEMENT_TYPE( NamespaceClassName, ... ) \
NamespaceClassName##::Type::Type() : NamespaceClassName##::Type::Type( GetName() ) \
{ \
} \
void NamespaceClassName##::Type::OnRegistered() \
{ \
	using CurrentlyImplementedType = NamespaceClassName##; \
	__VA_ARGS__ \
} \
const char* NamespaceClassName##::Type::GetName() const \
{ \
	return #NamespaceClassName; \
} \
RTTI_INTERNAL_REGISTER_TYPE( NamespaceClassName##::Type )
#pragma endregion

#pragma region PointerType
#define RTTI_INTERNAL_POINTER_TYPE_COMMON_BODY(BaseClass) \
	friend class ::rtti::RTTI; \
	static constexpr const char* c_namePostfix = "*"; \
	public: \
		static TypeId CalcId() \
		{ \
			TypeId id = internal::CalcHash( GetInternalTypeStatic().GetName() ); \
			return internal::CalcHash( c_namePostfix, id ); \
		} \
		static const Type& GetInternalTypeStatic() \
		{ \
			return GetTypeInstanceOf< T >(); \
		} \
		virtual const Type& GetInternalType() const override \
		{ \
			return GetInternalTypeStatic(); \
		} \
		static const PointerType& GetInstance() \
		{ \
			static const PointerType& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< PointerType >(); \
			return s_typeInstance; \
		} \
		virtual const PointerType< T >& GetPointerType() const override \
		{ \
			return GetTypeInstanceOf< T* >(); \
		} \
	protected: \
		PointerType() \
			: BaseClass( CalcId(), std::string( GetInternalTypeStatic().GetName() ) + c_namePostfix ) \
		{} \
		PointerType( TypeId typeId, std::string&& name ) \
			: BaseClass( typeId, std::move( name ) ) \
		{} \

namespace rtti
{
	template< class T = Type, class T2 >
	class PointerType : public PointerType< Type >
	{
		RTTI_INTERNAL_POINTER_TYPE_COMMON_BODY( PointerType< Type > );
	};

	template<>
	class PointerType< Type > : public Type
	{
	public:
		virtual const Type& GetInternalType() const = 0;

		virtual void ConstructInPlace( void* dest ) const override
		{
			*static_cast< void** >( dest ) = nullptr;
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const override
		{
			std::memcpy( dest, src, GetSize() );
		}
#endif

		void Destroy( void* ptr ) const override {}
		size_t GetSize() const override { return sizeof( void* ); }

		virtual const char* GetName() const override { return m_strName.c_str(); }

	protected:
		PointerType( TypeId typeId, std::string&& name )
			: Type( typeId )
			, m_strName( std::move( name ) )
		{}
		std::string m_strName;
	};

	template< class T >
	struct get_void { using type = void; };

	template< class T >
	class PointerType< T, typename get_void< typename T::Super >::type > : public PointerType< typename T::Super >
	{
		RTTI_INTERNAL_POINTER_TYPE_COMMON_BODY( PointerType< typename T::Super > );
	};
}
#pragma endregion

#pragma region ContainerType
namespace rtti
{
	class ContainerType : public Type
	{
	public:
		using Type::Type;
		virtual const Type& GetInternalType() const = 0;
	};
}
#pragma endregion

#pragma region ArrayType
namespace rtti
{
	template< class T, size_t Count >
	class ArrayType : public ContainerType
	{
		friend class ::rtti::RTTI;

	public:
		static TypeId CalcId()
		{
			TypeId id = internal::CalcHash( GetInternalTypeStatic().GetName() );
			id = internal::CalcHash( "[", id );
			id = internal::CalcHash( std::to_string( Count ).c_str(), id );
			return internal::CalcHash( "]", id );
		}

		static const ArrayType& GetInstance()
		{
			return ::rtti::RTTI::GetMutable().GetOrRegisterType< ArrayType >();
		}

		static const Type& GetInternalTypeStatic()
		{
			return GetTypeInstanceOf< T >();
		}

		virtual const PointerType< T[ Count ] >& GetPointerType() const override
		{
			return GetTypeInstanceOf< T (*)[ Count ] >();
		}

		const Type& GetInternalType() const override
		{
			return GetInternalTypeStatic();
		}

		const char* GetName() const override
		{
			return m_name.c_str();
		}

		void ConstructInPlace( void* dest ) const override
		{
			T* arr = static_cast< T* >( dest );

			for ( size_t i = 0u; i < Count; ++i )
			{
				GetInternalType().ConstructInPlace( &arr[ i ] );
			}
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const override
		{
			T* destArr = new ( dest ) T[ Count ];
			T* sourceArr = static_cast< T* >( src );

			for ( size_t i = 0u; i < Count; ++i )
			{
				GetInternalType().MoveInPlace( &destArr[ i ], &sourceArr[ i ] );
			}
		}
#endif

		void Destroy( void* ptr ) const override
		{
			T* arr = static_cast< T* >( ptr );

			for ( size_t i = 0u; i < Count; ++i )
			{
				GetInternalType().Destroy( &arr[ i ] );
			}
		}

		size_t GetSize() const override
		{
			return sizeof( T ) * Count;
		}

	private:
		ArrayType()
			: ContainerType( CalcId() )
		{
			m_name.reserve( strlen( GetInternalType().GetName() ) + 10 );
			m_name += GetInternalType().GetName();
			m_name += "[";
			m_name += std::to_string( Count );
			m_name += "]";
		}

		std::string m_name;
	};
}

#pragma endregion

#pragma region VectorType
namespace rtti
{
	template< class T >
	class VectorType : public ContainerType
	{
		friend class ::rtti::RTTI;

		static constexpr const char* c_namePrefix = "Vector< ";
		static constexpr const char* c_namePostfix = " >";

	public:
		static TypeId CalcId()
		{
			TypeId id = internal::CalcHash( c_namePrefix );
			id = internal::CalcHash( GetInternalTypeStatic().GetName(), id );
			return internal::CalcHash( c_namePostfix, id );
		}

		static const VectorType< T >& GetInstance()
		{
			return ::rtti::RTTI::GetMutable().GetOrRegisterType< VectorType >();
		}

		static const Type& GetInternalTypeStatic()
		{
			return GetTypeInstanceOf< T >();
		}

		virtual const PointerType< std::vector< T > >& GetPointerType() const override
		{
			return GetTypeInstanceOf< std::vector< T >* >();
		}

		virtual const Type& GetInternalType() const override
		{
			return GetInternalTypeStatic();
		}

		virtual const char* GetName() const override
		{
			return m_name.c_str();
		}

		virtual void ConstructInPlace( void* dest ) const override
		{
			*static_cast< std::vector< T >* >( dest ) = std::vector< T >();
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const override
		{
			new ( dest ) std::vector< T >( std::move( *static_cast< std::vector< T >* >( src ) ) );
		}
#endif

		virtual void Destroy( void* ptr ) const override
		{
			static_cast< std::vector< T >* >( ptr )->~vector< T >();
		}

		virtual size_t GetSize() const override
		{
			return sizeof( std::vector< T > );
		}

	private:
		VectorType()
			: ContainerType( CalcId() )
		{
			m_name.reserve( ( sizeof( c_namePrefix ) / sizeof( char ) ) + strlen( GetInternalType().GetName() ) + ( sizeof(c_namePostfix) / sizeof( char ) ) );
			m_name += c_namePrefix;
			m_name += GetInternalType().GetName();
			m_name += c_namePrefix;
		}

		std::string m_name;
	};
}
#pragma endregion

#pragma region Primitives
namespace rtti
{
	template< class T >
	class PrimitiveType : public Type
	{
		static_assert( std::is_arithmetic_v< T >, "That's not a primitive type!" );
		friend class ::rtti::RTTI;

	public:
		virtual const char* GetName() const;
		virtual void ConstructInPlace( void* dest ) const
		{
			*static_cast< T* >( dest ) = T();
		}

		virtual const PointerType< Type >& GetPointerType() const override
		{
			return GetTypeInstanceOf< T* >();
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const
		{
			std::memcpy( dest, src, sizeof( T ) );
		}
#endif
		virtual void Destroy( void* ptr ) const override {}

		virtual size_t GetSize() const override { return sizeof( T ); }

		virtual bool IsPrimitive() const override { return true; }

		static const rtti::PrimitiveType< T >& GetInstance()
		{
			static const rtti::PrimitiveType< T >& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< rtti::PrimitiveType< T > >();
			return s_typeInstance;
		}

	private:
		PrimitiveType< T >() : rtti::Type( GetName() ) {}
	};
}

#define RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( type ) \
template<> \
inline const char* ::rtti::PrimitiveType< type >::GetName() const \
{ \
	return #type; \
} \
template<> \
inline bool rtti::Type::IsA< type >() const \
{ \
	return *this == ::rtti::PrimitiveType< type >::GetInstance(); \
} \
RTTI_INTERNAL_REGISTER_TYPE( ::rtti::PrimitiveType<##type##> )

RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( float )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int32 )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int64 )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( bool )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned short )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned long long )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( double )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int8 )
#pragma endregion