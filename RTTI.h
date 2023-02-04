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
		IType()
		{
			RTTI::GetMutable().RegisterType( *this );
		}

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
			return reinterpret_cast< Uint64 >(this);
		}
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

#define CREATE_DEFAULT_INTERNAL_true( ClassName ) return nullptr;
#define CREATE_DEFAULT_INTERNAL_false( ClassName ) return new ClassName##();

#define DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, Inherits, Virtual, Abstract ) \
public: \
class Type : public ParentClassName##::##TYPE_CLASS_NAME_##Inherits## \
{ \
public: \
	virtual const char* GetName() const override; \
	virtual Bool InheritsFrom( const rtti::IType& type ) const override \
	{ \
		INHERITS_FROM_BODY_##Inherits##( ParentClassName ) \
	} \
	template< class T > \
	Bool InheritsFrom() const \
	{ \
		return rtti::IType::InheritsFrom< T >(); \
	} \
	std::unique_ptr< ClassName > CreateDefault() const \
	{ \
		return std::unique_ptr< ClassName >( CreateDefault_Internal() ); \
	} \
	virtual Bool IsAbstract() const override \
	{ \
		return Abstract; \
	} \
	virtual Bool IsVirtual() const override \
	{ \
		return Virtual; \
	} \
protected: \
	VIRTUAL_##Virtual ClassName##* CreateDefault_Internal() const \
	{ \
		CREATE_DEFAULT_INTERNAL_##Abstract##( ClassName ) \
	} \
}; \
	static const Type& GetTypeStatic() \
	{ \
		return s_typeInstance; \
	} \
	template< class T > \
	Bool IsA() const \
	{ \
		return static_cast<const rtti::IType&>( GetType() ).IsA< T >(); \
	} \
	template< class T > \
	Bool InheritsFrom() const \
	{ \
		return static_cast<const rtti::IType&>( GetType() ).InheritsFrom< T >(); \
	} \
	template< class T > \
	Bool InheritsFromOrIsA() const \
	{ \
		return IsA< T >() || InheritsFrom< T >(); \
	} \
	VIRTUAL_##Virtual const Type& GetType() const \
	{ \
		return s_typeInstance; \
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
	static Type s_typeInstance;

#define DECLARE_TYPE_INTERNAL( ClassName, Virtual ) \
DECLARE_TYPE_INTERNAL_PARENT( ClassName, rtti, false, Virtual, false)

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
DECLARE_TYPE_INTERNAL_PARENT( ClassName, rtti, false, true, true)

#define DECLARE_ABSTRACT_TYPE_INTERNAL_PARENT_DIRECT( ClassName, ParentClassName ) \
DECLARE_TYPE_INTERNAL_PARENT( ClassName, ParentClassName, true, true, true) \
	using Super = ParentClassName;

#define DECLARE_ABSTRACT_CLASS( ... ) EXPAND( GET_DECLARE_TYPE_MACRO( __VA_ARGS__, DECLARE_ABSTRACT_TYPE_INTERNAL_PARENT_DIRECT, DECLARE_ABSTRACT_TYPE_INTERNAL )( __VA_ARGS__ ) )

#define IMPLEMENT_TYPE( NamespaceClassName ) NamespaceClassName##::Type NamespaceClassName##::s_typeInstance; \
const char* NamespaceClassName##::Type::GetName() const \
{ \
	return #NamespaceClassName; \
}