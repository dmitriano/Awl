#pragma once

namespace awl
{
	template<typename Lambda>
	class scope_guard
	{
	public:

		scope_guard(const Lambda & l) : lambda(l)
		{
		}

		scope_guard(const scope_guard &) = delete;

		scope_guard & operator = (const scope_guard &) = delete;

		scope_guard(scope_guard && other) : lambda(std::move(other.lambda))
		{
			other.engaged = false;
		}

		scope_guard & operator = (scope_guard && other)
		{
			lambda = std::move(other.lambda);
			other.engaged = false;
		}

		~scope_guard()
		{
			if (engaged)
			{
				lambda();
			}
		}

		void release()
		{
			engaged = false;
		}

	private:

		Lambda lambda;

		bool engaged = true;
	};

	template <class Lambda>
	inline scope_guard<Lambda> make_scope_guard(const Lambda & l)
	{
		return scope_guard<Lambda>(l);
	}
}
