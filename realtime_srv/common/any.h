#pragma once

namespace realtime_srv
{

////自定义的any类annotated
//class any
//{
//public:
//
//	//保存真正数据的接口类
//	class placeholder
//	{
//	public:
//		virtual ~placeholder()
//		{}
//	public:
//
//		virtual const std::type_info & type() const = 0;
//		virtual placeholder * clone() const = 0;
//	};
//
//	//真正保存和获取数据的类。
//	template<typename ValueType>
//	class holder : public placeholder
//	{
//	public:
//		holder(const ValueType & value) : held(value)
//		{}
//
//	public:
//
//		virtual const std::type_info & type() const
//		{
//			return typeid(ValueType);
//		}
//
//		virtual placeholder * clone() const
//		{
//			return new holder(held);//使用了原型模式
//		}
//
//	public:
//
//		//真正的数据，就保存在这里
//		ValueType held;
//	};
//
//public:
//
//	any() : content(NULL)
//	{}
//
//	//模板构造函数，参数可以是任意类型，真正的数据保存在content中
//	template<typename ValueType>
//	any(const ValueType & value) : content(new holder<ValueType>(value))
//	{}
//
//	//拷贝构造函数
//	any(const any & other)
//		: content(other.content ? other.content->clone() : 0)
//	{}
//
//	//析构函数，删除保存数据的content对象
//	~any()
//	{
//		if (NULL != content)
//			delete content;
//	}
//
//private:
//	//一个placeholde对象指针，指向其子类folder的一个实现
//	// 即content( new holder<ValueType>(value) )语句
//	placeholder* content;
//
//	template<typename ValueType> friend ValueType any_cast(const any& operand);
//public:
//
//	//查询真实数据的类型。
//	const std::type_info & type() const
//	{
//		return content ? content->type() : typeid(void);
//	}
//};
//
//
////获取content->helder数据的方法。用来获取真正的数据
//template<typename ValueType>
//ValueType any_cast(const any& operand)
//{
//	assert(operand.type() == typeid(ValueType));
//	return static_cast<any::holder<ValueType> *>(operand.content)->held;
//}

class any
{
public: // structors

	any()
		: content(0)
	{}

	template<typename ValueType>
	any(const ValueType & value)
		: content(new holder<ValueType>(value))
	{}

	any(const any & other)
		: content(other.content ? other.content->clone() : 0)
	{}

	~any()
	{
		delete content;
	}

public: // modifiers

	any & swap(any & rhs)
	{
		std::swap(content, rhs.content);
		return *this;
	}

	template<typename ValueType>
	any & operator=(const ValueType & rhs)
	{
		any(rhs).swap(*this);
		return *this;
	}

	any & operator=(any rhs)
	{
		rhs.swap(*this);
		return *this;
	}

public: // queries

	bool empty() const
	{
		return !content;
	}

	const std::type_info & type() const
	{
		return content ? content->type() : typeid(void);
	}

public: // types (public so any_cast can be non-friend)

	class placeholder
	{
	public: // structors

		virtual ~placeholder()
		{}

	public: // queries

		virtual const std::type_info & type() const = 0;

		virtual placeholder * clone() const = 0;

	};

	template<typename ValueType>
	class holder : public placeholder
	{
	public: // structors

		holder(const ValueType & value)
			: held(value)
		{}

	public: // queries

		virtual const std::type_info & type() const
		{
			return typeid(ValueType);
		}

		virtual placeholder * clone() const
		{
			return new holder(held);
		}

	public: // representation

		ValueType held;

	private: // intentionally left unimplemented
		holder& operator=(const holder &);
	};

public: // representation (public so any_cast can be non-friend)

	placeholder * content;

};

template<typename ValueType>
ValueType any_cast(const any& operand)
{
	assert(operand.type() == typeid(ValueType));
	return static_cast<any::holder<ValueType> *>(operand.content)->held;
}


}