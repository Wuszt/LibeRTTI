#include "gtest/gtest.h"
#include "RTTIUTClasses.h"
#include <functional>

#define TestCaseName RTTI

typedef float Float;
typedef __int32 Int32;
typedef __int64 Int64;
typedef bool Bool;
typedef unsigned short Uint16;
typedef unsigned Uint32;
typedef unsigned long long Uint64;
typedef double Double;
typedef char Char;
typedef __int8 Byte;

using namespace rttiTest;

TEST( TestCaseName, TypeNameAsString )
{
	A a;
	AA aa;
	B b;

	c::C c_c;

	EXPECT_TRUE( strcmp( A::GetTypeStatic().GetName(), "rttiTest::A" ) == 0 );
	EXPECT_TRUE( strcmp( AA::GetTypeStatic().GetName(), "rttiTest::AA" ) == 0 );
	EXPECT_TRUE( strcmp( B::GetTypeStatic().GetName(), "rttiTest::B" ) == 0 );
	EXPECT_TRUE( strcmp( c::C::GetTypeStatic().GetName(), "rttiTest::c::C" ) == 0 );
	EXPECT_TRUE( strcmp( a.GetType().GetName(), "rttiTest::A" ) == 0 );
	EXPECT_TRUE( strcmp( c_c.GetType().GetName(), "rttiTest::c::C" ) == 0 );
	EXPECT_TRUE( strcmp( aa.GetType().GetName(), "rttiTest::AA" ) == 0 );
	EXPECT_TRUE( strcmp( b.GetType().GetName(), "rttiTest::B" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::vector< A > >().GetName(), "Vector< rttiTest::A >") == 0);
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::unordered_set< Float > >().GetName(), "Set< float >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::pair< Float, Bool > >().GetName(), "Pair< float, bool >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::unordered_map< Float, Bool > >().GetName(), "Map< float, bool >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::shared_ptr< rttiTest::A > >().GetName(), "SharedPtr< rttiTest::A >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::unique_ptr< rttiTest::A > >().GetName(), "UniquePtr< rttiTest::A >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::pair< const Float, const Bool > >().GetName(), "Pair< const float, const bool >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::unordered_map< Float, const Bool > >().GetName(), "Map< float, const bool >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::shared_ptr< const rttiTest::A > >().GetName(), "SharedPtr< const rttiTest::A >" ) == 0 );
	EXPECT_TRUE( strcmp( ::rtti::GetTypeInstanceOf< std::unique_ptr< const rttiTest::A > >().GetName(), "UniquePtr< const rttiTest::A >" ) == 0 );
}

TEST( TestCaseName, IsA )
{
	A a;
	AA aa;
	B b;
	c::C c;
	cc::CC cc;

	EXPECT_TRUE( a.IsA< A >() );
	EXPECT_TRUE( aa.IsA< AA >() );
	EXPECT_TRUE( b.IsA< B >() );
	EXPECT_TRUE( c.IsA< c::C >() );
	EXPECT_TRUE( cc.IsA< cc::CC >() );

	EXPECT_FALSE( a.IsA< AA >() );
	EXPECT_FALSE( a.IsA< B >() );
	EXPECT_FALSE( aa.IsA< A >() );
	EXPECT_FALSE( aa.IsA< B >() );
	EXPECT_FALSE( b.IsA< A >() );
	EXPECT_FALSE( b.IsA< AA >() );

	const A::Type& aTypeClass = AA::GetTypeStatic();
	EXPECT_TRUE( aTypeClass.IsA< AA >() );

	std::unique_ptr<A> aPtr = AA::GetTypeStatic().Construct();
	EXPECT_FALSE( aPtr->IsA< A >() );
	EXPECT_TRUE( aPtr->IsA< AA >() );

	BB bb;
	B* bPtr = &bb;
	EXPECT_TRUE( bPtr->IsA< B >() );
	EXPECT_FALSE( bPtr->IsA< BB >() );

	c::C* cPtr = &cc;
	EXPECT_FALSE( cPtr->IsA< c::C >() );
	EXPECT_TRUE( cPtr->IsA< cc::CC >() );

	const B::Type& bTypeClass = BB::GetTypeStatic();
	EXPECT_TRUE( bTypeClass.IsA< BB >() );
}

TEST( TestCaseName, InheritsFrom )
{
	EXPECT_TRUE( AA::InheritsFromStatic< A >() );
	EXPECT_TRUE( BB::InheritsFromStatic< B >() );
	EXPECT_TRUE( cc::CC::InheritsFromStatic< c::C >() );
	EXPECT_TRUE( abc::InheritsFromAbstract::InheritsFromStatic< abstract::Abstract >() );
	EXPECT_TRUE( abc::AbstractInheritsFromAbstract::InheritsFromStatic< abstract::Abstract >() );

	EXPECT_FALSE( A::InheritsFromStatic< B >() );
	EXPECT_FALSE( B::InheritsFromStatic< BB >() );
	EXPECT_FALSE( A::InheritsFromStatic< A >() );
	EXPECT_FALSE( AA::InheritsFromStatic< AA >() );

	A a;
	AA aa;
	AAA aaa;

	BB bb;
	BBB bbb;

	EXPECT_TRUE( aa.InheritsFrom< A >() );
	EXPECT_TRUE( aaa.InheritsFrom< AA >() );
	EXPECT_TRUE( aaa.InheritsFrom< A >() );

	EXPECT_TRUE( bb.InheritsFrom< B >() );
	EXPECT_TRUE( bbb.InheritsFrom< BB >() );
	EXPECT_TRUE( bbb.InheritsFrom< B >() );

	std::unique_ptr<A> aPtr = AAA::GetTypeStatic().Construct();
	EXPECT_TRUE( aPtr->InheritsFrom< A >() );
	EXPECT_TRUE( aPtr->InheritsFrom< AA >() );

	aPtr = AA::GetTypeStatic().Construct();
	EXPECT_TRUE( aPtr->InheritsFrom< A >() );

	EXPECT_FALSE( a.InheritsFrom< A >() );
	EXPECT_FALSE( aa.InheritsFrom< AA >() );
	EXPECT_FALSE( aaa.InheritsFrom< AAA >() );

	aPtr = AAA::GetTypeStatic().Construct();
	EXPECT_FALSE( aPtr->InheritsFrom< AAA >() );
	aPtr = AA::GetTypeStatic().Construct();
	EXPECT_FALSE( aPtr->InheritsFrom< AA >() );
}

TEST( TestCaseName, GetParent )
{
	EXPECT_TRUE( A::GetTypeStatic().GetParent() == nullptr );
	EXPECT_TRUE( AA::GetTypeStatic().GetParent() == &A::GetTypeStatic() );
	EXPECT_TRUE( AAA::GetTypeStatic().GetParent() == &AA::GetTypeStatic() );
	EXPECT_FALSE( A::GetTypeStatic().GetParent() == &B::GetTypeStatic() );

	EXPECT_TRUE( B::GetTypeStatic().GetParent() == nullptr );
	EXPECT_TRUE( BB::GetTypeStatic().GetParent() == &B::GetTypeStatic() );
}

TEST( TestCaseName, InheritsFromOrIsA )
{
	EXPECT_TRUE( A::InheritsFromOrIsAStatic< A >() );
	EXPECT_TRUE( AA::InheritsFromOrIsAStatic< A >() );
	EXPECT_TRUE( AA::InheritsFromOrIsAStatic< AA >() );
	EXPECT_TRUE( B::InheritsFromOrIsAStatic< B >() );
	EXPECT_TRUE( BB::InheritsFromOrIsAStatic< B >() );
	EXPECT_TRUE( BB::InheritsFromOrIsAStatic< BB >() );
	EXPECT_TRUE( cc::CC::InheritsFromOrIsAStatic< cc::CC >() );

	EXPECT_FALSE( B::InheritsFromOrIsAStatic< BB >() );
	EXPECT_FALSE( A::InheritsFromOrIsAStatic< AA >() );

	A a;
	AA aa;
	AAA aaa;

	BB bb;
	BBB bbb;

	EXPECT_TRUE( aa.InheritsFromOrIsA< A >() );
	EXPECT_TRUE( aaa.InheritsFromOrIsA< AA >() );
	EXPECT_TRUE( aaa.InheritsFromOrIsA< A >() );

	EXPECT_TRUE( bb.InheritsFromOrIsA< B >() );
	EXPECT_TRUE( bbb.InheritsFromOrIsA< BB >() );
	EXPECT_TRUE( bbb.InheritsFromOrIsA< B >() );

	std::unique_ptr<A> aPtr = AAA::GetTypeStatic().Construct();
	EXPECT_TRUE( aPtr->InheritsFromOrIsA< A >() );
	EXPECT_TRUE( aPtr->InheritsFromOrIsA< AA >() );
	EXPECT_TRUE( aPtr->InheritsFromOrIsA< AAA >() );

	aPtr = AA::GetTypeStatic().Construct();
	EXPECT_TRUE( aPtr->InheritsFromOrIsA< A >() );
	EXPECT_TRUE( aPtr->InheritsFromOrIsA< AA >() );
	EXPECT_FALSE( aPtr->InheritsFromOrIsA< AAA >() );
}

TEST( TestCaseName, Construct )
{
	std::unique_ptr<A> aPtr = std::make_unique<AAA>();
	std::unique_ptr<A> aPtr2 = aPtr->GetType().Construct();

	EXPECT_TRUE( aPtr->GetType().IsA< AAA >() );
	EXPECT_TRUE( aPtr->GetType() == aPtr2->GetType() );

	EXPECT_TRUE( abstract::Abstract::GetTypeStatic().Construct() == nullptr );

	std::unique_ptr< abstract::Abstract > aPtr3 = abc::InheritsFromAbstract::GetTypeStatic().Construct();
	EXPECT_TRUE( aPtr3->GetType().Construct()->GetType() == abc::InheritsFromAbstract::GetTypeStatic() );
}

namespace rttiTest
{
	struct TestFoo
	{
		RTTI_DECLARE_STRUCT( TestFoo );
		Uint32 m_value = 123u;
	};
}

RTTI_IMPLEMENT_TYPE( rttiTest::TestFoo );

TEST( TestCaseName, ConstructInPlace )

{
	const ::rtti::Type& type = TestFoo::GetTypeStatic();

	void* buff = new Byte[ type.GetSize() ];

	type.ConstructInPlace( buff );

	EXPECT_EQ( reinterpret_cast< TestFoo* >( buff )->m_value, 123u );

	TestFoo::GetTypeStatic().Destroy( buff );
}

namespace
{
	class Destroyable
	{
		RTTI_DECLARE_POLYMORPHIC_CLASS( Destroyable );

	public:
		Destroyable() = default;
		Destroyable( std::function<void()> func )
			: m_func( func )
		{}

		virtual ~Destroyable()
		{
			if ( m_func )
			{
				m_func();
			}
		}

		void SetFunc( std::function<void()> func )
		{
			m_func = func;
		}

		std::function<void()> m_func;
	};

	RTTI_IMPLEMENT_TYPE( Destroyable );

	class InheritsFromDestroyable : public Destroyable
	{
		RTTI_DECLARE_CLASS( InheritsFromDestroyable, Destroyable );

	public:
		~InheritsFromDestroyable()
		{
			if ( m_func )
			{
				m_func();
			}
		}

		Uint32 m_tmp = 2u;
	};

	RTTI_IMPLEMENT_TYPE( InheritsFromDestroyable );
}

TEST( TestCaseName, Destroy )
{
	{
		Uint32 triggers = 0u;
		Destroyable d( [ & ]() { ++triggers; } );
		const ::rtti::Type& type = d.GetType();

		EXPECT_EQ( triggers, 0u );
		type.Destroy( &d );
		EXPECT_EQ( triggers, 1u );
	}

	{
		Uint32 triggers = 0u;
		const Destroyable::Type& type = InheritsFromDestroyable::GetTypeStatic();
		std::unique_ptr< Destroyable > instance = type.Construct();
		instance->SetFunc( [ & ]() { ++triggers; } );

		EXPECT_EQ( triggers, 0u );
		type.Destroy( instance.get() );
		EXPECT_EQ( triggers, 2u );

		type.ConstructInPlace( instance.get() );
	}
}

TEST( TestCaseName, Size )
{
	EXPECT_EQ( A::GetTypeStatic().GetSize(), sizeof( A ) );
	EXPECT_EQ( AA::GetTypeStatic().GetSize(), sizeof( AA ) );
	EXPECT_EQ( AAA::GetTypeStatic().GetSize(), sizeof( AAA ) );
	EXPECT_EQ( B::GetTypeStatic().GetSize(), sizeof( B ) );
	EXPECT_EQ( BB::GetTypeStatic().GetSize(), sizeof( BB ) );
	EXPECT_EQ( BBB::GetTypeStatic().GetSize(), sizeof( BBB ) );
	EXPECT_EQ( c::C::GetTypeStatic().GetSize(), sizeof( c::C ) );
	EXPECT_EQ( cc::CC::GetTypeStatic().GetSize(), sizeof( cc::CC ) );
	EXPECT_EQ( abstract::Abstract::GetTypeStatic().GetSize(), sizeof( abstract::Abstract ) );
	EXPECT_EQ( abc::AbstractInheritsFromAbstract::GetTypeStatic().GetSize(), sizeof( abc::AbstractInheritsFromAbstract ) );
	EXPECT_EQ( abc::InheritsFromAbstract::GetTypeStatic().GetSize(), sizeof( abc::InheritsFromAbstract ) );
}

TEST( TestCaseName, AreNonPrimitiveTypesRegistered )
{
	Bool anyNonPrimitives = false;
	::rtti::Get().VisitTypes( [ & ]( const ::rtti::Type& type )
		{
			anyNonPrimitives |= type.GetKind() != rtti::Type::Kind::Primitive;
			return ::rtti::VisitOutcome::Continue;
		} );

	EXPECT_TRUE( anyNonPrimitives );
}

TEST( TestCaseName, ArePrimitiveTypesRegistered )
{
	Bool anyPrimitives = false;
	std::vector< const ::rtti::Type* > types = ::rtti::Get().GetTypes();
	for ( const ::rtti::Type* type : types )
	{
		anyPrimitives |= type->GetKind() == rtti::Type::Kind::Primitive;
	}

	EXPECT_TRUE( anyPrimitives );
}

TEST( TestCaseName, PrimitiveTypesNames )
{
	EXPECT_EQ( std::strcmp( ::rtti::PrimitiveType< Float >::GetInstance().GetName(), "float" ), 0 );
	EXPECT_EQ( std::strcmp( ::rtti::PrimitiveType< Double >::GetInstance().GetName(), "double" ), 0 );
	EXPECT_EQ( std::strcmp( ::rtti::PrimitiveType< __int64 >::GetInstance().GetName(), "__int64" ), 0 );
}

TEST( TestCaseName, PrimitiveTypesEquality )
{
	const ::rtti::Type& floatType = ::rtti::PrimitiveType< Float >::GetInstance();
	EXPECT_TRUE( floatType.IsA< Float >() );
	EXPECT_FALSE( floatType.IsA< Bool >() );
	EXPECT_FALSE( floatType.IsA< Int32 >() );
}

TEST( TestCaseName, PrimitiveTypesSizes )
{
	EXPECT_EQ( ::rtti::PrimitiveType< Float >::GetInstance().GetSize(), 4u );
	EXPECT_EQ( ::rtti::PrimitiveType< Double >::GetInstance().GetSize(), 8u );
	EXPECT_EQ( ::rtti::PrimitiveType< Bool >::GetInstance().GetSize(), 1u );
	EXPECT_EQ( ::rtti::PrimitiveType< Uint64 >::GetInstance().GetSize(), 8u );
}

TEST( TestCaseName, TypesUniqueIDs )
{
	std::unordered_set< Uint64 > ids;
	::rtti::Get().VisitTypes( [ & ]( const ::rtti::Type& type )
		{
			EXPECT_FALSE( ids.contains( type.GetID() ) );
			ids.emplace( type.GetID() );
			return ::rtti::VisitOutcome::Continue;
		} );
}

TEST( TestCaseName, PersistentTypesIds )
{
	EXPECT_EQ( ::rtti::PrimitiveType< Float >::GetInstance().GetID(), 97526364u );
	EXPECT_EQ( ::rtti::PrimitiveType< Bool >::GetInstance().GetID(), 3029738u );
}

namespace rttiTest
{
	struct StructWithProperties
	{
		RTTI_DECLARE_STRUCT( StructWithProperties );
		TestFoo m_firstValue;
		TestFoo m_secondValue;
		TestFoo m_thirdValue;
		Float m_primitiveType = 0.0f;
		Float**** m_primitiveTypePtr = nullptr;
		A**** m_userTypePtr = nullptr;
		StructWithProperties**** m_pointerToOwnerType = nullptr;
		std::vector< Float > m_vectorFloat;
		const Float* m_constPtr = nullptr;
		std::vector< const Float* > m_vectorOfPtrs;
		std::vector< Float >* m_ptrToVector = nullptr;
		std::array< Float, 123 > m_stdArray;
		Float m_arr[ 321 ];
		Float m_multiArray[5][12];
		std::array< std::array< Float, 3 >[5], 9 > m_mixedArray;
		Float (*m_pointerToArray)[ 12 ] = nullptr; 
		std::array< const Float, 4 >* m_pointerToStdArray = nullptr;
		std::unordered_set< Float > m_set;
		std::pair< const Float, const TestFoo > m_pair;
		std::unordered_map< Float, const Bool > m_map;
		std::shared_ptr< Float > m_sharedPtr;
		std::unique_ptr< Float > m_uniquePtr;
	};

	struct StructWithPropertiesInherited : public StructWithProperties
	{
		RTTI_DECLARE_STRUCT( StructWithPropertiesInherited, rttiTest::StructWithProperties );
		Bool m_boolean = false;
	};
}

RTTI_IMPLEMENT_TYPE( rttiTest::StructWithProperties,
	RTTI_REGISTER_PROPERTY( m_firstValue );
	RTTI_REGISTER_PROPERTY( m_thirdValue );
	RTTI_REGISTER_PROPERTY( m_primitiveType );
	RTTI_REGISTER_PROPERTY( m_primitiveTypePtr );
	RTTI_REGISTER_PROPERTY( m_userTypePtr );
	RTTI_REGISTER_PROPERTY( m_pointerToOwnerType );
	RTTI_REGISTER_PROPERTY( m_vectorFloat );
	RTTI_REGISTER_PROPERTY( m_constPtr );
	RTTI_REGISTER_PROPERTY( m_vectorOfPtrs );
	RTTI_REGISTER_PROPERTY( m_ptrToVector );
	RTTI_REGISTER_PROPERTY( m_stdArray );
	RTTI_REGISTER_PROPERTY( m_arr );
	RTTI_REGISTER_PROPERTY( m_multiArray );
	RTTI_REGISTER_PROPERTY( m_mixedArray );
	RTTI_REGISTER_PROPERTY( m_pointerToArray );
	RTTI_REGISTER_PROPERTY( m_pointerToStdArray );
	RTTI_REGISTER_PROPERTY( m_set );
	RTTI_REGISTER_PROPERTY( m_pair );
	RTTI_REGISTER_PROPERTY( m_map );
	RTTI_REGISTER_PROPERTY( m_sharedPtr );
	RTTI_REGISTER_PROPERTY( m_uniquePtr );
)

RTTI_IMPLEMENT_TYPE( rttiTest::StructWithPropertiesInherited,
	RTTI_REGISTER_PROPERTY( m_boolean )
)

TEST( TestCaseName, PropertiesOffsets )
{
	auto func = [ & ]< class T >( )
	{
		Uint32 performedTests = 0u;
		for ( Uint32 i = 0u; i < T::GetTypeStatic().GetPropertiesAmount(); ++i )
		{
			const ::rtti::Property* property = T::GetTypeStatic().GetProperty( i );
			if ( strcmp( property->GetName(), "m_firstValue" ) == 0 )
			{
				EXPECT_EQ( property->GetOffset(), 0u );
				++performedTests;
			}
			else if ( strcmp( property->GetName(), "m_thirdValue" ) == 0 )
			{
				EXPECT_EQ( property->GetOffset(), 8u );
				++performedTests;
			}
			else if ( strcmp( property->GetName(), "m_primitiveType" ) == 0 )
			{
				EXPECT_TRUE( property->GetType().GetKind() == rtti::Type::Kind::Primitive );
				EXPECT_EQ( property->GetOffset(), 12u );
				++performedTests;
			}
		}

		EXPECT_EQ( performedTests, 3u );
	};

	func.operator() < rttiTest::StructWithProperties > ( );
	func.operator() < rttiTest::StructWithPropertiesInherited > ( );

	{
		Bool performedTest = false;
		for ( Uint32 i = 0u; i < rttiTest::StructWithPropertiesInherited::GetTypeStatic().GetPropertiesAmount(); ++i )
		{
			const ::rtti::Property* property = rttiTest::StructWithPropertiesInherited::GetTypeStatic().GetProperty( i );

			if ( strcmp( property->GetName(), "m_boolean" ) == 0 )
			{
				EXPECT_EQ( property->GetOffset(), sizeof( rttiTest::StructWithProperties ) );
				performedTest = true;
			}
		}
		EXPECT_TRUE( performedTest );
	}
}

TEST( TestCaseName, PropertiesUniqueIDs )
{
	std::unordered_set< Uint64 > ids;
	for ( Uint32 i = 0u; i < StructWithPropertiesInherited::GetTypeStatic().GetPropertiesAmount(); ++i )
	{
		const ::rtti::Property* property = StructWithPropertiesInherited::GetTypeStatic().GetProperty( i );
		EXPECT_FALSE( ids.contains( property->GetID() ) );
		ids.emplace( property->GetID() );
	}
}

TEST( TestCaseName, PersistentPropertiesIds )
{
	EXPECT_EQ( StructWithPropertiesInherited::GetTypeStatic().FindProperty( "m_firstValue" )->GetID(), 2850178305894955955 );
	EXPECT_EQ( StructWithPropertiesInherited::GetTypeStatic().FindProperty( "m_boolean" )->GetID(), 95669080446550 );
}

TEST( TestCaseName, CastingProperties )
{
	EXPECT_TRUE( StructWithPropertiesInherited::GetTypeStatic().FindProperty( "m_primitiveType" )->GetType() == rtti::GetTypeInstanceOf< Float >() );
	EXPECT_FALSE( StructWithPropertiesInherited::GetTypeStatic().FindProperty( "m_primitiveType" )->GetType() == rtti::GetTypeInstanceOf< Int32 > () );
}

TEST( TestCaseName, SetAndGetValueThroughProperty )
{
	StructWithPropertiesInherited owner;
	owner.m_primitiveType = 123u;

	const rtti::Type& type = StructWithPropertiesInherited::GetTypeStatic();

	const rtti::Property* property = type.FindProperty( "m_primitiveType" );

	property->SetValue( &owner, 321.0f );
	EXPECT_EQ( owner.m_primitiveType, 321.0f );

	EXPECT_EQ( property->GetValue< float >( &owner ), 321.0f );
}

TEST( TestCaseName, type_of )
{
	// Various types
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< Float >::type, ::rtti::PrimitiveType< Float > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< StructWithPropertiesInherited >::type, StructWithPropertiesInherited::Type >;
		EXPECT_TRUE( same );
	}

	// Pointers
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< Float* >::type, ::rtti::PointerType< Float > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< Float**** >::type, ::rtti::PointerType< Float*** > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< StructWithPropertiesInherited* >::type, ::rtti::PointerType< StructWithPropertiesInherited > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< StructWithPropertiesInherited******* >::type, ::rtti::PointerType< StructWithPropertiesInherited****** > >;
		EXPECT_TRUE( same );
	}

	// Vectors
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::vector< Float > >::type, ::rtti::VectorType< Float > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::vector< StructWithPropertiesInherited > >::type, ::rtti::VectorType< StructWithPropertiesInherited > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::vector< Float**** > >::type, ::rtti::VectorType< Float**** > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::vector< StructWithPropertiesInherited* > >::type, ::rtti::VectorType< StructWithPropertiesInherited* > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::vector< std::vector< std::vector< Float > > > >::type, ::rtti::VectorType< std::vector< std::vector< Float > > > >;
		EXPECT_TRUE( same );
	}

	//Arrays
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::array< Float, 5 > >::type, ::rtti::ArrayType< Float, 5 > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< Float[ 5 ] >::type, ::rtti::ArrayType< Float, 5 > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::array< std::array< Float, 5 >, 8 > >::type, ::rtti::ArrayType< std::array< Float, 5 >, 8 > >;
		EXPECT_TRUE( same );
	}

	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< Float[ 5 ][ 8 ] >::type, ::rtti::ArrayType< Float[ 8 ], 5 > >;
		EXPECT_TRUE( same );
	}

	//Sets
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::unordered_set< std::unordered_set< Float > > >::type, ::rtti::SetType< std::unordered_set< Float > > >;
		EXPECT_TRUE( same );
	}

	//Pair
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::pair< std::pair< Float, Int32 >, std::pair< Bool, Double > > >::type, ::rtti::PairType< std::pair< Float, Int32 >, std::pair< Bool, Double > > >;
		EXPECT_TRUE( same );
	}

	//Map
	{
		constexpr Bool same = std::is_same_v< ::rtti::type_of< std::unordered_map< std::unordered_map< Float, Int32 >, std::unordered_map< Bool, Double > > >::type, ::rtti::MapType< std::unordered_map< Float, Int32 >, std::unordered_map< Bool, Double > > >;
		EXPECT_TRUE( same );
	}
}

TEST( TestCaseName, GetTypeInstanceOf )
{
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< Float >(), ::rtti::PrimitiveType< Float >::GetInstance() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< rttiTest::A >(), rttiTest::A::GetTypeStatic() );

	EXPECT_EQ( ::rtti::GetTypeInstanceOf< rttiTest::A* >(), ::rtti::PointerType< rttiTest::A >::GetInstance() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< Float* >(), ::rtti::PointerType< Float >::GetInstance() );

	EXPECT_EQ( ::rtti::GetTypeInstanceOf< std::vector< Float > >(), ::rtti::VectorType< Float >::GetInstance() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< std::unordered_set< Float > >(), ::rtti::SetType< Float >::GetInstance() );
	EXPECT_EQ( ( ::rtti::GetTypeInstanceOf< std::pair< Float, Bool > >() ), ( ::rtti::PairType< Float, Bool >::GetInstance() ) );
}

TEST( TestCaseName, Pointers )
{
	StructWithProperties str;

	const auto& rt = rtti::RTTI::Get();
	const auto& type = str.GetType();

	auto testFunc = [ & ]( const char* propName, const char* typeName )
	{
		const auto* prop = type.FindProperty( propName );
		const auto& propType = prop->GetType();

		auto internalTypesExtractor = [ & ]( const ::rtti::Type& inType, Uint32 deepness )
		{
			const ::rtti::Type* type = &inType;
			for ( Uint32 i = 0u; i < deepness; ++i )
			{
				type = &static_cast< const ::rtti::PointerType<>* >( type )->GetInternalTypeDesc().GetType();
			}

			return type;
		};

		{
			const ::rtti::Type* testType = internalTypesExtractor( propType, 0 );
			EXPECT_EQ( std::string( testType->GetName() ), std::string( typeName ) + "****" );
		}

		{
			const ::rtti::Type* testType = internalTypesExtractor( propType, 1 );
			EXPECT_EQ( std::string( testType->GetName() ), std::string( typeName ) + "***" );
		}

		{
			const ::rtti::Type* testType = internalTypesExtractor( propType, 2 );
			EXPECT_EQ( std::string( testType->GetName() ), std::string( typeName ) + "**" );
		}

		{
			const ::rtti::Type* testType = internalTypesExtractor( propType, 3 );
			EXPECT_EQ( std::string( testType->GetName() ), std::string( typeName ) + "*" );
		}

		{
			const ::rtti::Type* testType = internalTypesExtractor( propType, 4 );
			EXPECT_EQ( std::string( testType->GetName() ), std::string( typeName ) );
		}
	};

	testFunc( "m_primitiveTypePtr", "float" );
	testFunc( "m_userTypePtr", rttiTest::A::GetTypeStatic().GetName() );
	testFunc( "m_pointerToOwnerType", rttiTest::StructWithProperties::GetTypeStatic().GetName() );
}

static void TestContainer( const ::rtti::ContainerType* containerType )
{
	ASSERT_TRUE( containerType );

	containerType = dynamic_cast< const ::rtti::ContainerType* >( &containerType->GetInternalTypeDesc().GetType() );
	ASSERT_TRUE( containerType );

	containerType = dynamic_cast< const ::rtti::ContainerType* >( &containerType->GetInternalTypeDesc().GetType() );
	ASSERT_TRUE( containerType );

	const ::rtti::PrimitiveType< Float >* floatType = dynamic_cast< const ::rtti::PrimitiveType< Float >* >( &containerType->GetInternalTypeDesc().GetType() );
	ASSERT_TRUE( floatType );
}

TEST( TestCaseName, Vectors )
{
	TestContainer( &::rtti::GetTypeInstanceOf< std::vector< std::vector< std::vector< Float > > > >() );
}

TEST( TestCaseName, Arrays )
{
	TestContainer( &::rtti::GetTypeInstanceOf< std::array< std::array< std::array< Float, 3 >, 2 >, 1 > >() );

	EXPECT_TRUE( ( ::rtti::GetTypeInstanceOf< std::array< Float, 5 > >() == ::rtti::GetTypeInstanceOf< Float[ 5 ] >() ) );
}

TEST( TestCaseName, ArraysConstructInPlace )
{
	Int32 buffer[ 100 ];
	for ( size_t i = 0u; i < sizeof( buffer ) / sizeof( Int32 ); ++i )
	{
		buffer[ i ] = 123;
	}

	const auto& arrType = ::rtti::GetTypeInstanceOf< decltype( buffer ) >();
	arrType.ConstructInPlace( &buffer );

	for ( size_t i = 0u; i < sizeof( buffer ) / sizeof( Int32 ); ++i )
	{
		EXPECT_EQ( buffer[ i ], 0 );
	}
}

namespace rttiTest
{
	void Foo( const ::rtti::PointerType< StructWithProperties >& ptrType )
	{
		EXPECT_TRUE( ptrType == ::rtti::PointerType< StructWithPropertiesInherited >::GetInstance() );
		EXPECT_FALSE( ptrType.InheritsFrom< StructWithPropertiesInherited >() );
		EXPECT_FALSE( ptrType.InheritsFrom< StructWithProperties >() );
		EXPECT_FALSE( ptrType.IsA< StructWithProperties >() );
		EXPECT_FALSE( ptrType.IsA< StructWithProperties* >() );
		EXPECT_TRUE( ptrType.IsA< StructWithPropertiesInherited* >() );
	}
}

TEST( TestCaseName, PointerTypesInheritance )
{
	rttiTest::Foo( ::rtti::PointerType< StructWithPropertiesInherited >::GetInstance() );
}

TEST( TestCaseName, Pairs )
{
	const auto& type = ::rtti::GetTypeInstanceOf< std::pair< std::pair< Bool, Double >, std::pair< Int32, Float > > >();
	EXPECT_EQ( ( type.GetFirstInternalTypeDesc().GetType() ), ( ::rtti::GetTypeInstanceOf< std::pair< Bool, Double > >() ) ) ;
	EXPECT_EQ( ( type.GetSecondInternalTypeDesc().GetType() ), ( ::rtti::GetTypeInstanceOf< std::pair< Int32, Float > >() ) );
}

TEST( TestCaseName, Maps )
{
	const auto& type = ::rtti::GetTypeInstanceOf< std::unordered_map< Double, std::unordered_map< Int32, Float > > >();
	const auto& internalType = static_cast< const ::rtti::PairType< Double, std::unordered_map< Int32, Float > >& >( type.GetInternalTypeDesc().GetType() );
	EXPECT_EQ( ( internalType.GetFirstInternalTypeDesc().GetType() ), ( ::rtti::GetTypeInstanceOf< Double >() ));
	EXPECT_EQ( ( internalType.GetSecondInternalTypeDesc().GetType() ), ( ::rtti::GetTypeInstanceOf< std::unordered_map< Int32, Float > >() ));
}

TEST( TestCaseName, RuntimeTypeWrittingAndReading )
{
	::rtti::RuntimeType<>& testType = ::rtti::RuntimeType<>::Create( "RuntimeTypeWrittingAndReading" );
	testType.AddProperty< Float >( "FloatVar" );
	testType.AddProperty< StructWithPropertiesInherited >( "StructVar" );
	testType.AddProperty< Uint32 >( "Uint32" );
	
	std::unique_ptr <Byte[] > buffer( new Byte[ testType.GetSize() ] );

	testType.ConstructInPlace( buffer.get() );
	testType.FindProperty( "FloatVar" )->GetValue< Float >( buffer.get() ) = 321.123f;
	testType.FindProperty( "Uint32" )->GetValue< Uint32 >( buffer.get() ) = 1234567u;

	EXPECT_EQ( testType.FindProperty( "FloatVar" )->GetValue< Float >( buffer.get() ), 321.123f );
	EXPECT_EQ( testType.FindProperty( "Uint32" )->GetValue< Uint32 >( buffer.get() ), 1234567u );
}

namespace rttiTest
{
	struct BaseStruct
	{
		RTTI_DECLARE_STRUCT( BaseStruct );
		Byte m_byte;
	};

	struct SomeStruct : public BaseStruct
	{
		RTTI_DECLARE_STRUCT( SomeStruct, BaseStruct );

		Float m_float;
		Double m_double;
	};
}

RTTI_IMPLEMENT_TYPE( rttiTest::BaseStruct,
	RTTI_REGISTER_PROPERTY( m_byte ))

RTTI_IMPLEMENT_TYPE( rttiTest::SomeStruct,
	RTTI_REGISTER_PROPERTY( m_float )
	RTTI_REGISTER_PROPERTY( m_double ))

static void TestRuntimeType(const ::rtti::Type& testType)
{
	rttiTest::SomeStruct someStruct;
	testType.FindProperty( "m_byte" )->GetValue< Byte >( &someStruct ) = 12;
	testType.FindProperty( "m_float" )->GetValue< Float >( &someStruct ) = 321.123f;
	testType.FindProperty( "m_double" )->GetValue< Double >( &someStruct ) = 567.765;

	EXPECT_EQ( someStruct.m_byte, 12 );
	EXPECT_EQ( someStruct.m_float, 321.123f );
	EXPECT_EQ( someStruct.m_double, 567.765 );
}

TEST( TestCaseName, RuntimeTypePropertiesWithAllignment )
{
	{
		auto& testType = ::rtti::RuntimeType<>::Create( "RuntimeTypePropertiesWithAllignment0" );
		const auto& someType = ::rtti::GetTypeInstanceOf< rttiTest::SomeStruct >();
		for ( Uint32 i = 0u; i < someType.GetPropertiesAmount(); ++i )
		{
			const auto& property = someType.GetProperty( i );
			testType.AddProperty( property->GetName(), property->GetType(), rtti::InstanceFlags::None );
		}

		TestRuntimeType( testType );
	}

	{
		auto& testType = ::rtti::RuntimeType< SomeStruct >::Create( "RuntimeTypePropertiesWithAllignment1" );
		TestRuntimeType( testType );
	}
}

TEST( TestCaseName, SharedPtrs )
{
	const auto& type = ::rtti::GetTypeInstanceOf< std::shared_ptr< std::shared_ptr< Float > > >();
	EXPECT_EQ( type.GetInternalTypeDesc().GetType(), ::rtti::GetTypeInstanceOf< std::shared_ptr< Float > >() );
	const auto& internalType = static_cast< const ::rtti::SharedPtrType< Float >& >( type.GetInternalTypeDesc().GetType() );
	EXPECT_EQ( internalType.GetInternalTypeDesc().GetType(), ::rtti::GetTypeInstanceOf < Float >() );
}

TEST( TestCaseName, UniquePtrs )
{
	const auto& type = ::rtti::GetTypeInstanceOf< std::unique_ptr< std::unique_ptr< Float > > >();
	EXPECT_EQ( type.GetInternalTypeDesc().GetType(), ::rtti::GetTypeInstanceOf< std::unique_ptr< Float > >() );
	const auto& internalType = static_cast< const ::rtti::UniquePtrType< Float >& >( type.GetInternalTypeDesc().GetType() );
	EXPECT_EQ( internalType.GetInternalTypeDesc().GetType(), ::rtti::GetTypeInstanceOf < Float >() );
}

TEST( TestCaseName, InstanceFlags )
{
	const auto& type = rttiTest::StructWithProperties::GetTypeStatic();

	EXPECT_FALSE( type.FindProperty( "m_constPtr" )->HasFlags( ::rtti::InstanceFlags::Ref ) );
	EXPECT_FALSE( type.FindProperty( "m_constPtr" )->HasFlags( ::rtti::InstanceFlags::Const ) );

	const auto* prop = type.FindProperty( "m_constPtr" );
	ASSERT_TRUE( prop );

	const auto& internalType = static_cast< const ::rtti::PointerType< void >& >( prop->GetType() );

	EXPECT_FALSE( internalType.GetInternalTypeDesc().HasFlags( ::rtti::InstanceFlags::Ref ) );
	EXPECT_TRUE( internalType.GetInternalTypeDesc().HasFlags( ::rtti::InstanceFlags::Const ) );
}

namespace rttiTest
{
	struct CFoo
	{
		RTTI_DECLARE_STRUCT( CFoo );

		void FooFunc( Int32 a, Float b, AAA aaa, std::vector< Bool > vec ) {}
		virtual Int32 FooFunc2() { return 123; }

		Int32 FooSum( const Int32 a, Int32 b ){ return a + b; }
		void FooRef( Int32& ref ) { ++ref; }
		Int32 FooConstRef( const Int32& ref ) { return ref; }
		void FooPtr( Int32* ptr ) { *ptr = 123; }
		Int32 FooConstPtr( const Int32* ptr ) { return *ptr; }
		Float FooGet() const { return m_floatVar; }
		void FooSet( Float val ) { m_floatVar = val; }
		Int32& FooGetRef() { static Int32 f = 123; return f; }

		Float m_floatVar = 3.14f;
	};

	struct CFooChild : public CFoo
	{
		RTTI_DECLARE_STRUCT( CFooChild, CFoo );
		virtual Int32 FooFunc2() override
		{
			return 321;
		}

		Int32 FooFunc3()
		{
			return 666;
		}
	};
}

RTTI_IMPLEMENT_TYPE( rttiTest::CFoo,
	RTTI_REGISTER_METHOD( FooFunc );
	RTTI_REGISTER_METHOD( FooFunc2 );
	RTTI_REGISTER_METHOD( FooSum );
	RTTI_REGISTER_METHOD( FooRef ); 
	RTTI_REGISTER_METHOD( FooConstRef );
	RTTI_REGISTER_METHOD( FooPtr );
	RTTI_REGISTER_METHOD( FooConstPtr );
	RTTI_REGISTER_METHOD( FooGet );
	RTTI_REGISTER_METHOD( FooSet );
	RTTI_REGISTER_METHOD( FooGetRef );
);

RTTI_IMPLEMENT_TYPE( rttiTest::CFooChild,
	RTTI_REGISTER_METHOD( FooFunc3 )
);

TEST( TestCaseName, MethodSignature )
{
	std::vector< const rtti::Type* > types;
	rtti::method_signature< decltype( &CFoo::FooFunc ) >::VisitArgumentTypes( [ & ]( const auto& type, rtti::InstanceFlags flags ) { types.emplace_back( &type ); return ::rtti::VisitOutcome::Continue; } );
	EXPECT_EQ( types[ 0 ], &rtti::GetTypeInstanceOf< Int32 >() );
	EXPECT_EQ( types[ 1 ], &rtti::GetTypeInstanceOf< Float >() );
	EXPECT_EQ( types[ 2 ], &rtti::GetTypeInstanceOf< AAA >() );
	EXPECT_EQ( types[ 3 ], &rtti::GetTypeInstanceOf< std::vector< Bool > >() );
	EXPECT_EQ( rtti::method_signature< decltype( &CFoo::FooFunc ) >::GetReturnTypeDesc(), nullptr );
	EXPECT_EQ( rtti::method_signature< decltype( &CFoo::FooFunc2 ) >::GetReturnTypeDesc(), &rtti::GetTypeInstanceOf< Int32 >() );
}

TEST( TestCaseName, TypeMethods )
{
	const auto& type = rttiTest::CFoo::GetTypeStatic();
	const auto* method = type.FindMethod( "FooFunc" );
	EXPECT_EQ( method->GetParametersAmount(), 4 );

	EXPECT_EQ( method->GetParametersAmount(), 4 );
	EXPECT_EQ( &method->GetParameterTypeDesc( 0 )->GetType(), &rtti::GetTypeInstanceOf< Int32 >() );
	EXPECT_EQ( &method->GetParameterTypeDesc( 1 )->GetType(), &rtti::GetTypeInstanceOf< Float >() );
	EXPECT_EQ( &method->GetParameterTypeDesc( 2 )->GetType(), &rtti::GetTypeInstanceOf< AAA >() );
	EXPECT_EQ( &method->GetParameterTypeDesc( 3 )->GetType(), &rtti::GetTypeInstanceOf< std::vector< Bool > >() );
	EXPECT_EQ( method->GetReturnTypeDesc(), nullptr );

	EXPECT_EQ( &type.FindMethod( "FooFunc2" )->GetReturnTypeDesc()->GetType(), &rtti::GetTypeInstanceOf< Int32 >());
	EXPECT_EQ( type.FindMethod( "FooFunc2" )->GetParametersAmount(), 0);

	EXPECT_TRUE( type.FindMethod( "FooRef" )->GetParameterTypeDesc( 0 )->HasFlags( ::rtti::InstanceFlags::Ref ) );
	EXPECT_TRUE( type.FindMethod( "FooConstRef" )->GetParameterTypeDesc( 0 )->HasFlags( ::rtti::InstanceFlags::Ref | ::rtti::InstanceFlags::Const ) );
	EXPECT_TRUE( type.FindMethod( "FooGetRef" )->GetReturnTypeDesc()->HasFlags( ::rtti::InstanceFlags::Ref ) );
}

TEST( TestCaseName, FindingMethods )
{
	const auto& type = rttiTest::CFoo::GetTypeStatic();
	EXPECT_EQ( type.FindMethod( "FooFunc" ), type.FindMethod( "FooFunc" ) );
	EXPECT_EQ( type.FindMethod( "FooFunc2" ), type.FindMethod( "FooFunc2" ) );
}

TEST( TestCaseName, CallingMethods )
{
	const auto& type = rttiTest::CFoo::GetTypeStatic();
	CFoo obj;

	{
		const auto* method = type.FindMethod( "FooSum" );
		struct
		{
			Int32 a = 50;
			Int32 b = 6;
		} params;

		Int32 retVal = 0;

		method->Call( &obj, &params, &retVal );
		EXPECT_EQ( retVal, 56 );
	}

	{
		const auto* method = type.FindMethod( "FooPtr" );
		Int32 input = 0;
		Int32* ptr = &input;

		method->Call( &obj, &ptr, nullptr);
		EXPECT_EQ( input, 123 );
	}

	{
		const auto* method = type.FindMethod( "FooConstPtr" );
		Int32 input = 6546456;
		Int32* ptr = &input;

		Int32 output = 0;

		method->Call( &obj, &ptr, &output );
		EXPECT_EQ( input, output );
	}

	{
		const auto* method = type.FindMethod( "FooSet" );
		Float input = 3.14f;

		obj.m_floatVar = 0.0f;
		method->Call( &obj, &input, nullptr );
		EXPECT_EQ( obj.m_floatVar, 3.14f );
	}

	{
		const auto* method = type.FindMethod( "FooGet" );
		obj.m_floatVar = 1.23f;
		Float result = 0.0f;
		method->Call( &obj, nullptr, &result );
		EXPECT_EQ( result, obj.m_floatVar );
	}

	{
		Int32 a = 0;
		const auto* method = type.FindMethod( "FooRef" );
		method->Call( &obj, &a, nullptr );
		EXPECT_EQ( a, 1 );

		method->Call( &obj, &a, nullptr );
		EXPECT_EQ( a, 2 );

		method->Call( &obj, &a, nullptr );
		EXPECT_EQ( a, 3 );

		method->Call( &obj, &a, nullptr );
		EXPECT_EQ( a, 4 );

		method->Call( &obj, &a, nullptr );
		EXPECT_EQ( a, 5 );
	}

	{
		const auto* method = type.FindMethod( "FooConstRef" );
		Int32 a = 321;
		Int32 result = 123;
		EXPECT_NE( a, result );
		method->Call( &obj, &a, &result );
		EXPECT_EQ( a, result );
	}

	{
		const auto* method = type.FindMethod( "FooGetRef" );
		Int32 result = 0;
		method->Call( &obj, nullptr, &result );
		EXPECT_EQ( 123, result );
	}

	{
		{
			CFooChild childObj;
			{
				const auto* method = ::rtti::GetTypeInstanceOf< rttiTest::CFooChild >().FindMethod( "FooFunc2" );
				Int32 result = 0;
				method->Call( &childObj, nullptr, &result );
				EXPECT_EQ( 321, result );
			}
			{
				const auto* method = ::rtti::GetTypeInstanceOf< rttiTest::CFoo >().FindMethod( "FooFunc2" );
				Int32 result = 0;
				method->Call( &childObj, nullptr, &result );
				EXPECT_EQ( 321, result );
			}

			EXPECT_EQ( ::rtti::GetTypeInstanceOf< rttiTest::CFoo >().FindMethod( "FooFunc3" ), nullptr );

			{
				const auto* method = ::rtti::GetTypeInstanceOf< rttiTest::CFooChild >().FindMethod( "FooFunc3" );
				Int32 result = 0;
				method->Call( &childObj, nullptr, &result );
				EXPECT_EQ( 666, result );
			}
		}
	}
}

TEST( TestCaseName, IgnoringConsts )
{
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< const Float >(), ::rtti::GetTypeInstanceOf< Float >() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< const rttiTest::StructWithProperties >(), ::rtti::GetTypeInstanceOf< rttiTest::StructWithProperties >() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< Float* >(), ::rtti::GetTypeInstanceOf< Float* const >() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< const std::shared_ptr< Float > >(), ::rtti::GetTypeInstanceOf< std::shared_ptr< Float > >() );
	EXPECT_EQ( ::rtti::GetTypeInstanceOf< const std::vector< Float > >(), ::rtti::GetTypeInstanceOf< std::vector< Float > >() );
	EXPECT_EQ( ( ::rtti::GetTypeInstanceOf< const std::pair< Float, Int32 > >() ), ( ::rtti::GetTypeInstanceOf< std::pair< Float, Int32 > >() ) );

	EXPECT_NE( ::rtti::GetTypeInstanceOf< const Float* >(), ::rtti::GetTypeInstanceOf< Float* >() );
	EXPECT_NE( ( ::rtti::GetTypeInstanceOf< std::shared_ptr< const Float > >() ), ( ::rtti::GetTypeInstanceOf< std::shared_ptr< Float > >() ) );
	EXPECT_NE( ( ::rtti::GetTypeInstanceOf< std::pair< Float, Int32 > >() ), ( ::rtti::GetTypeInstanceOf< std::pair< const Float, Int32 > >() ) );
}