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

		~ScopeGuard()
		{
			lambda();
		}

	private:

		Lambda & lambda;
	};
}
