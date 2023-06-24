#pragma once

#include "../LibeRTTI.h"

namespace rttiTest
{
	class A
	{
		RTTI_DECLARE_POLYMORPHIC_CLASS( A )
	};

	static_assert( sizeof( A ) == 8u );

	class AA : public A
	{
		RTTI_DECLARE_POLYMORPHIC_CLASS( AA, A )
	};

	static_assert( sizeof( AA ) == 8u );

	class AAA : public AA
	{
		RTTI_DECLARE_POLYMORPHIC_CLASS( AAA, AA )
	};

	static_assert( sizeof( AAA ) == 8u );

	class B
	{
		RTTI_DECLARE_CLASS( B )
	};

	static_assert( sizeof( B ) == 1u );

	class BB : public B
	{
		RTTI_DECLARE_CLASS( BB, B )
	};

	static_assert( sizeof( BB ) == 1u );

	class BBB : public BB
	{
		RTTI_DECLARE_CLASS( BBB, BB )
	};

	static_assert( sizeof( BBB ) == 1u );

	namespace c
	{
		class C
		{
			RTTI_DECLARE_POLYMORPHIC_CLASS( C );
		};
	}

	namespace cc
	{
		class CC : public c::C
		{
			RTTI_DECLARE_POLYMORPHIC_CLASS( CC, c::C );
		};
	}

	namespace abstract
	{
		class Abstract
		{
			RTTI_DECLARE_ABSTRACT_CLASS( Abstract )
		public:
			virtual void Foo() = 0;
			virtual ~Abstract() {}
		};
	}

	namespace abc
	{
		class AbstractInheritsFromAbstract : public abstract::Abstract
		{
			RTTI_DECLARE_ABSTRACT_CLASS( AbstractInheritsFromAbstract, abstract::Abstract );
		};

		class InheritsFromAbstract : public AbstractInheritsFromAbstract
		{
			RTTI_DECLARE_POLYMORPHIC_CLASS( InheritsFromAbstract, abc::AbstractInheritsFromAbstract );
		public:
			virtual void Foo() override {}
		};
	}
}