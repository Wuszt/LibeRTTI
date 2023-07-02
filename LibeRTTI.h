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

#ifndef RTTI_CFG_CREATE_STD_VECTOR_TYPE
#define RTTI_CFG_CREATE_STD_VECTOR_TYPE 1
#endif

#ifndef RTTI_CFG_CREATE_STD_SET_TYPE
#define RTTI_CFG_CREATE_STD_SET_TYPE 1
#endif

#ifndef RTTI_CFG_CREATE_STD_PAIR_TYPE
#define RTTI_CFG_CREATE_STD_PAIR_TYPE 1
#endif

#ifndef RTTI_CFG_CREATE_STD_MAP_TYPE
#define RTTI_CFG_CREATE_STD_MAP_TYPE 1
#endif

#ifndef RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
#define RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE 1
#endif

#ifndef RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
#define RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE 1
#endif

#pragma region Includes
#include <vector>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <string>
#include <array>

#if RTTI_CFG_CREATE_STD_PAIR_TYPE || RTTI_CFG_CREATE_STD_MAP_TYPE
#include <utility>
#endif

#if RTTI_CFG_CREATE_STD_SET_TYPE
#include <unordered_set>
#endif

#if RTTI_CFG_CREATE_STD_MAP_TYPE
#include <unordered_map>
#endif
#pragma endregion

#pragma region ForwardDeclarations
namespace rtti
{
	class Type;
	template< class T, class T2 = void > class PointerType;
	template< class T > class VectorType;
	template< class T > class SetType;
	template< class T1, class T2 > class MapType;
	template< class T, size_t Count > class ArrayType;
	template< class T > class PrimitiveType;
	template< class T > class SharedPtrType;
	template< class T > class UniquePtrType;
	using TypeId = size_t;

	namespace internal
	{
		template< class T1, class T2 > class PairType;
	}
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

#if RTTI_CFG_CREATE_STD_SET_TYPE
		template< class T >
		struct is_vector : std::false_type {};

		template< class T >
		struct is_vector< std::vector< T > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_SET_TYPE
		template< class T >
		struct is_set : std::false_type {};

		template< class T >
		struct is_set< std::unordered_set< T > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_PAIR_TYPE
		template< class T >
		struct is_pair : std::false_type {};

		template< class T1, class T2 >
		struct is_pair< std::pair< T1, T2 > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_MAP_TYPE
		template< class T >
		struct is_map : std::false_type {};

		template< class T1, class T2 >
		struct is_map< std::unordered_map< T1, T2 > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
		template< class T >
		struct is_sharedPtr : std::false_type {};

		template< class T >
		struct is_sharedPtr< std::shared_ptr< T > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
		template< class T >
		struct is_uniquePtr : std::false_type {};

		template< class T >
		struct is_uniquePtr< std::unique_ptr< T > > : std::true_type {};
#endif

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
	struct type_of< T, std::enable_if_t< std::is_same_v< T, void > > >{ using type = Type; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_class_v< T > && !internal::is_template< T >::value && !internal::is_array< T >::value > > { using type = typename T::Type; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_fundamental_v< T > && !std::is_same_v< T, void >  > > { using type = PrimitiveType< T >; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_pointer_v< T > > > { using type = PointerType< std::remove_const_t< std::remove_pointer_t< T > > >; };

#if RTTI_CFG_CREATE_STD_VECTOR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_vector< T >::value > > { using type = VectorType< std::remove_const_t< typename T::value_type > >; };
#endif

#if RTTI_CFG_CREATE_STD_SET_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_set< T >::value > > { using type = SetType< std::remove_const_t< typename T::value_type > >; };
#endif

#if RTTI_CFG_CREATE_STD_PAIR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_pair< T >::value > > { using type = internal::PairType< std::remove_const_t< typename T::first_type >, std::remove_const_t< typename T::second_type > >; };
#endif

#if RTTI_CFG_CREATE_STD_MAP_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_map< T >::value > > { using type = MapType< std::remove_const_t< typename T::key_type >, std::remove_const_t< typename T::mapped_type > >; };
#endif

#if RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_sharedPtr< T >::value > > { using type = SharedPtrType< std::remove_const_t< typename T::element_type > >; };
#endif

#if RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_uniquePtr< T >::value > > { using type = UniquePtrType< std::remove_const_t< typename T::element_type > >; };
#endif

	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_array< T >::value > >{ private: using internalType = std::remove_reference_t< decltype( *std::begin( std::declval< T& >() ) ) >; public: using type = ArrayType< std::remove_const_t< internalType >, sizeof( T ) / sizeof( internalType ) >; };

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

		template< class T, class... TArgs >
		const T& GetOrRegisterType( const TArgs& ... args ) const
		{
			return const_cast< RTTI& >( this ).GetOrRegisterType< T >( std::forward< const TArgs& >( args )... );
		}

		template< class T, class... TArgs, std::enable_if_t< internal::has_CalcId< T >::value, bool > = true >
		T& GetOrRegisterType( const TArgs& ... args )
		{
			TypeId id = T::CalcId( args... );
			auto found = m_types.find( id );

			if ( found != m_types.end() )
			{
				return static_cast< T& >( *found->second );
			}

			T* instance = new T( std::forward< const TArgs& >( args )... );
			m_types.emplace( id, instance );

			instance->OnRegistered();

			return *instance;
		}

		template< class T, class... TArgs, std::enable_if_t< !internal::has_CalcId< T >::value, bool > = true >
		T& GetOrRegisterType( const TArgs& ... args )
		{
			std::unique_ptr< T > instance( new T( std::forward< const TArgs& >( args )... ) );

			auto currentInstance = m_types.find( instance->GetID() );

			if ( currentInstance == m_types.end() )
			{
				T& result = *instance;
				currentInstance = m_types.emplace( instance->GetID(), std::move( instance ) ).first;

				result.OnRegistered();
				return result;
			}

			return static_cast< T& >( *currentInstance->second );
		}

		const Type* FindType( TypeId id ) const
		{
			auto found = m_types.find( id );
			if ( found != m_types.end() )
			{
				return found->second.get();
			}

			return nullptr;
		}

		const Type* FindType( const char* name ) const
		{
			TypeId id = internal::CalcHash( name );
			return FindType( id );
		}

	private:
		std::unordered_map< TypeId, std::unique_ptr< Type > > m_types;
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
		enum class Kind
		{
			Class,
			Struct,
			Primitive,
			Pointer,
			Container,
			RuntimeType,
		};

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

		virtual Kind GetKind() const = 0;

		virtual void ConstructInPlace( void* dest ) const = 0
		{}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const = 0
		{}
#endif
		virtual void Destroy( void* ptr ) const = 0
		{}

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

		virtual size_t GetSize() const = 0
		{ return 0u; }

		virtual size_t GetAlignment() const = 0
		{ return 0u; }

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

		static std::unique_ptr< ::rtti::Property > CreateProperty( const char* name, size_t offset, const Type& type )
		{
			return std::unique_ptr< ::rtti::Property >( new ::rtti::Property( name, offset, type ) );
		}

		template< class T >
		static std::unique_ptr< ::rtti::Property > CreateProperty( const char* name, size_t offset )
		{
			return CreateProperty( name, offset, GetTypeInstanceOf< std::remove_const_t< T > >() );
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

#define RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, Inherits, Virtual, Abstract, KindName ) \
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
		virtual Kind GetKind() const override \
		{ \
			return KindName ; \
		} \
		std::unique_ptr< ClassName > Construct() const \
		{ \
			return std::unique_ptr< ClassName >( Construct_Internal() ); \
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
		virtual size_t GetAlignment() const override \
		{ \
			return alignof( ClassName ); \
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

#define RTTI_INTERNAL_DECLARE_TYPE( ClassName, Virtual, Kind ) \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ::rtti, false, Virtual, false, Kind )

#define RTTI_INTERNAL_DECLARE_TYPE_PARENT_DIRECT( ClassName, ParentClassName, Virtual, Kind ) \
public: \
	using Super = ParentClassName; \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, true, Virtual, false, Kind ) \

#define RTTI_INTERNAL_EXPAND( x ) x

#define RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO(_1,_2,NAME,...) NAME

#define RTTI_DECLARE_POLYMORPHIC_CLASS(...) RTTI_INTERNAL_EXPAND(RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO(__VA_ARGS__, RTTI_INTERNAL_DECLARE_TYPE_PARENT_DIRECT, RTTI_INTERNAL_DECLARE_TYPE)(__VA_ARGS__, true, Kind::Class))

#define RTTI_DECLARE_TYPE_INTERNAL( Kind, ... ) RTTI_INTERNAL_EXPAND(RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO(__VA_ARGS__, RTTI_INTERNAL_DECLARE_TYPE_PARENT_DIRECT, RTTI_INTERNAL_DECLARE_TYPE)( __VA_ARGS__, false, Kind ))

#define RTTI_DECLARE_CLASS(...) RTTI_DECLARE_TYPE_INTERNAL( Kind::Class, __VA_ARGS__ )

#define RTTI_DECLARE_STRUCT(...) RTTI_DECLARE_TYPE_INTERNAL( Kind::Struct, __VA_ARGS__) \
public:

#define RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE( ClassName, Kind ) \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ::rtti, false, true, true, Kind )

#define RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE_PARENT_DIRECT( ClassName, ParentClassName, Kind ) \
public: \
	using Super = ParentClassName; \
RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, true, true, true, Kind ) \

#define RTTI_DECLARE_ABSTRACT_CLASS( ... ) RTTI_INTERNAL_EXPAND( RTTI_INTERNAL_GET_DECLARE_TYPE_MACRO( __VA_ARGS__, RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE_PARENT_DIRECT, RTTI_INTERNAL_DECLARE_ABSTRACT_TYPE )( __VA_ARGS__, Kind::Class ) )
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
namespace rtti
{
	namespace internal
	{
		template< class DerivedClass, class T, class ParentClass >
		class PointerTypeImplementation : public ParentClass
		{
			friend class ::rtti::RTTI;
			static constexpr const char* c_namePostfix = "*";
		public:
			static TypeId CalcId()
			{
				TypeId id = internal::CalcHash( GetInternalTypeStatic().GetName() );
				return internal::CalcHash( c_namePostfix, id );
			}

			static const typename type_of< T >::type& GetInternalTypeStatic()
			{
				return GetTypeInstanceOf< T >();
			}

			virtual const typename type_of< T >::type& GetInternalType() const override
			{
				return GetInternalTypeStatic();
			}

			static const DerivedClass& GetInstance()
			{
				static const DerivedClass& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< DerivedClass >();
				return s_typeInstance;
			}

			virtual ::rtti::Type::Kind GetKind() const override
			{
				return ::rtti::Type::Kind::Pointer;
			}

		protected:
			PointerTypeImplementation()
				: ParentClass( CalcId(), std::string( GetInternalTypeStatic().GetName() ) + c_namePostfix )
			{}

			PointerTypeImplementation( TypeId typeId, std::string&& name )
				: ParentClass( typeId, std::move( name ) )
			{}
		};
	}
}

namespace rtti
{
	template< class T = void, class T2 >
	class PointerType : public internal::PointerTypeImplementation< PointerType< T >, T, PointerType< void > >
	{
		using internal::PointerTypeImplementation< PointerType< T >, T, PointerType< void > >::PointerTypeImplementation;
	};

	template<>
	class PointerType< void > : public Type
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

		virtual void Destroy( void* ptr ) const override {}
		virtual size_t GetSize() const override { return sizeof( void* ); }
		virtual size_t GetAlignment() const override { return alignof( void* ); }

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
	class PointerType< T, typename get_void< typename T::Super >::type > : public internal::PointerTypeImplementation< PointerType< T >, T, PointerType< typename T::Super > >
	{
		using internal::PointerTypeImplementation< PointerType< T >, T, PointerType< typename T::Super > >::PointerTypeImplementation;
	};
}
#pragma endregion

#pragma region InternalTemplateType
namespace rtti
{
	namespace internal
	{
		template< class DerivedType, class TrueType, class ParentType >
		class TemplateType : public ParentType
		{
			friend class ::rtti::RTTI;

		public:
			static TypeId CalcId()
			{
				TypeId id = internal::CalcHash( DerivedType::GetBaseName() );
				id = internal::CalcHash( "< ", id );
				const auto internalTypes = DerivedType::GetInternalTypesStatic();
				id = internal::CalcHash( internalTypes[0u]->GetName(), id);

				for ( size_t i = 1u; i < internalTypes.size(); ++i )
				{
					id = internal::CalcHash( ", ", id);
					id = internal::CalcHash( internalTypes[ i ]->GetName(), id);
				}

				return internal::CalcHash( " >", id );
			}

			static const DerivedType& GetInstance()
			{
				static const DerivedType& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< DerivedType >();
				return s_typeInstance;
			}

			virtual const char* GetName() const override
			{
				return m_name.c_str();
			}

			virtual void ConstructInPlace( void* dest ) const override
			{
				*static_cast< TrueType* >( dest ) = TrueType();
			}

#if RTTI_REQUIRE_MOVE_CTOR
			virtual void MoveInPlace( void* dest, void* src ) const override
			{
				new ( dest ) TrueType( std::move( *static_cast< TrueType* >( src ) ) );
			}
#endif

			virtual size_t GetSize() const override
			{
				return sizeof( TrueType );
			}

			virtual size_t GetAlignment() const override
			{
				return alignof( TrueType );
			}

		protected:
			TemplateType()
				: ParentType( CalcId() )
			{
				size_t internalTypesNamesLength = 0u;
				const auto internalTypes = DerivedType::GetInternalTypesStatic();
				for ( const auto* type : internalTypes )
				{
					internalTypesNamesLength += strlen(type->GetName());
				}

				m_name.reserve( strlen( DerivedType::GetBaseName() ) + internalTypesNamesLength + ( internalTypes.size() - 1 ) * 2u + 4u);
				m_name += DerivedType::GetBaseName();
				m_name += "< ";
				m_name += internalTypes[ 0u ]->GetName();

				for ( size_t i = 1u; i < internalTypes.size(); ++i )
				{
					m_name += ", ";
					m_name += internalTypes[ i ]->GetName();
				}

				m_name += " >";
			}

		private:
			std::string m_name;
		};
	}
}
#pragma endregion

#pragma region PairType
#if RTTI_CFG_CREATE_STD_PAIR_TYPE || RTTI_CFG_CREATE_STD_MAP_TYPE
namespace rtti
{
	namespace internal
	{
		template< class T1, class T2 >
		class PairType : public internal::TemplateType< PairType< T1, T2 >, std::pair< T1, T2 >, Type >
		{
			friend class ::rtti::RTTI;
			friend class internal::TemplateType< PairType< T1, T2 >, std::pair< T1, T2 >, Type >;
			friend class MapType< T1, T2 >;

		public:
			static constexpr const char* GetBaseName() { return "Pair"; }

			static const typename type_of< T1 >::type& GetFirstInternalTypeStatic()
			{
				return GetTypeInstanceOf< T1 >();
			}

			static const typename type_of< T2 >::type& GetSecondInternalTypeStatic()
			{
				return GetTypeInstanceOf< T2 >();
			}

			const typename type_of< T1 >::type& GetFirstInternalType() const
			{
				return GetFirstInternalTypeStatic();
			}

			const typename type_of< T2 >::type& GetSecondInternalType() const
			{
				return GetSecondInternalTypeStatic();
			}

			virtual void Destroy( void* ptr ) const override
			{
				static_cast< std::pair< T1, T2 >* >( ptr )->~pair< T1, T2 >();
			}

			virtual ::rtti::Type::Kind GetKind() const override
			{
				return ::rtti::Type::Kind::Class;
			}

		private:
			static std::array<const Type*, 2> GetInternalTypesStatic()
			{
				return { &GetFirstInternalTypeStatic(), &GetSecondInternalTypeStatic() };
			}
		};
	}

	#if RTTI_CFG_CREATE_STD_PAIR_TYPE
	template< class T1, class T2 >
	using PairType = internal::PairType< T1, T2 >;
	#endif
}
#endif
#pragma endregion

#pragma region ContainerType
namespace rtti
{
	class ContainerType : public Type
	{
	public:
		virtual const Type& GetInternalType() const = 0;
		virtual Kind GetKind() const override { return Kind::Container; }
	protected:
		using Type::Type;
	};

	namespace internal
	{
		template< class InternalType, class DerivedType, class TrueType >
		class DynamicContainerTemplateType : public TemplateType< DerivedType, TrueType, ContainerType >
		{
			friend class TemplateType< DerivedType, TrueType, ContainerType >;

		public:
			static const InternalType& GetInternalTypeStatic()
			{
				return InternalType::GetInstance();
			}

			virtual const InternalType& GetInternalType() const override
			{
				return InternalType::GetInstance();
			}

		private:
			static std::array<const Type*, 1> GetInternalTypesStatic()
			{
				return { &GetInternalTypeStatic() };
			}
		};
	}
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

		size_t GetAlignment() const override
		{
			return alignof( T );
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
#if RTTI_CFG_CREATE_STD_VECTOR_TYPE
namespace rtti
{
	template< class T >
	class VectorType : public internal::DynamicContainerTemplateType< typename type_of< T >::type, VectorType< T >, std::vector< T > >
	{
	public:
		static constexpr const char* GetBaseName() { return "Vector"; }

		virtual void Destroy( void* ptr ) const override
		{
			static_cast< std::vector< T >* >( ptr )->~vector< T >();
		}
	};
}
#endif
#pragma endregion

#pragma region SetType
#if RTTI_CFG_CREATE_STD_SET_TYPE
namespace rtti
{
	template< class T >
	class SetType : public internal::DynamicContainerTemplateType< typename type_of< T >::type, SetType< T >, std::unordered_set< T > >
	{
	public:
		static constexpr const char* GetBaseName() { return "Set"; }

		virtual void Destroy( void* ptr ) const override
		{
			static_cast< std::unordered_set< T >* >( ptr )->~unordered_set< T >();
		}
	};
}
#endif
#pragma endregion

#pragma region MapType
namespace rtti
{
	template< class TKey, class TValue >
	class MapType : public internal::DynamicContainerTemplateType< internal::PairType< TKey, TValue >, MapType< TKey, TValue >, std::unordered_map< TKey, TValue > >
	{
		friend class internal::TemplateType< MapType, std::unordered_map< TKey, TValue >, ContainerType >;

	public:
		static constexpr const char* GetBaseName() { return "Map"; }

		virtual void Destroy( void* ptr ) const override
		{
			static_cast< std::unordered_map< TKey, TValue >* >( ptr )->~unordered_map< TKey, TValue >();
		}

	private:
		static std::array<const Type*, 2> GetInternalTypesStatic()
		{
			return internal::PairType< TKey, TValue >::GetInternalTypesStatic();
		}
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
			new ( dest ) T();
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const
		{
			std::memcpy( dest, src, sizeof( T ) );
		}
#endif
		virtual void Destroy( void* ptr ) const override {}

		virtual size_t GetSize() const override { return sizeof( T ); }

		virtual size_t GetAlignment() const override { return alignof( T ); }

		virtual Kind GetKind() const override { return Kind::Primitive; }

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

#pragma region RuntimeType
namespace rtti
{
	template< class Parent = void >
	class RuntimeType : public type_of< Parent >::type
	{
		friend class ::rtti::RTTI;
		using ParentType = typename type_of< Parent >::type;
	public:
		static RuntimeType& Create( std::string name )
		{
			if ( ::rtti::Get().FindType( name.c_str() ) )
			{
				throw; // Type with this name already exists
			}

			return ::rtti::RTTI::GetMutable().GetOrRegisterType< RuntimeType >( std::move( name ) );
		}

		const std::unique_ptr< ::rtti::Property >& AddProperty( const char* name, const Type& type )
		{
			size_t currentOffset = ParentType::GetSize();
			
			if ( GetPropertiesAmount() > 0u )
			{
				const auto& lastProperty = GetProperty( GetPropertiesAmount() - 1u );
				currentOffset = lastProperty->GetOffset() + lastProperty->GetType().GetSize();
				currentOffset = ( ( currentOffset + ( type.GetAlignment() - 1u ) ) & ~( type.GetAlignment() - 1u ) );
			}

			m_properties.emplace_back( ::rtti::Type::CreateProperty( name, currentOffset, type ) );
			m_size = currentOffset - ParentType::GetSize() + type.GetSize();
			m_alignment = std::max( m_alignment, type.GetAlignment() );
			return m_properties.back();
		}

		template< class T >
		const std::unique_ptr< ::rtti::Property >& AddProperty( const char* name )
		{
			return AddProperty( name, GetTypeInstanceOf< T >() );
		}

		virtual const char* GetName() const override
		{
			return m_name.c_str();
		}

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::RuntimeType;
		}

		virtual void ConstructInPlace( void* dest ) const override
		{
			ParentType::ConstructInPlace( dest );
			for ( const auto& property : m_properties )
			{
				const auto& type = property->GetType();
				type.ConstructInPlace( static_cast< uint8_t* >( dest ) + property->GetOffset() );
			}
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const override
		{
			ParentType::MoveInPlace( dest, src );
			for ( const auto& property : m_properties )
			{
				const auto& type = property->GetType();
				type.MoveInPlace( static_cast< uint8_t* >( dest ) + property->GetOffset(), static_cast< uint8_t* >( src ) + property->GetOffset() );
			}
		}
#endif

		virtual void Destroy( void* ptr ) const override
		{
			ParentType::Destroy( ptr );
			for ( const auto& property : m_properties )
			{
				const auto& type = property->GetType();
				type.Destroy( static_cast< uint8_t* >( ptr ) + property->GetOffset() );
			}
		}

		virtual size_t GetSize() const override
		{
			return m_size;
		}

		virtual size_t GetAlignment() const override
		{
			return m_alignment;
		}

		virtual size_t GetPropertiesAmount() const override { return ParentType::GetPropertiesAmountStatic() + m_properties.size(); }
		virtual const ::rtti::Property* GetProperty( size_t index ) const override
		{
			if ( index < ParentType::GetPropertiesAmountStatic() )
			{
				return ParentType::GetPropertyStatic( index );
			}
			else
			{
				return m_properties[ index - ParentType::GetPropertiesAmountStatic() ].get();
			}
		}

	private:
		RuntimeType( std::string name )
			: ParentType( name.c_str() )
			, m_name( std::move( name ) )
		{}

		std::string m_name;
		std::vector< std::unique_ptr< ::rtti::Property > > m_properties;
		size_t m_size = ParentType::GetSize();
		size_t m_alignment = ParentType::GetAlignment();
	};
}
#pragma endregion

#pragma region SharedPtr
#if RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
namespace rtti
{
	template< class T >
	class SharedPtrType : public internal::TemplateType< SharedPtrType< T >, std::shared_ptr< T >, rtti::Type >
	{
		friend class ::rtti::RTTI;
		friend class internal::TemplateType< SharedPtrType, std::shared_ptr< T >, rtti::Type >;

	public:
		static constexpr const char* GetBaseName() { return "SharedPtr"; }

		static const typename type_of< T >::type& GetInternalTypeStatic()
		{
			return GetTypeInstanceOf< T >();
		}

		const typename type_of< T >::type& GetInternalType() const
		{
			return GetInternalTypeStatic();
		}

		virtual void Destroy( void* ptr ) const override
		{
			static_cast< std::shared_ptr< T >* >( ptr )->~shared_ptr< T >();
		}

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::Pointer;
		}

	private:
		static std::array<const Type*, 1> GetInternalTypesStatic()
		{
			return { &GetInternalTypeStatic() };
		}
	};
}
#endif
#pragma endregion

#pragma region UniquePtr
#if RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
namespace rtti
{
	template< class T >
	class UniquePtrType : public internal::TemplateType< UniquePtrType< T >, std::unique_ptr< T >, rtti::Type >
	{
		friend class ::rtti::RTTI;
		friend class internal::TemplateType< UniquePtrType< T >, std::unique_ptr< T >, rtti::Type >;

	public:
		static constexpr const char* GetBaseName() { return "UniquePtr"; }

		static const typename type_of< T >::type& GetInternalTypeStatic()
		{
			return GetTypeInstanceOf< T >();
		}

		const typename type_of< T >::type& GetInternalType() const
		{
			return GetInternalTypeStatic();
		}

		virtual void Destroy( void* ptr ) const override
		{
			static_cast< std::unique_ptr< T >* >( ptr )->~unique_ptr< T >();
		}

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::Pointer;
		}

	private:
		static std::array<const Type*, 1> GetInternalTypesStatic()
		{
			return { &GetInternalTypeStatic() };
		}
	};
}
#endif
#pragma endregion
