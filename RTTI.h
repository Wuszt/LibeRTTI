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
#include <unordered_map>
#include <memory>
#include <type_traits>

#ifndef RTTI_REQUIRE_MOVE_CTOR
#define RTTI_REQUIRE_MOVE_CTOR 1
#endif

#ifndef RTTI_ALLOW_MULTI_POINTERS
#define RTTI_ALLOW_MULTI_POINTERS 1
#endif

namespace rtti
{
	class Type;
	class NativeType;
	class DynamicType;

	using TypeId = size_t;

	class RTTI
	{
	public:
		void GetNativeTypes( std::vector< const NativeType* >& outTypes ) const
		{
			outTypes.reserve( m_nativeTypes.size() );
			for ( const auto& [id, type] : m_nativeTypes )
			{
				outTypes.emplace_back(type);
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

		template< class T >
		void RegisterNativeType();

		template< class T, class... TArgs >
		const T& GetOrRegisterDynamicType( const TArgs& ... args );

	private:

		std::unordered_map< TypeId, const NativeType* > m_nativeTypes;
		std::unordered_map< TypeId, std::unique_ptr< const DynamicType > > m_dynamicTypes;
	};

	static auto Get = RTTI::Get;

	namespace rtti_internal
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
	}

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
			, m_id( rtti_internal::CalcHash( name ) )
			, m_offset( offset )
			, m_type( type )
		{}

	private:
		const char* m_name = nullptr;
		TypeId m_id = 0u;
		size_t m_offset = 0u;
		const Type& m_type;
	};

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
			return *this == T::GetTypeStatic();
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
			return FindProperty( rtti_internal::CalcHash( name ) );
		}

	protected:
		Type( TypeId id )
			: m_id( id )
		{}

		static size_t GetPropertiesAmountStatic() { return 0u; }
		static const ::rtti::Property* GetPropertyStatic( size_t index ) { return nullptr; }

		template< class T >
		static std::unique_ptr< ::rtti::Property > CreateProperty( const char* name, size_t offset )
		{
			return std::unique_ptr< ::rtti::Property >( new ::rtti::Property( name, offset, GetTypeInstanceOf< T >() ) );
		}

	private:
		TypeId m_id = 0u;
	};

	class NativeType : public Type
	{
	protected:
		NativeType( const char* name )
			: Type( rtti_internal::CalcHash( name ) )
		{}

		virtual void OnRegistered() {}
	};

	template< class T >
	inline void RTTI::RegisterNativeType()
	{
		T& instance = T::GetMutableInstance();
		if ( m_nativeTypes.contains( instance.GetID() ) )
		{
			if ( m_nativeTypes[ instance.GetID() ] == &instance )
			{
				return;
			}

			// Your types have to be unique! You probably declared 2 classes with the same name in 2 different TUs.
			throw;
		}

		instance.OnRegistered();

		m_nativeTypes.emplace( instance.GetID(), &instance );
	}

	class DynamicType : public Type
	{
		friend class ::rtti::RTTI;
	protected:
		DynamicType( TypeId id )
			: Type( id )
		{}
	};

	class PointerType : public DynamicType
	{
	private:
		static constexpr const char* c_namePostfix = "*";

	public:
		static TypeId CalcId( const Type& internalType )
		{
			TypeId id = rtti_internal::CalcHash( internalType.GetName() );
			return rtti_internal::CalcHash( c_namePostfix, id );
		}

		virtual const char* GetName() const override { return m_strName.c_str(); }

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
		size_t GetSize() const override	{ return sizeof( void* ); }

		const Type& GetInternalType() const
		{
			return m_internalType;
		}

	protected:
		PointerType( const Type& internalType )
			: DynamicType( CalcId( internalType ) )
			, m_internalType( internalType )
			, m_strName( std::string( internalType.GetName() ) + c_namePostfix )
		{}

		std::string m_strName;
		const Type& m_internalType;
	};

	class ContainerType : public DynamicType
	{
	public:
		using DynamicType::DynamicType;
		virtual const Type& GetInternalType() const = 0;
	};

	template< class T >
	class VectorType : public ContainerType
	{
		friend class ::rtti::RTTI;

		static constexpr const char* c_namePrefix = "Vector< ";
		static constexpr const char* c_namePostfix = " >";

	public:
		static TypeId CalcId()
		{
			TypeId id = rtti_internal::CalcHash( c_namePrefix );
			id = rtti_internal::CalcHash( GetInternalTypeStatic().GetName(), id );
			return rtti_internal::CalcHash( c_namePostfix, id );
		}

		static const VectorType< T >& GetInstance()
		{
			return ::rtti::RTTI::GetMutable().GetOrRegisterDynamicType< VectorType< T > >();
		}

		static const Type& GetInternalTypeStatic()
		{
			return GetTypeInstanceOf< T >();
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
			, m_name( std::string( c_namePrefix ) + GetInternalType().GetName() + c_namePostfix )
		{}

		std::string m_name;
	};

	template< class T, class... TArgs >
	const T& RTTI::GetOrRegisterDynamicType( const TArgs& ... args )
	{
		TypeId id = T::CalcId( args... );
		auto found = m_dynamicTypes.find( id );

		if ( found != m_dynamicTypes.end() )
		{
			return static_cast< const T& >( *found->second );
		}

		return static_cast< const T& >( *m_dynamicTypes.emplace( id, new T( args... ) ).first->second );
	}

	template< class T >
	class PrimitiveType : public NativeType
	{
		static_assert( std::is_arithmetic_v< T >, "That's not a primitive type!" );
		friend class ::rtti::RTTI;

	public:
		virtual const char* GetName() const;
		virtual void ConstructInPlace( void* dest ) const 
		{
			*static_cast< T* >( dest ) = T();
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const
		{ 
			std::memcpy( dest, src, sizeof( T ) ); 
		}
#endif
		virtual void Destroy( void* ptr ) const {}

		virtual size_t GetSize() const { return sizeof( T ); }

		virtual bool IsPrimitive() const { return true; }

		static const rtti::PrimitiveType< T >& GetInstance()
		{
			return GetMutableInstance();
		}

		class PointerType : public ::rtti::PointerType
		{
			friend class ::rtti::RTTI;

		public:
			static TypeId CalcId( const ::rtti::Type& internalType ) 
			{ 
				return ::rtti::PointerType::CalcId( internalType );
			}

			static TypeId CalcId() 
			{ 
				return ::rtti::PointerType::CalcId( PrimitiveType< T >::GetInstance() ); 
			}

			template< size_t IndirectionsAmount, std::enable_if_t< IndirectionsAmount == 1, bool > = true >
			static const PointerType& GetInstance()
			{ 
				return RTTI::GetMutable().GetOrRegisterDynamicType< PointerType >();
			}

#if RTTI_ALLOW_MULTI_POINTERS
			template< size_t IndirectionsAmount, std::enable_if_t< (IndirectionsAmount > 1), bool > = true >
			static const PointerType& GetInstance()
			{
				return RTTI::GetMutable().GetOrRegisterDynamicType< PointerType >( GetInstance< IndirectionsAmount - 1 >() );
			}
#endif

		private:
			PointerType( const PointerType& internalType ) 
				: ::rtti::PointerType( static_cast< const Type& >( internalType ) )
			{}

			PointerType() 
				: ::rtti::PointerType( PrimitiveType< T >::GetInstance() )
			{}
		};

	private:
		PrimitiveType< T >() : rtti::NativeType( GetName() ) {}
		static rtti::PrimitiveType< T >& GetMutableInstance()
		{
			static rtti::PrimitiveType< T > s_typeInstance;
			return s_typeInstance;
		}
	};

	template< class T, int Amount, class T2 = void >
	struct extract_indirections_internal {};

	template< class T, int Amount >
	struct extract_indirections_internal< T, Amount, std::enable_if_t< !std::is_pointer< T >::value > > { using type = T; constexpr static int amount = Amount; };

	template< class T, int Amount >
	struct extract_indirections_internal< T, Amount, std::enable_if_t< std::is_pointer< T >::value > > : std::conditional_t< std::is_pointer< T >::value, extract_indirections_internal< std::remove_pointer_t< T >, Amount + 1 >, extract_indirections_internal< T, Amount > > {};

	template< class T >
	struct extract_indirections : extract_indirections_internal< T, 0 > {};

	template <typename> struct is_template : std::false_type {};

	template <template <typename...> class Tmpl, typename ...Args>
	struct is_template<Tmpl<Args...>> : std::true_type {};

	template< class T >
	struct is_vector : std::false_type{};

	template< class T >
	struct is_vector< std::vector< T > > : std::true_type {};

	template< class T, class T2 = void >
	struct type_of {};

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_class_v< T > && !is_template< T >::value > > { using type = T::Type; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_fundamental_v< T > > > { using type = PrimitiveType< T >; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_pointer_v< T > > > { using type = type_of< typename extract_indirections< T >::type >::type::PointerType; };

	template< class T >
	struct type_of< T, std::enable_if_t< is_vector< T >::value > > { using type = VectorType< typename T::value_type >; };

	template< class T, std::enable_if_t< ( std::is_class_v< T > && !is_template< T >::value ) || std::is_fundamental_v< T >, bool > = true >
	const Type& GetTypeInstanceOf()
	{
		return type_of< T >::type::GetInstance();
	}

	template< class T, std::enable_if_t< std::is_pointer_v< T >, bool > = true >
	const ::rtti::PointerType& GetTypeInstanceOf()
	{
		using PointerType = type_of< T >::type;
		return PointerType::template GetInstance< extract_indirections< T >::amount >();
	}

	template< class T, std::enable_if_t< is_vector< T >::value, bool > = true >
	const VectorType< typename T::value_type >& GetTypeInstanceOf()
	{
		return VectorType< typename T::value_type >::GetInstance();
	}
}

#define RTTI_INHERITS_FROM_BODY_true( ParentClassName ) return ParentClassName##::GetTypeStatic() == type || ParentClassName##::GetTypeStatic().InheritsFrom( type );
#define RTTI_INHERITS_FROM_BODY_false( ParentClassName ) return false;

#define RTTI_VIRTUAL_true virtual
#define RTTI_VIRTUAL_false

#define RTTI_INHERITS_FROM_STATIC_BODY_true return Super::GetTypeStatic().IsA< T >() || Super::InheritsFromStatic< T >();
#define RTTI_INHERITS_FROM_STATIC_BODY_false return false;

#define RTTI_CONSTRUCT_INTERNAL_BODY_true( ClassName ) return nullptr;
#define RTTI_CONSTRUCT_INTERNAL_BODY_false( ClassName ) return new ClassName##();

#define RTTI_CONSTRUCT_IN_PLACE_INTERNAL_BODY_true( ClassName, dest )
#define RTTI_CONSTRUCT_IN_PLACE_INTERNAL_BODY_false( ClassName, dest ) new (##dest##) ClassName##();

#if RTTI_REQUIRE_MOVE_CTOR
#define RTTI_MOVE_IN_PLACE_INTERNAL_true( ClassName ) virtual void MoveInPlace( void* dest, void* src  ) const override {}
#define RTTI_MOVE_IN_PLACE_INTERNAL_false( ClassName ) virtual void MoveInPlace( void* dest, void* src ) const override { new (dest) ClassName##(std::move( *static_cast< ClassName##* >( src ) ) ); }
#else
#define RTTI_MOVE_IN_PLACE_INTERNAL_true( ClassName )
#define RTTI_MOVE_IN_PLACE_INTERNAL_false( ClassName )
#endif

#define RTTI_PARENT_CLASS_TYPE_true( ParentClassName ) ParentClassName::Type
#define RTTI_PARENT_CLASS_TYPE_false( ParentClassName ) ::rtti::NativeType

#define RTTI_PARENT_POINTER_TYPE_true( ParentClassName ) ParentClassName##::Type::PointerType
#define RTTI_PARENT_POINTER_TYPE_false( ParentClassName ) ::rtti::PointerType

#define RTTI_DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, Inherits, Virtual, Abstract ) \
public: \
	using ParentClassType = RTTI_PARENT_CLASS_TYPE_##Inherits##( ParentClassName ) ; \
	class Type : public ParentClassType \
	{ \
	friend class ::rtti::RTTI; \
	public: \
		virtual const char* GetName() const override; \
		virtual bool InheritsFrom( const ::rtti::Type& type ) const override \
		{ \
			RTTI_INHERITS_FROM_BODY_##Inherits##( ParentClassName ) \
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
			RTTI_CONSTRUCT_IN_PLACE_INTERNAL_BODY_##Abstract##( ClassName, dest )\
		} \
		RTTI_MOVE_IN_PLACE_INTERNAL_##Abstract##( ClassName ) \
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
			return GetMutableInstance(); \
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
		using ParentPointerType = RTTI_PARENT_POINTER_TYPE_##Inherits##( ParentClassName ) ; \
		class PointerType : public ParentPointerType \
		{ \
		public: \
			static ::rtti::TypeId CalcId( const Type& internalType ) { return ::rtti::PointerType::CalcId( internalType ); } \
			static ::rtti::TypeId CalcId() { return ::rtti::PointerType::CalcId( ClassName##::Type::GetInstance() ); } \
			template< size_t IndirectionsAmount, std::enable_if_t< IndirectionsAmount == 1, bool > = true > \
			static const PointerType& GetInstance() \
			{ \
				return ::rtti::RTTI::GetMutable().GetOrRegisterDynamicType< PointerType >(); \
			} \
			template< size_t IndirectionsAmount, std::enable_if_t< ( IndirectionsAmount > 1 ), bool > = true > \
			static const PointerType& GetInstance() \
			{ \
				return ::rtti::RTTI::GetMutable().GetOrRegisterDynamicType< PointerType >( static_cast< const Type& >( GetInstance< IndirectionsAmount - 1 >() ) ); \
			} \
			PointerType() \
				: ParentPointerType( ClassName##::Type::GetInstance() ) \
			{} \
			PointerType( const Type& internalType ) \
				: ParentPointerType( internalType ) \
			{} \
		}; \
	protected: \
		RTTI_VIRTUAL_##Virtual ClassName##* Construct_Internal() const \
		{ \
			RTTI_CONSTRUCT_INTERNAL_BODY_##Abstract##( ClassName ) \
		} \
		Type(); \
		Type( const char* name ) : ParentClassType ( name ) {} \
		static size_t GetPropertiesAmountStatic() { return GetInstance().GetPropertiesAmount(); } \
		static const ::rtti::Property* GetPropertyStatic( size_t index ) { return GetInstance().GetProperty( index ); } \
		virtual void OnRegistered() override; \
	private: \
		std::vector< std::unique_ptr< ::rtti::Property > > m_properties; \
		static Type& GetMutableInstance() \
		{ \
			static Type s_typeInstance; \
			return s_typeInstance; \
		} \
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
	RTTI_VIRTUAL_##Virtual const Type& GetType() const \
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
		RTTI_INHERITS_FROM_STATIC_BODY_##Inherits; \
	} \
	using ClassType = Type; \
private:

#define RTTI_DECLARE_TYPE_INTERNAL( ClassName, Virtual ) \
RTTI_DECLARE_TYPE_INTERNAL_PARENT( ClassName, ::rtti, false, Virtual, false)

#define RTTI_DECLARE_TYPE_INTERNAL_PARENT_DIRECT( ClassName, ParentClassName, Virtual ) \
RTTI_DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, true, Virtual, false) \
	using Super = ParentClassName;

#define RTTI_EXPAND( x ) x

#define RTTI_GET_DECLARE_TYPE_MACRO(_1,_2,NAME,...) NAME

#define RTTI_DECLARE_POLYMORPHIC_CLASS(...) RTTI_EXPAND(RTTI_GET_DECLARE_TYPE_MACRO(__VA_ARGS__, RTTI_DECLARE_TYPE_INTERNAL_PARENT_DIRECT, RTTI_DECLARE_TYPE_INTERNAL)(__VA_ARGS__, true))

#define RTTI_DECLARE_CLASS(...) RTTI_EXPAND(RTTI_GET_DECLARE_TYPE_MACRO(__VA_ARGS__, RTTI_DECLARE_TYPE_INTERNAL_PARENT_DIRECT, RTTI_DECLARE_TYPE_INTERNAL)(__VA_ARGS__, false))

#define RTTI_DECLARE_STRUCT(...) RTTI_DECLARE_CLASS(__VA_ARGS__) \
public:

#define RTTI_DECLARE_ABSTRACT_TYPE_INTERNAL( ClassName ) \
RTTI_DECLARE_TYPE_INTERNAL_PARENT( ClassName, ::rtti, false, true, true)

#define RTTI_DECLARE_ABSTRACT_TYPE_INTERNAL_PARENT_DIRECT( ClassName, ParentClassName ) \
RTTI_DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, true, true, true) \
	using Super = ParentClassName;

#define RTTI_DECLARE_ABSTRACT_CLASS( ... ) RTTI_EXPAND( RTTI_GET_DECLARE_TYPE_MACRO( __VA_ARGS__, RTTI_DECLARE_ABSTRACT_TYPE_INTERNAL_PARENT_DIRECT, RTTI_DECLARE_ABSTRACT_TYPE_INTERNAL )( __VA_ARGS__ ) )

#define RTTI_CONCAT(X, Y) RTTI_CONCAT2(X, Y)
#define RTTI_CONCAT2(X,Y) X##Y

#define RTTI_REGISTER_TYPE( TypeClass ) \
namespace RTTI_CONCAT( rtti_internal, __COUNTER__ ) \
{ \
namespace \
{ \
struct Initializer { Initializer() { ::rtti::RTTI::GetMutable().RegisterNativeType< TypeClass >(); } }; \
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
RTTI_REGISTER_TYPE( NamespaceClassName##::Type )

#define RTTI_REGISTER_PROPERTY( PropertyName ) m_properties.emplace_back( CreateProperty< decltype( CurrentlyImplementedType::##PropertyName ) >( #PropertyName, offsetof( CurrentlyImplementedType, PropertyName ) ) );

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
RTTI_REGISTER_TYPE( ::rtti::PrimitiveType<##type##> )

RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( float )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int32 )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int64 )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( bool )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned short )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( unsigned long long )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( double )
RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( __int8 )