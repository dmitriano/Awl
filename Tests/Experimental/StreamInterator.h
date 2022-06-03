/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/SequentialStream.h" 
#include "Awl/Io/ReadWrite.h"

#include <iterator>

namespace awl::io
{
	namespace detail
	{
		template <class T>
		class OutputElement
		{
		public:

			OutputElement(SequentialOutputStream& out) : m_out(out) {}

			OutputElement& operator = (T&& val)
			{
				Write(*p_out, std::forward<T>(val), *p_context);
			}

		private:

			SequentialOutputStream& m_out;
		};
	}
	
	template <class T, class Context>
	class output_iterator
	{
	public:

		using iterator_category = std::output_iterator_tag;

		//A value is not T but T*, because the list is a contaner of elements of type T *.
		using value_type = T;

		//Required by std::iterator_traits in GCC.
		using difference_type = std::ptrdiff_t;

		using pointer = value_type*;

		using reference = value_type&;

		output_iterator() : output_iterator(nullptr) {}
		
		output_iterator(SequentialOutputStream& out, Context& ctx) : p_out(&out), p_context(&ctx) {}

	private:

		SequentialOutputStream* p_out;
		Context* p_context;
	};
}
