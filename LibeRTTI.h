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
#include <functional>
#include <stdint.h>

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
	using uint64 = unsigned __int64;

	class Type;
	template< class T, class T2 = void > class PointerType;
	template< class T > class VectorType;
	template< class T > class SetType;
	template< class T1, class T2 > class MapType;
	template< class T, size_t Count > class ArrayType;
	template< class T > class PrimitiveType;
	template< class T > class SharedPtrType;
	template< class T > class UniquePtrType;
	template< class T > class RuntimeType;
	template< class T > class EnumType;
	using ID = uint64;

	namespace internal
	{
		template< class T1, class T2 > class PairType;
	}
}
#pragma endregion

#pragma region VisitOutcome
namespace rtti
{
	enum class VisitOutcome : uint8_t
	{
		Break,
		Continue
	};
}
#pragma endregion

#pragma region InstanceFlags
namespace rtti
{
	enum class InstanceFlags : uint8_t
	{
		None = 0,
		Const = 1 << 0,
		Ref = 1 << 1,
	};

	constexpr InstanceFlags operator|( InstanceFlags lVal, InstanceFlags rVal )
	{
		return static_cast< InstanceFlags >( static_cast< uint8_t >( lVal ) | static_cast< uint8_t >( rVal ) );
	}

	constexpr InstanceFlags operator&( InstanceFlags lVal, InstanceFlags rVal )
	{
		return static_cast< InstanceFlags >( static_cast< uint8_t >( lVal ) & static_cast< uint8_t >( rVal ) );
	}

	template< class T >
	constexpr InstanceFlags GetInstanceFlags()
	{
		return ( std::is_const_v< std::remove_reference_t< T > > ? InstanceFlags::Const : InstanceFlags::None )
			 | ( std::is_reference_v< T > ? InstanceFlags::Ref : InstanceFlags::None );
	}
}
#pragma endregion

#pragma region TypeOf
namespace rtti
{
	namespace internal
	{
		template < typename > struct is_template : std::false_type {};

		template < template < typename... > class Tmpl, typename ...Args>
		struct is_template< Tmpl< Args... > > : std::true_type {};

		template <template < typename... > class Tmpl, typename ...Args>
		struct is_template< const Tmpl< Args... > > : std::true_type {};

#if RTTI_CFG_CREATE_STD_SET_TYPE
		template< class T >
		struct is_vector : std::false_type {};

		template< class T >
		struct is_vector< std::vector< T > > : std::true_type {};

		template< class T >
		struct is_vector< const std::vector< T > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_SET_TYPE
		template< class T >
		struct is_set : std::false_type {};

		template< class T >
		struct is_set< std::unordered_set< T > > : std::true_type {};

		template< class T >
		struct is_set< const std::unordered_set< T > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_PAIR_TYPE
		template< class T >
		struct is_pair : std::false_type {};

		template< class T1, class T2 >
		struct is_pair< std::pair< T1, T2 > > : std::true_type {};

		template< class T1, class T2 >
		struct is_pair< const std::pair< T1, T2 > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_MAP_TYPE
		template< class T >
		struct is_map : std::false_type {};

		template< class T1, class T2 >
		struct is_map< std::unordered_map< T1, T2 > > : std::true_type {};

		template< class T1, class T2 >
		struct is_map< const std::unordered_map< T1, T2 > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
		template< class T >
		struct is_sharedPtr : std::false_type {};

		template< class T >
		struct is_sharedPtr< std::shared_ptr< T > > : std::true_type {};

		template< class T >
		struct is_sharedPtr< const std::shared_ptr< T > > : std::true_type {};
#endif

#if RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
		template< class T >
		struct is_uniquePtr : std::false_type {};

		template< class T >
		struct is_uniquePtr< std::unique_ptr< T > > : std::true_type {};

		template< class T >
		struct is_uniquePtr< const std::unique_ptr< T > > : std::true_type {};
#endif

		template<class T>
		struct is_array : std::false_type {};

		template<class T, std::size_t N>
		struct is_array< T[ N ] > : std::true_type {};

		template<class T, std::size_t N>
		struct is_array< const T[ N ] > : std::true_type {};

		template<class T, std::size_t N>
		struct is_array< std::array< T, N > > : std::true_type {};

		template<class T, std::size_t N>
		struct is_array< const std::array< T, N > > : std::true_type {};
	}

	template< class T, class T2 = void >
	struct type_of {};

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_same_v< T, void > > >{ using type = Type; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_enum_v< T > > > { using type = EnumType< std::remove_cvref_t< T > >; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_class_v< T > && !internal::is_template< T >::value && !internal::is_array< T >::value > > { using type = typename T::Type; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_fundamental_v< T > && !std::is_same_v< T, void >  > > { using type = PrimitiveType< std::remove_cvref_t< T > >; };

	template< class T >
	struct type_of< T, std::enable_if_t< std::is_pointer_v< T > > > { using type = PointerType< std::remove_pointer_t< T > >; };

#if RTTI_CFG_CREATE_STD_VECTOR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_vector< T >::value > > { using type = VectorType< typename T::value_type >; };
#endif

#if RTTI_CFG_CREATE_STD_SET_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_set< T >::value > > { using type = SetType< typename T::value_type >; };
#endif

#if RTTI_CFG_CREATE_STD_PAIR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_pair< T >::value > > { using type = internal::PairType< typename T::first_type, typename T::second_type >; };
#endif

#if RTTI_CFG_CREATE_STD_MAP_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_map< T >::value > > { using type = MapType< typename T::key_type, typename T::mapped_type >; };
#endif

#if RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_sharedPtr< T >::value > > { using type = SharedPtrType< typename T::element_type >; };
#endif

#if RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_uniquePtr< T >::value > > { using type = UniquePtrType< typename T::element_type >; };
#endif

	template< class T >
	struct type_of< T, std::enable_if_t< internal::is_array< T >::value > >{ private: using internalType = std::remove_cvref_t< decltype( *std::begin( std::declval< T& >() ) ) >; public: using type = ArrayType< internalType, sizeof( T ) / sizeof( internalType ) >; };

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
		static constexpr uint64 CalcHash( const char* name, uint64 seed = 0u )
		{
			for ( uint64 i = 0u; name[ i ] != 0; ++i )
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

			template<typename C> static yes& Test( really_has < ID(), &C::CalcId >* );
			template<typename> static no& Test( ... );

		public:
			static constexpr bool value = sizeof( Test<T>( 0 ) ) == sizeof( yes );
		};

		template< class T >
		static const rtti::Type* TryToGetTypeInstance()
		{
			return &rtti::GetTypeInstanceOf< T >();
		}

		template<>
		static const rtti::Type* TryToGetTypeInstance< void >()
		{
			return nullptr;
		}

		template< class T >
		static const rtti::Type* TryToGetInstance()
		{
			return &T::GetInstance();
		}

		template<>
		static const rtti::Type* TryToGetInstance< ::rtti::Type >()
		{
			return nullptr;
		}

		template<int Index, class T, class... Ts>
		struct get_type_from_pack
		{
			using type = typename get_type_from_pack< Index - 1, Ts... >::type;
		};

		template<class T, class... Ts>
		struct get_type_from_pack< 0, T, Ts... >
		{
			using type = T;
		};

		template< class T >
		struct remove_const_from_func
		{
			using type = T;
		};

		template< class R, class TObj, class... Args >
		struct remove_const_from_func< R( TObj::* )( Args... ) const >
		{
			using type = R( TObj::* )( Args... );
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
		std::vector< const Type* > GetTypes() const
		{
			std::vector< const Type* > types;
			types.reserve( m_types.size() );
			for ( const auto& type : m_types )
			{
				types.emplace_back( type.get() );
			}

			return types;
		}

		template< class TFunc >
		void VisitTypes( const TFunc& visitFunc ) const
		{
			for ( const auto& typePtr : m_types )
			{
				if ( visitFunc( *typePtr ) == ::rtti::VisitOutcome::Break )
				{
					break;
				}
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
			ID id = T::CalcId( args... );
			auto found = m_typesLUT.find( id );

			if ( found != m_typesLUT.end() )
			{
				return static_cast< T& >( *found->second );
			}

			T* instance = new T( std::forward< const TArgs& >( args )... );
			m_types.emplace_back( instance );
			m_typesLUT.emplace( id, instance );

			instance->OnRegistered();

			return *instance;
		}

		template< class T, class... TArgs, std::enable_if_t< !internal::has_CalcId< T >::value, bool > = true >
		T& GetOrRegisterType( const TArgs& ... args )
		{
			std::unique_ptr< T > instance( new T( std::forward< const TArgs& >( args )... ) );

			auto currentInstance = m_typesLUT.find( instance->GetID() );

			if ( currentInstance == m_typesLUT.end() )
			{
				T& result = *instance;
				m_typesLUT.emplace( instance->GetID(), instance.get() );
				m_types.emplace_back( std::move( instance ) );

				result.OnRegistered();
				return result;
			}

			return static_cast< T& >( *currentInstance->second );
		}

		const Type* FindType( ID id ) const
		{
			auto found = m_typesLUT.find( id );
			if ( found != m_typesLUT.end() )
			{
				return found->second;
			}

			return nullptr;
		}

		const Type* FindType( const char* name ) const
		{
			ID id = internal::CalcHash( name );
			return FindType( id );
		}

	private:
		std::unordered_map< ID, Type* > m_typesLUT;
		std::vector< std::unique_ptr< Type > > m_types;
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

		const char* GetName() const 
		{ 
			return m_name;
		}

		size_t GetOffset() const
		{
			return m_offset;
		}

		ID GetID() const
		{
			return m_id;
		}

		void* GetAddress( void* owner ) const
		{
			return static_cast< char* >( owner ) + GetOffset();
		}

		const void* GetAddress( const void* owner ) const
		{
			return static_cast< const char* >( owner ) + GetOffset();
		}

		template< class T >
		T& GetValue( void* owner ) const
		{
			return *( static_cast< T* >( GetAddress( owner ) ) );
		}

		template< class T >
		const T& GetValue( const void* owner ) const
		{
			return *( static_cast< const T* >( GetAddress( owner ) ) );
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

		bool HasFlags( InstanceFlags flags ) const
		{
			return ( flags & m_instanceFlags ) == flags;
		}

		InstanceFlags GetFlags() const
		{
			return m_instanceFlags;
		}

	private:
		Property( const char* name, size_t offset, const Type& type, InstanceFlags flags )
			: m_name( name )
			, m_id( internal::CalcHash( name ) )
			, m_offset( offset )
			, m_type( type )
			, m_instanceFlags( flags )
		{}

		const char* m_name = nullptr;
		ID m_id = 0u;
		size_t m_offset = 0u;
		const Type& m_type;
		InstanceFlags m_instanceFlags = InstanceFlags::None;
	};
}

#define RTTI_REGISTER_PROPERTY( PropertyName ) TryToAddProperty( CreateProperty< decltype( CurrentlyImplementedType::##PropertyName ) >( #PropertyName, offsetof( CurrentlyImplementedType, PropertyName ) ) );
#pragma endregion

#pragma region InternalTypeDesc
namespace rtti
{
	class InternalTypeDesc
	{
	public:
		InternalTypeDesc( const Type& type, InstanceFlags flags )
			: m_type( type )
			, m_flags( flags )
		{}

		const Type& GetType() const
		{
			return m_type;
		}

		std::string ConstructName() const
		{
			std::string name;
			AppendName( name );

			return name;
		}

		void AppendName( std::string& destination ) const;

		bool HasFlags( InstanceFlags flags ) const
		{
			return ( m_flags & flags ) == flags;
		}

		InstanceFlags GetFlags() const
		{
			return m_flags;
		}

	private:
		const Type& m_type;
		InstanceFlags m_flags = InstanceFlags::None;
	};
}
#pragma endregion

#pragma region Methods
namespace rtti
{
	namespace internal
	{
		template< class R, class TObj, class... TArgs >
		struct method_signature_common
		{
			static constexpr int ArgsAmount = sizeof...( TArgs );

			template< size_t Index = 0u, class... Args >
			static std::enable_if_t< Index != ArgsAmount, void > Call( R( TObj::* func )( TArgs... ), void* obj, void* args, void* returnVal, Args... pack )
			{
				using T = typename get_type_from_pack< Index, TArgs... >::type;
				uint8_t* argAddress = reinterpret_cast< uint8_t* >( args );
				const size_t offset = ( alignof( T ) - ( reinterpret_cast< size_t >( argAddress ) & ( alignof( T ) - 1 ) ) ) & ( alignof( T ) - 1 );
				argAddress = argAddress + offset;
				Call< Index + 1 >( func, obj, reinterpret_cast< void* >( argAddress + sizeof( T ) ), returnVal, pack..., *reinterpret_cast< std::remove_reference_t< T >* >( argAddress ) );
			}

			template< size_t Index = 0u, class... Args >
			static std::enable_if_t< Index == ArgsAmount && !std::is_same_v< void, R >, void > Call( R( TObj::* func )( TArgs... ), void* obj, void* args, void* returnVal, Args&&... pack )
			{
				*( std::remove_reference_t< R >* )returnVal = ( reinterpret_cast< TObj* >( obj )->*func )( std::forward< Args >( pack )... );
			}

			template< size_t Index = 0u, class... Args >
			static std::enable_if_t< Index == ArgsAmount && std::is_same_v< void, R >, void > Call( R( TObj::* func )( TArgs... ), void* obj, void* args, void* returnVal, Args&&... pack )
			{
				( reinterpret_cast< TObj* >( obj )->*func )( std::forward< Args >( pack )... );
			}

			static const rtti::Type* GetReturnTypeDesc()
			{
				return internal::TryToGetTypeInstance< std::remove_cvref_t< R > >();
			}

			static InstanceFlags GetReturnTypeDescInstanceFlags()
			{
				return GetInstanceFlags< R >();
			}
		};
	}

	template< class T >
	struct method_signature;

	template< class R, class TObj >
	struct method_signature< R( TObj::* )() > : public internal::method_signature_common< R, TObj >
	{
		template< class TFunc >
		static void VisitArgumentTypes( const TFunc& func )
		{}
	};

	template< class R, class TObj, class TArg, class... TArgs >
	struct method_signature< R( TObj::* )( TArg, TArgs... )> : public internal::method_signature_common< R, TObj, TArg, TArgs... >
	{
		template< class TFunc >
		static void VisitArgumentTypes( const TFunc& func )
		{
			if ( func( GetTypeInstanceOf< std::remove_cvref_t< TArg > >(), GetInstanceFlags< TArg >() ) == VisitOutcome::Continue )
			{
				method_signature< R( TObj::* )( TArgs... ) >::VisitArgumentTypes( func );
			}
		}
	};

	using ParameterTypeDesc = InternalTypeDesc;

	class Function
	{
		friend class Type;

	public:
		const char* GetName() const
		{
			return m_name;
		}

		size_t GetParametersAmount() const
		{
			return m_parameterTypeDescs.size();
		}

		const ParameterTypeDesc* GetParameterTypeDesc( size_t index ) const
		{
			if ( index < m_parameterTypeDescs.size() )
			{
				return &m_parameterTypeDescs[ index ];
			}

			return nullptr;
		}

		const ParameterTypeDesc* GetReturnTypeDesc() const
		{
			return m_returnTypeDesc.get();
		}

		void Call( void* obj, void* args, void* ret ) const
		{
			m_func( obj, args, ret );
		}

		ID GetID() const
		{
			return m_id;
		}

	private:
		using InternalFuncType = std::function< void( void*, void*, void* ) >;
		Function( const char* name, std::vector< ParameterTypeDesc > parameterTypeDescs, InternalFuncType func )
			: m_name( name )
			, m_parameterTypeDescs( std::move( parameterTypeDescs ) )
			, m_func( std::move( func ) )
		{
			for ( const ParameterTypeDesc typeDesc : m_parameterTypeDescs )
			{
				m_id = internal::CalcHash( typeDesc.ConstructName().c_str(), m_id );
			}

			m_id = internal::CalcHash( m_name, m_id );
		}

		Function( const char* name, const Type& returnType, InstanceFlags returnTypeFlags, std::vector< ParameterTypeDesc > parameterTypeDescs, InternalFuncType func )
			: Function( name, parameterTypeDescs, func )
		{
			m_returnTypeDesc = std::make_unique< ParameterTypeDesc >( returnType, returnTypeFlags );
		}

		std::vector< ParameterTypeDesc > m_parameterTypeDescs;
		std::unique_ptr< ParameterTypeDesc > m_returnTypeDesc;
		const char* m_name = nullptr;
		InternalFuncType m_func;
		ID m_id = 0u;
	};


#define RTTI_REGISTER_METHOD( MethodName ) TryToAddMethod( CreateFunction( #MethodName , &##CurrentlyImplementedType::##MethodName ) );
}
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
			RawPointer,
			UniquePointer,
			SharedPointer,
			Array,
			Vector,
			Set,
			Map,
			RuntimeType,
			Enum,
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

		virtual const Type* GetParent() const
		{
			return nullptr;
		}

		virtual Kind GetKind() const = 0;

		virtual void ConstructInPlace( void* dest ) const = 0
		{}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const = 0
		{}
#endif
		virtual void Destroy( void* address ) const = 0
		{}

		virtual bool IsAbstract() const
		{
			return false;
		}

		virtual bool IsVirtual() const
		{
			return false;
		}

		ID GetID() const
		{
			return m_id;
		}

		virtual size_t GetSize() const = 0
		{ return 0u; }

		virtual size_t GetAlignment() const = 0
		{ return 0u; }

		virtual size_t GetPropertiesAmount() const { return 0u; }
		virtual const ::rtti::Property* GetProperty( size_t index ) const { return nullptr; }

		const Property* FindProperty( ID wantedId ) const
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

		virtual size_t GetMethodsAmount() const { return 0u; }
		virtual const ::rtti::Function* GetMethod( size_t index ) const { return nullptr; }

		const Function* FindMethod( const char* name ) const
		{
			for ( size_t i = 0; i < GetMethodsAmount(); ++i )
			{
				const Function* method = GetMethod( i );
				if ( strcmp( name, method->GetName() ) == 0 )
				{
					return method;
				}
			}

			return nullptr;
		}

	protected:
		Type( const char* name )
			: Type( internal::CalcHash( name ) )
		{}

		Type( ID id )
			: m_id( id )
		{}

		virtual void OnRegistered() {}

		static ::rtti::Property CreateProperty( const char* name, size_t offset, const Type& type, InstanceFlags flags )
		{
			return ::rtti::Property( name, offset, type, flags );
		}

		template< class T >
		static ::rtti::Property CreateProperty( const char* name, size_t offset )
		{
			static_assert( !std::is_reference_v< T >, "Reference properties are not supported!" );
			static_assert( !std::is_const_v< T >, "Const properties are not supported!" );
			return CreateProperty( name, offset, GetTypeInstanceOf< T >(), GetInstanceFlags< T >() );
		}

		template< class T >
		static ::rtti::Property CreateInternalProperty( const char* name, size_t offset )
		{
			return CreateProperty( name, offset, GetTypeInstanceOf< T >(), GetInstanceFlags< T >() );
		}

		template< class TFunc >
		static ::rtti::Function CreateFunction( const char* name, TFunc funcPtr )
		{
			using TNonConstFunc = internal::remove_const_from_func< TFunc >::type;
			using MethodSignature = method_signature< TNonConstFunc >;
			TNonConstFunc nonConstFuncPtr = reinterpret_cast< TNonConstFunc >( funcPtr );

			std::vector< ParameterTypeDesc > parameterTypes;
			MethodSignature::VisitArgumentTypes( [ &parameterTypes ]( const Type& type, InstanceFlags flags )
				{
					parameterTypes.push_back( { type, flags } );
					return ::rtti::VisitOutcome::Continue;
				} );

			auto func = [ nonConstFuncPtr ]( void* obj, void* args, void* ret )
				{
					MethodSignature::Call( nonConstFuncPtr, obj, args, ret );
				};

			if ( MethodSignature::GetReturnTypeDesc() )
			{
				return ::rtti::Function( name, *MethodSignature::GetReturnTypeDesc(), MethodSignature::GetReturnTypeDescInstanceFlags(), std::move( parameterTypes ), std::move( func ) );
			}
			else
			{
				return ::rtti::Function( name, std::move( parameterTypes ), std::move( func ) );
			}
			
		}

	private:
		ID m_id = 0u;
	};
}
#pragma endregion

#pragma region InternalTypeDescImpl
namespace rtti
{
	inline void InternalTypeDesc::AppendName( std::string& destination ) const
	{
		destination.reserve( destination.size() + strlen( GetType().GetName() ) );
		if ( HasFlags( InstanceFlags::Const ) )
		{
			destination += "const ";
		}

		destination += GetType().GetName();

		if ( HasFlags( InstanceFlags::Ref ) )
		{
			destination += "&";
		}
	}
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

#define RTTI_INTERNAL_GET_PARENT_true( ParentClassName ) virtual const rtti::Type* GetParent() const override { return &ParentClassName::GetTypeStatic(); }
#define RTTI_INTERNAL_GET_PARENT_false( ParentClassName ) 

#define RTTI_INTERNAL_DECLARE_TYPE_PARENT( ClassName, ParentClassName, Inherits, Virtual, Abstract, KindName ) \
using This = ClassName; \
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
		RTTI_INTERNAL_GET_PARENT_##Inherits##( ParentClassName ) \
		std::unique_ptr< ClassName > Construct() const \
		{ \
			return std::unique_ptr< ClassName >( Construct_Internal() ); \
		} \
		virtual void ConstructInPlace( void* dest ) const override \
		{ \
			RTTI_INTERNAL_CONSTRUCT_IN_PLACE_BODY_##Abstract##( ClassName, dest )\
		} \
		RTTI_INTERNAL_MOVE_IN_PLACE_##Abstract##( ClassName ) \
		virtual void Destroy( void* address ) const override \
		{ \
			static_cast< ClassName##* >( address )->~##ClassName##(); \
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
		virtual size_t GetPropertiesAmount() const override \
		{ \
			const auto* parentTypeInstance = ::rtti::internal::TryToGetInstance< ParentClassType >(); \
			return (parentTypeInstance ? parentTypeInstance->GetPropertiesAmount() : 0u) + m_properties.size(); \
		} \
		virtual const ::rtti::Property* GetProperty( size_t index ) const override \
		{ \
			const auto* parentTypeInstance = ::rtti::internal::TryToGetInstance< ParentClassType >(); \
			const size_t inheritedPropertiesAmount = parentTypeInstance ? parentTypeInstance->GetPropertiesAmount() : 0; \
			if( index < inheritedPropertiesAmount ) \
			{ \
				return parentTypeInstance->GetProperty( index ); \
			} \
			else \
			{ \
				return &m_properties[ index - inheritedPropertiesAmount ]; \
			} \
		} \
		virtual size_t GetMethodsAmount() const override \
		{ \
			const auto* parentTypeInstance = ::rtti::internal::TryToGetInstance< ParentClassType >(); \
			return ( parentTypeInstance ? parentTypeInstance->GetMethodsAmount() : 0u ) + m_methods.size(); \
		} \
		virtual const ::rtti::Function* GetMethod( size_t index ) const override \
		{ \
			const auto* parentTypeInstance = ::rtti::internal::TryToGetInstance< ParentClassType >(); \
			const size_t inheritedMethodsAmount = parentTypeInstance ? parentTypeInstance->GetMethodsAmount() : 0; \
			if( index < inheritedMethodsAmount ) \
			{ \
				return parentTypeInstance->GetMethod( index ); \
			} \
			else \
			{ \
				return &m_methods[ index - inheritedMethodsAmount ]; \
			} \
		} \
	protected: \
		RTTI_INTERNAL_VIRTUAL_##Virtual ClassName##* Construct_Internal() const \
		{ \
			RTTI_INTERNAL_CONSTRUCT_BODY_##Abstract##( ClassName ) \
		} \
		Type(); \
		Type( const char* name ) : ParentClassType ( name ) {} \
		virtual void OnRegistered() override; \
	private: \
		void TryToAddProperty( ::rtti::Property&& prop ) \
		{ \
			if ( !FindProperty( prop.GetID() ) ) \
			{ \
				m_properties.emplace_back( std::move( prop ) ); \
			} \
		} \
		void TryToAddMethod( ::rtti::Function&& func ) \
		{ \
			for ( size_t i = 0; i < GetMethodsAmount(); ++i ) \
			{ \
				const ::rtti::Function* method = GetMethod( i ); \
				if ( method->GetID() == func.GetID() ) \
				{ \
					return; \
				} \
			} \
			m_methods.emplace_back( std::move( func ) ); \
		} \
		std::vector< ::rtti::Property > m_properties; \
		std::vector< ::rtti::Function > m_methods; \
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
			static ID CalcId()
			{
				ID id = internal::CalcHash( GetInternalTypeDescStatic().GetType().GetName(), static_cast< uint8_t >( GetInternalTypeDescStatic().GetFlags() ) );
				return internal::CalcHash( c_namePostfix, id );
			}

			static InternalTypeDesc GetInternalTypeDescStatic()
			{
				return { GetTypeInstanceOf< T >(), GetInstanceFlags< T >() };
			}

			virtual InternalTypeDesc GetInternalTypeDesc() const override
			{
				return GetInternalTypeDescStatic();
			}

			static const DerivedClass& GetInstance()
			{
				static const DerivedClass& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< DerivedClass >();
				return s_typeInstance;
			}

			virtual ::rtti::Type::Kind GetKind() const override
			{
				return ::rtti::Type::Kind::RawPointer;
			}

		protected:
			PointerTypeImplementation()
				: ParentClass( CalcId(), GetInternalTypeDescStatic().ConstructName() + c_namePostfix)
			{}

			PointerTypeImplementation( ID typeId, std::string&& name )
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
		virtual InternalTypeDesc GetInternalTypeDesc() const = 0;

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

		virtual void Destroy( void* address ) const override {}
		virtual size_t GetSize() const override { return sizeof( void* ); }
		virtual size_t GetAlignment() const override { return alignof( void* ); }

		virtual const char* GetName() const override { return m_strName.c_str(); }

	protected:
		PointerType( ID typeId, std::string&& name )
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
			static ID CalcId()
			{
				ID id = internal::CalcHash( DerivedType::GetBaseName() );
				id = internal::CalcHash( "< ", id );
				const auto internalTypes = DerivedType::GetInternalTypeDescsStatic();
				id = internal::CalcHash( internalTypes[ 0u ].ConstructName().c_str(), id);

				for ( size_t i = 1u; i < internalTypes.size(); ++i )
				{
					id = internal::CalcHash( ", ", id);
					id = internal::CalcHash( internalTypes[ i ].ConstructName().c_str(), id);
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
				new ( dest ) TrueType();
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
				const auto internalTypes = DerivedType::GetInternalTypeDescsStatic();

				m_name += DerivedType::GetBaseName();
				m_name += "< ";
				internalTypes[ 0u ].AppendName( m_name );

				for ( size_t i = 1u; i < internalTypes.size(); ++i )
				{
					m_name += ", ";
					internalTypes[ i ].AppendName( m_name );
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
			virtual size_t GetPropertiesAmount() const override { return m_properties.size(); }
			virtual const ::rtti::Property* GetProperty( size_t index ) const override
			{
				if ( index < m_properties.size() )
				{
					return &m_properties[ index ];
				}

				return nullptr;
			}

			static constexpr const char* GetBaseName() { return "Pair"; }

			static InternalTypeDesc GetFirstInternalTypeDescStatic()
			{
				return { GetTypeInstanceOf< T1 >(), GetInstanceFlags< T1 >() };
			}

			static InternalTypeDesc GetSecondInternalTypeDescStatic()
			{
				return { GetTypeInstanceOf< T2 >(), GetInstanceFlags< T2 >() };
			}

			InternalTypeDesc GetFirstInternalTypeDesc() const
			{
				return GetFirstInternalTypeDescStatic();
			}

			InternalTypeDesc GetSecondInternalTypeDesc() const
			{
				return GetSecondInternalTypeDescStatic();
			}

			virtual void Destroy( void* address ) const override
			{
				static_cast< std::pair< T1, T2 >* >( address )->~pair< T1, T2 >();
			}

			virtual ::rtti::Type::Kind GetKind() const override
			{
				return ::rtti::Type::Kind::Class;
			}

		private:
			PairType()
			{
				using TrueType = std::pair< T1, T2 >;
				m_properties.emplace_back( Type::CreateInternalProperty< T1 >( "First", offsetof( TrueType, first ) ) );
				m_properties.emplace_back( Type::CreateInternalProperty< T2 >( "Second", offsetof( TrueType, second ) ) );
			}

			static std::array< InternalTypeDesc, 2 > GetInternalTypeDescsStatic()
			{
				return { GetFirstInternalTypeDescStatic(), GetSecondInternalTypeDescStatic() };
			}

			std::vector< ::rtti::Property > m_properties;
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
		virtual InternalTypeDesc GetInternalTypeDesc() const = 0;
		virtual size_t GetElementsAmount( const void* address ) const = 0;

		virtual void VisitElementsAsProperties( const void* containerAddress, const std::function< VisitOutcome( const rtti::Property& ) >& visitFunc ) const = 0;

	protected:
		using Type::Type;
	};

	class DynamicContainerType : public ContainerType
	{
	public:
		virtual void EmplaceElement( void* containerAddress, void* elementAddress ) const = 0;
		virtual void AddDefaultElement( void* containerAddress ) const = 0;
		virtual void Clear( void* containerAddress ) const = 0;
		virtual void RemoveElementAtIndex( void* containerAddress, uint32_t index ) const = 0;

	protected:
		using ContainerType::ContainerType;
	};

	namespace internal
	{
		template< class InternalType, class DerivedType, class TrueType >
		class DynamicContainerTemplateType : public TemplateType< DerivedType, TrueType, DynamicContainerType >
		{
			friend class TemplateType< DerivedType, TrueType, DynamicContainerType >;

		public:
			static InternalTypeDesc GetInternalTypeDescStatic()
			{
				return { InternalType::GetInstance(), GetInstanceFlags< InternalType >() };
			}

			virtual InternalTypeDesc GetInternalTypeDesc() const override
			{
				return GetInternalTypeDescStatic();
			}

			virtual void VisitElementsAsProperties( const void* containerAddress, const std::function< VisitOutcome( const rtti::Property& ) >& visitFunc ) const override
			{
				size_t i = 0u;
				for ( const auto& element : *static_cast< const TrueType* >( containerAddress ) )
				{
					std::string name = "[";
					name += std::to_string( i++ );
					name += "]";
					auto visitOutcome = visitFunc( Type::CreateProperty( name.c_str(), reinterpret_cast< const uint8_t* >( &element ) - reinterpret_cast< const uint8_t* >( containerAddress ), GetInternalTypeDesc().GetType(), GetInternalTypeDesc().GetFlags() ) );
					if ( visitOutcome == VisitOutcome::Break )
					{
						break;
					}
				}
			}

		private:
			static std::array< InternalTypeDesc, 1 > GetInternalTypeDescsStatic()
			{
				return { GetInternalTypeDescStatic() };
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
		static ID CalcId()
		{
			ID id = internal::CalcHash( GetInternalTypeDescStatic().GetType().GetName(), static_cast< uint8_t >( GetInternalTypeDescStatic().GetFlags() ) );
			id = internal::CalcHash( "[", id );
			id = internal::CalcHash( std::to_string( Count ).c_str(), id );
			return internal::CalcHash( "]", id );
		}

		static const ArrayType& GetInstance()
		{
			return ::rtti::RTTI::GetMutable().GetOrRegisterType< ArrayType >();
		}

		static InternalTypeDesc GetInternalTypeDescStatic()
		{
			return { GetTypeInstanceOf< T >(), GetInstanceFlags< T >() };
		}

		InternalTypeDesc GetInternalTypeDesc() const override
		{
			return GetInternalTypeDescStatic();
		}

		const char* GetName() const override
		{
			return m_name.c_str();
		}

		virtual Kind GetKind() const override 
		{ 
			return Kind::Array; 
		}

		virtual size_t GetElementsAmount( const void* address ) const override
		{
			return Count;
		}

		void ConstructInPlace( void* dest ) const override
		{
			using TNonConst = std::remove_const_t< T >;
			TNonConst* arr = static_cast< TNonConst* >( dest );

			for ( size_t i = 0u; i < Count; ++i )
			{
				GetInternalTypeDesc().GetType().ConstructInPlace( &arr[ i ] );
			}
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const override
		{
			using TNonConst = std::remove_const_t< T >;
			TNonConst* destArr = new ( dest ) TNonConst[ Count ];
			TNonConst* sourceArr = static_cast< TNonConst* >( src );

			for ( size_t i = 0u; i < Count; ++i )
			{
				GetInternalTypeDesc().GetType().MoveInPlace( &destArr[ i ], &sourceArr[ i ] );
			}
		}
#endif

		void Destroy( void* address ) const override
		{
			using TNonConst = std::remove_const_t< T >;
			TNonConst* arr = static_cast< TNonConst* >( address );

			for ( size_t i = 0u; i < Count; ++i )
			{
				GetInternalTypeDesc().GetType().Destroy( &arr[i] );
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

		virtual void VisitElementsAsProperties( const void* containerAddress, const std::function< VisitOutcome( const rtti::Property& ) >& visitFunc ) const override
		{
			for ( size_t i = 0u; i < GetElementsAmount( containerAddress ); ++i )
			{
				std::string name = "[";
				name += std::to_string( i );
				name += "]";
				auto visitOutcome = visitFunc( Type::CreateProperty( name.c_str(), i * GetInternalTypeDesc().GetType().GetSize(), GetInternalTypeDesc().GetType(), GetInternalTypeDesc().GetFlags() ) );
				if ( visitOutcome == VisitOutcome::Break )
				{
					break;
				}
			}
		}

	private:
		ArrayType()
			: ContainerType( CalcId() )
		{
			GetInternalTypeDesc().AppendName( m_name );
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

		virtual size_t GetElementsAmount( const void* address ) const override
		{
			return static_cast< const std::vector< T >* >( address )->size();
		}

		virtual void Destroy( void* address ) const override
		{
			static_cast< std::vector< T >* >( address )->~vector< T >();
		}

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::Vector;
		}

		virtual void EmplaceElement( void* containerAddress, void* elementAddress ) const override
		{
			static_cast< std::vector< T >* >( containerAddress )->emplace_back( std::move( *static_cast< T* >( elementAddress ) ) );
		}

		virtual void AddDefaultElement( void* containerAddress ) const override
		{
			static_cast< std::vector< T >* >( containerAddress )->emplace_back();
		}

		virtual void Clear( void* containerAddress ) const override
		{
			static_cast< std::vector< T >* >( containerAddress )->clear();
		}

		virtual void RemoveElementAtIndex( void* containerAddress, uint32_t index ) const override
		{
			std::vector< T >* vector = static_cast< std::vector< T >* >( containerAddress );
			vector->erase( vector->begin() + index );
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

		virtual size_t GetElementsAmount( const void* address ) const override
		{
			return static_cast< const std::unordered_set< T >* >( address )->size();
		}

		virtual void Destroy( void* address ) const override
		{
			static_cast< std::unordered_set< T >* >( address )->~unordered_set< T >();
		}

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::Set;
		}

		virtual void EmplaceElement( void* containerAddress, void* elementAddress ) const override
		{
			static_cast< std::unordered_set< T >* >( containerAddress )->emplace( std::move( *static_cast< T* >( elementAddress ) ) );
		}

		virtual void AddDefaultElement( void* containerAddress ) const override
		{
			static_cast< std::unordered_set< T >* >( containerAddress )->emplace();
		}

		virtual void Clear( void* containerAddress ) const override
		{
			static_cast< std::unordered_set< T >* >( containerAddress )->clear();
		}

		virtual void RemoveElementAtIndex( void* containerAddress, uint32_t index ) const override
		{
			std::unordered_set< T >* set = static_cast< std::unordered_set< T >* >( containerAddress );
			uint32_t i = 0u;
			for ( auto it = set->begin(); it != set->end(); ++it )
			{
				if ( i++ == index )
				{
					set->erase( it );
				}
			}
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
		friend class internal::TemplateType< MapType, std::unordered_map< TKey, TValue >, DynamicContainerType >;

	public:
		static constexpr const char* GetBaseName() { return "Map"; }

		virtual void Destroy( void* address ) const override
		{
			static_cast< std::unordered_map< TKey, TValue >* >( address )->~unordered_map< TKey, TValue >();
		}

		virtual size_t GetElementsAmount( const void* address ) const override
		{
			return static_cast< const std::unordered_map< TKey, TValue >* >( address )->size();
		}

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::Map;
		}

		virtual void EmplaceElement( void* containerAddress, void* elementAddress ) const override
		{
			const std::pair< TKey, TValue >&& pair = std::move( *static_cast< std::pair< TKey, TValue >* >( elementAddress ) );
			static_cast< std::unordered_map< TKey, TValue >* >( containerAddress )->emplace( pair.first, pair.second );
		}

		virtual void AddDefaultElement( void* containerAddress ) const override
		{
			static_cast< std::unordered_map< TKey, TValue >* >( containerAddress )->emplace( TKey(), TValue() );
		}

		virtual void Clear( void* containerAddress ) const override
		{
			static_cast< std::unordered_map< TKey, TValue >* >( containerAddress )->clear();
		}

		virtual void RemoveElementAtIndex( void* containerAddress, uint32_t index ) const override
		{
			std::unordered_map< TKey, TValue >* map = static_cast< std::unordered_map< TKey, TValue >* >( containerAddress );
			uint32_t i = 0u;
			for ( auto it = map->begin(); it != map->end(); ++it )
			{
				if ( i++ == index )
				{
					map->erase( it );
				}
			}
		}

	private:
		static std::array< InternalTypeDesc, 2 > GetInternalTypeDescsStatic()
		{
			return internal::PairType< TKey, TValue >::GetInternalTypeDescsStatic();
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
		virtual void Destroy( void* address ) const override {}

		virtual size_t GetSize() const override { return sizeof( T ); }

		virtual size_t GetAlignment() const override { return alignof( T ); }

		virtual Kind GetKind() const override { return Kind::Primitive; }

		static const rtti::PrimitiveType< T >& GetInstance()
		{
			static const rtti::PrimitiveType< T >& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< rtti::PrimitiveType< T > >();
			return s_typeInstance;
		}

	protected:
		PrimitiveType< T >() : rtti::Type( GetName() ) {}
	};
}

#define RTTI_DECLARE_AND_IMPLEMENT_PRIMITIVE_TYPE( type ) \
template<> \
inline const char* ::rtti::PrimitiveType< type >::GetName() const \
{ \
	return #type; \
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

#pragma region Enums
namespace rtti
{
	class EnumTypeBase : public Type
	{
	public:
		virtual Kind GetKind() const override { return Kind::Enum; }

		using ValueType = unsigned long long;
		struct MemberDesc
		{
			ValueType m_value = 0;
			const char* m_name = nullptr;
		};

		template< class TFunc >
		void VisitMembers( const TFunc& func ) const
		{
			for ( const MemberDesc& member : m_members )
			{
				if ( func( member ) == VisitOutcome::Break )
				{
					break;
				}
			}
		}

		const MemberDesc* GetCurrentMember( const void* address ) const
		{
			return GetMemberForValue( GetCurrentValue( address ) );
		}

		virtual void SetCurrentMember( void* address, const MemberDesc& member ) const = 0;

	protected:
		virtual ValueType GetCurrentValue( const void* address ) const = 0;

		template< class T >
		const MemberDesc* GetMemberForValue( T value ) const
		{
			auto it = std::find_if( m_members.begin(), m_members.end(), [ & ]( MemberDesc member ) { return member.m_value == static_cast< ValueType >( value ); } );
			if ( it != m_members.end() )
			{
				return &*it;
			}

			return nullptr;
		}

		const ValueType* GetValueFromName( const char* name ) const
		{
			auto it = std::find_if( m_members.begin(), m_members.end(), [ & ]( MemberDesc member ) { return strcmp( member.m_name, name ) == 0; } );
			if ( it != m_members.end() )
			{
				return &it->m_value;
			}

			return nullptr;
		}

		using Type::Type;
		std::vector< MemberDesc > m_members;
	};

	template< class T >
	class EnumTypeOfUnderlyingType : public EnumTypeBase
	{
	public:
		using UnderlyingClassType = PrimitiveType< T >;

		virtual ValueType GetCurrentValue( const void* address ) const override
		{
			return static_cast< ValueType >( *reinterpret_cast< const T* >( address ) );
		}

		virtual void SetCurrentMember( void* address, const MemberDesc& member ) const override
		{
			*reinterpret_cast< T* >( address ) = static_cast< T >( member.m_value );
		}

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
		virtual void Destroy( void* address ) const override {}

		virtual size_t GetSize() const override { return sizeof( T ); }

		virtual size_t GetAlignment() const override { return alignof( T ); }

		const UnderlyingClassType& GetUnderlyingType() const
		{
			return UnderlyingClassType::GetInstance();
		}

	protected:
		using EnumTypeBase::EnumTypeBase;
	};

	template< class T >
	class EnumType : public EnumTypeOfUnderlyingType< std::underlying_type_t < T > >
	{
		friend class ::rtti::RTTI;
		using UnderlyingType = std::underlying_type_t< T >;
	public:

		virtual const char* GetName() const;
		
		static const rtti::EnumType< T >& GetInstance()
		{
			static const rtti::EnumType< T >& s_typeInstance = ::rtti::RTTI::GetMutable().GetOrRegisterType< rtti::EnumType< T > >();
			return s_typeInstance;
		}

		const char* GetValueName( T value ) const
		{
			if ( const EnumTypeBase::MemberDesc* member = EnumTypeBase::GetMemberForValue( value ) )
			{
				return member->m_name;
			}

			return nullptr;
		}

		bool GetValueFromName( const char* name, T& outValue ) const
		{
			if ( const EnumTypeBase::ValueType* value = EnumTypeOfUnderlyingType< UnderlyingType >::GetValueFromName( name ) )
			{
				outValue = static_cast< T >( *value );
				return true;
			}

			return false;
		}

	private:
		EnumType< T >() : EnumTypeOfUnderlyingType< std::underlying_type_t < T > >( GetName() ) {}

		virtual void OnRegistered() override;
	};
}

#define RTTI_DECLARE_AND_IMPLEMENT_ENUM( type, ... ) \
template<> \
inline const char* ::rtti::EnumType< type >::GetName() const \
{ \
	return #type; \
} \
void ::rtti::EnumType< type >::OnRegistered() \
{ \
	using CurrentType = type##; \
	using CurrentlyImplementedType = ::rtti::EnumType<##type##>; \
	__VA_ARGS__ \
} \
RTTI_INTERNAL_REGISTER_TYPE( ::rtti::EnumType<##type##> )

#define RTTI_REGISTER_ENUM_MEMBER( Member ) m_members.push_back( { static_cast< EnumTypeBase::ValueType >( CurrentType::##Member ) , #Member } );
#pragma endregion

#pragma region RuntimeType
namespace rtti
{
	namespace internal
	{
		template< class TType >
		TType& CreateType( std::string name )
		{
			if ( ::rtti::Get().FindType( name.c_str() ) )
			{
				throw; // Type with this name already exists
			}

			return ::rtti::RTTI::GetMutable().GetOrRegisterType< TType >( std::move( name ) );
		}

		template< class TParentClass >
		struct RuntimeTypeHelperCommon
		{
			const TParentClass* GetParentTypeInstance() const
			{
				return m_parentType;
			}

		protected:
			using TType = RuntimeType< TParentClass >;
			static TType& CreateInternal( std::string name, const TParentClass& parentType )
			{
				TType& type = CreateType< TType >( std::move( name ) );
				type.RuntimeTypeHelperCommon< TParentClass >::m_parentType = &parentType;
				return type;
			}

		private:
			const TParentClass* m_parentType = nullptr;
		};

		template< class TParentClass >
		struct RuntimeTypeHelper : public RuntimeTypeHelperCommon< TParentClass >
		{
			using TType = RuntimeType< TParentClass >;
			static TType& Create( std::string name )
			{
				return RuntimeTypeHelperCommon< TParentClass >::CreateInternal( std::move( name ), TParentClass::Type::GetInstance() );
			}
		};

		template< class T >
		struct RuntimeTypeHelper< RuntimeType< T > > : public RuntimeTypeHelperCommon< RuntimeType< T > >
		{
			using TType = RuntimeType< RuntimeType< T > >;
			static TType& Create( std::string name, const RuntimeType< T >& parentType )
			{
				return RuntimeTypeHelperCommon< RuntimeType< T > >::CreateInternal( std::move( name ), parentType );
			}
		};

		template<>
		struct RuntimeTypeHelper< rtti::Type >
		{
			using TType = RuntimeType< rtti::Type >;

			static TType& Create( std::string name )
			{
				return CreateType< TType >( std::move( name ) );
			}

			const ::rtti::Type* GetParentTypeInstance() const
			{
				return nullptr;
			}
		};
	}

	template< class ParentClassType = rtti::Type >
	class RuntimeType : public ParentClassType, public internal::RuntimeTypeHelper< ParentClassType >
	{
		friend class ::rtti::RTTI;
		using internal::RuntimeTypeHelper< ParentClassType >::GetParentTypeInstance;
	public:
		using internal::RuntimeTypeHelper< ParentClassType >::Create;
		const ::rtti::Property& AddProperty( const char* name, const Type& type, InstanceFlags flags )
		{
			const auto* parentTypeInstance = GetParentTypeInstance();
			size_t parentSize = parentTypeInstance ? parentTypeInstance->GetSize() : 0;
			size_t currentOffset = parentSize;
			
			if ( GetPropertiesAmount() > 0u )
			{
				const auto& lastProperty = GetProperty( GetPropertiesAmount() - 1u );
				currentOffset = lastProperty->GetOffset() + lastProperty->GetType().GetSize();
				currentOffset = ( ( currentOffset + ( type.GetAlignment() - 1u ) ) & ~( type.GetAlignment() - 1u ) );
			}

			m_properties.emplace_back( ::rtti::Type::CreateProperty( name, currentOffset, type, flags ) );
			m_size = currentOffset - parentSize + type.GetSize();
			m_alignment = std::max( m_alignment, type.GetAlignment() );
			return m_properties.back();
		}

		template< class T >
		const ::rtti::Property& AddProperty( const char* name )
		{
			return AddProperty( name, GetTypeInstanceOf< T >(), GetInstanceFlags< T >() );
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
			if ( const auto* parentTypeInstance = GetParentTypeInstance() )
			{
				parentTypeInstance->ConstructInPlace( dest );
			}

			for ( const auto& property : m_properties )
			{
				const auto& type = property.GetType();
				type.ConstructInPlace( static_cast< uint8_t* >( dest ) + property.GetOffset() );
			}
		}

#if RTTI_REQUIRE_MOVE_CTOR
		virtual void MoveInPlace( void* dest, void* src ) const override
		{
			if ( const auto* parentTypeInstance = GetParentTypeInstance() )
			{
				parentTypeInstance->MoveInPlace( dest, src );
			}

			for ( const auto& property : m_properties )
			{
				const auto& type = property.GetType();
				type.MoveInPlace( static_cast< uint8_t* >( dest ) + property.GetOffset(), static_cast< uint8_t* >( src ) + property.GetOffset() );
			}
		}
#endif

		virtual void Destroy( void* address ) const override
		{
			if ( const auto* parentTypeInstance = GetParentTypeInstance() )
			{
				parentTypeInstance->Destroy( address );
			}

			for ( const auto& property : m_properties )
			{
				const auto& type = property.GetType();
				type.Destroy( static_cast< uint8_t* >( address ) + property.GetOffset() );
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

		virtual size_t GetPropertiesAmount() const override 
		{ 
			const auto* parentTypeInstance = GetParentTypeInstance();
			return (parentTypeInstance ? parentTypeInstance->GetPropertiesAmount() : 0) + m_properties.size(); 
		}
		virtual const ::rtti::Property* GetProperty( size_t index ) const override
		{
			const auto* parentTypeInstance = GetParentTypeInstance();
			const size_t inheritedPropertiesAmount =  parentTypeInstance ? parentTypeInstance->GetPropertiesAmount() : 0;
			if ( index < inheritedPropertiesAmount )
			{
				return parentTypeInstance->GetProperty( index );
			}
			
			if ( index - inheritedPropertiesAmount < m_properties.size() )
			{
				return &m_properties[ index - inheritedPropertiesAmount ];
			}

			return nullptr;
		}

	protected:
		RuntimeType( std::string name )
			: ParentClassType( name.c_str() )
			, m_name( std::move( name ) )
		{
			if ( const auto* parentTypeInstance = GetParentTypeInstance() )
			{
				m_size = parentTypeInstance->GetSize();
				m_alignment = parentTypeInstance->GetAlignment();
			}	
		}

	private:
		std::string m_name;
		std::vector< ::rtti::Property > m_properties;
		size_t m_size = 0;
		size_t m_alignment =  0;
	};
}
#pragma endregion

#pragma region SharedPtr
#if RTTI_CFG_CREATE_STD_SHAREDPTR_TYPE
namespace rtti
{
	class SharedPtrBaseType : public rtti::Type
	{
	public:
		static constexpr const char* GetBaseName() { return "SharedPtr"; }

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::SharedPointer;
		}

		virtual void* GetPointedAddress( void* address ) const = 0;
		virtual const void* GetPointedAddress( const void* address ) const = 0;
		virtual void SetPointedAddress( void* address, void* pointedAddress ) const = 0;
		virtual void AssignSharedPtr( void* src, void* dest ) const = 0;

		virtual InternalTypeDesc GetInternalTypeDesc() const = 0;

	protected:
		using rtti::Type::Type;
	};

	template< class T >
	class SharedPtrType : public internal::TemplateType< SharedPtrType< T >, std::shared_ptr< T >, rtti::SharedPtrBaseType >
	{
		friend class ::rtti::RTTI;
		friend class internal::TemplateType< SharedPtrType, std::shared_ptr< T >, rtti::SharedPtrBaseType >;
		using TNonConst = std::remove_const_t< T >;

	public:
		static InternalTypeDesc GetInternalTypeDescStatic()
		{
			return { GetTypeInstanceOf< T >(), GetInstanceFlags< T >() };
		}

		virtual InternalTypeDesc GetInternalTypeDesc() const override
		{
			return GetInternalTypeDescStatic();
		}

		virtual void Destroy( void* address ) const override
		{
			static_cast< std::shared_ptr< T >* >( address )->~shared_ptr< T >();
		}

		virtual void* GetPointedAddress( void* address ) const override
		{
			return static_cast< std::shared_ptr< TNonConst >* >( address )->get();
		}

		virtual const void* GetPointedAddress( const void* address ) const override
		{
			return static_cast< const std::shared_ptr< T >* >( address )->get();
		}

		virtual void SetPointedAddress( void* address, void* pointedAddress ) const
		{
			auto* ptr = static_cast< std::shared_ptr< TNonConst >* >( address );
			*ptr = std::shared_ptr< TNonConst >( static_cast< TNonConst* >( pointedAddress ) );
		}

		virtual void AssignSharedPtr( void* src, void* dest ) const
		{
			std::shared_ptr< T >* srcShared = static_cast< std::shared_ptr< T >* >( src );
			std::shared_ptr< T >* destShared = static_cast< std::shared_ptr< T >* >( dest );
			*destShared = *srcShared;
		}

	private:
		static std::array< InternalTypeDesc, 1 > GetInternalTypeDescsStatic()
		{
			return { GetInternalTypeDescStatic() };
		}
	};
}
#endif
#pragma endregion

#pragma region UniquePtr
#if RTTI_CFG_CREATE_STD_UNIQUEPTR_TYPE
namespace rtti
{
	class UniquePtrBaseType : public rtti::Type
	{
	public:
		static constexpr const char* GetBaseName() { return "UniquePtr"; }

		virtual ::rtti::Type::Kind GetKind() const override
		{
			return ::rtti::Type::Kind::UniquePointer;
		}

		virtual void* GetPointedAddress( void* address ) const = 0;
		virtual const void* GetPointedAddress( const void* address ) const = 0;
		virtual void SetPointedAddress( void* address, void* pointedAddress ) const = 0;

		virtual InternalTypeDesc GetInternalTypeDesc() const = 0;

	protected:
		using rtti::Type::Type;
	};

	template< class T >
	class UniquePtrType : public internal::TemplateType< UniquePtrType< T >, std::unique_ptr< T >, UniquePtrBaseType >
	{
		friend class ::rtti::RTTI;
		friend class internal::TemplateType< UniquePtrType< T >, std::unique_ptr< T >, UniquePtrBaseType >;
		using TNonConst = std::remove_const_t< T >;

	public:
		static InternalTypeDesc GetInternalTypeDescStatic()
		{
			return { GetTypeInstanceOf< T >(), GetInstanceFlags< T >() };
		}

		virtual InternalTypeDesc GetInternalTypeDesc() const override
		{
			return GetInternalTypeDescStatic();
		}

		virtual void Destroy( void* address ) const override
		{
			static_cast< std::unique_ptr< T >* >( address )->~unique_ptr< T >();
		}

		virtual void* GetPointedAddress( void* address ) const override
		{
			return static_cast< std::unique_ptr< TNonConst >* >( address )->get();
		}

		virtual const void* GetPointedAddress( const void* address ) const override
		{
			return static_cast< const std::unique_ptr< T >* >( address )->get();
		}

		virtual void SetPointedAddress( void* address, void* pointedAddress ) const
		{
			auto* ptr = static_cast< std::unique_ptr< TNonConst >* >( address );
			*ptr = std::unique_ptr< TNonConst >( static_cast< TNonConst* >( pointedAddress ) );
		}

	private:
		static std::array< InternalTypeDesc, 1 > GetInternalTypeDescsStatic()
		{
			return { GetInternalTypeDescStatic() };
		}
	};
}
#endif
#pragma endregion