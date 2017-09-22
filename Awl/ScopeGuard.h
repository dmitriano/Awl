#pragma once

namespace awl
{
	template<typename Lambda>
	class ScopeGuard
	{
	public:

		ScopeGuard(Lambda & l) : lambda(l)
		{
		}

		ScopeGuard(const ScopeGuard &) = delete;

		ScopeGuard & operator = (ScopeGuard &) = delete;

		ScopeGuard(const ScopeGuard && other)
		{
		}

		ScopeGuard & operator = (ScopeGuard && other)
		{
			lambda = std::move(other.lambda);
		}

		~ScopeGuard()
		{
			lambda();
		}

	private:

		Lambda & lambda;
	};

	template <class Lambda>
	inline ScopeGuard<Lambda> make_scoped_guard(Lambda & l)
	{
		return ScopeGuard<Lambda>(l);
	}
}
