#pragma once

namespace awl
{
	template<typename Lambda>
	class scope_guard
	{
	public:

		scope_guard(const Lambda & f, bool e = true) : free(f), engaged(e)
		{
		}

		scope_guard(const scope_guard &) = delete;

		scope_guard & operator = (const scope_guard &) = delete;

		scope_guard(scope_guard && other) : free(std::move(other.free))
		{
			other.engaged = false;
		}

		scope_guard & operator = (scope_guard && other)
		{
			free = std::move(other.free);
			other.engaged = false;
		}

		~scope_guard()
		{
			if (engaged)
			{
				free();
			}
		}

		void release()
		{
			engaged = false;
		}

	private:

		Lambda free;

		bool engaged = true;
	};

	template <class Lambda>
	inline scope_guard<Lambda> make_scope_guard(const Lambda & free, bool engaged = true)
	{
		return scope_guard<Lambda>(free, engaged);
	}

	template <class Init, class Free>
	inline scope_guard<Free> make_scope_guard(const Init & init, const Free & free, bool engaged = true)
	{
		if (engaged)
		{
			init();
		}

		return make_scope_guard(free, engaged);
	}
}
