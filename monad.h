#include <iostream>

// Monad class storing a value of type T, or an error of type E
template<typename T, typename E>
class Monad {
	static constexpr bool different_types = !std::is_same_v<T, E>;
	union Data {
		T value_;
		E error_;

		// Default constructor initialises Data as the error type
		Data()
		: error_(E{}) {}
		Data(T value)
		: value_(value) {}

		// Data(E error) : error_(error) {}
		template <bool B = different_types>
		Data(E);
		template<>
		Data<false>(E) = delete;
		template<>
		Data<true>(E error)
		: error_(error) {}

		~Data() {
			value_.~T();
			error_.~E();
		}
	};
	using M_default = Monad<T, E>;
	using M_function = std::function<M_default(const M_default&)>;

public:
	///////////////////////////////////////// Constructors /////////////////////////////////////////

	// Monads are default constructed as the error type.
	// This is to be consistent with std::optional.
	Monad()
	: has_value_(false) {}

	Monad(T&& value)
	: has_value_(true)
	, data_(value) {}

	template <bool B = different_types>
	Monad(E&&);
	// Deleted definition must be first declaration :o
	template<>
	Monad<false>(E&&) = delete;
	template<>
	Monad<true>(E&& error) : has_value_(false), data_(error) {}

	// Copy constructor/assignment
	Monad(const M_default& other)
	: has_value_(other.has_value_) {
		if (has_value_)
			data_.value_ = other.data_.value_;
		else
			data_.error_ = other.data_.error_;
	}
	auto operator=(const M_default& other) -> M_default& {
		*this = M_default{other};
		return *this;
	}

	// Move constructor/assignment
	Monad(M_default&& other) noexcept = default;
	auto operator=(M_default&& other) -> M_default& = default;

	~Monad() noexcept = default;

	/////////////////////////////////////////// Accessors //////////////////////////////////////////

	auto has_value() const -> const bool {
		return has_value_;
	}

	// TODO: decide what happens when the monad is in its error form
	auto get_value() const -> const T& {
		return data_.value_;
	}

	/////////////////////////////////////////// Modifiers //////////////////////////////////////////
	// Unlikely ever needed, but may prove useful in the case of Monad<T, T>'s

	auto set_value(T& value) -> void {
		has_value_ = true;
		data_.value_ = value;
	}

	auto set_error(E& error) -> void {
		has_value_ = false;
		data_.error_ = error;
	}

	///////////////////////////////////// Function Application /////////////////////////////////////

	// How do we specialise for functions?
	// In Haskell, we do:
	// ```
	// monad >>= f1 >>= f2 >>= f3
	// ```
	// to apply functions f1, f2, f3 in order to the monad.
	// Here, we can do the same thing, except by default the `>>=` operator in C++ is
	// right-associative. This means that the expression `monad >>= f1 >>= f2` gets interpreted as
	// `monad >>= (f1 >>= f2)`. To get around this, we simply define the >>= operator on functions f1
	// and f2 in a left-associative manner, i.e., (f1 >>= f2 >>= f3)(m) = f3(f2(f1))(m) =
	// f3(f2(f1(m))). Luckily, the same problem doesn't happen with the `>>` operator (which is for
	// some reason left-associative).
	friend auto operator>>=(const M_function& f1, const M_function& f2) -> const M_function {
		return [f1, f2](const M_default& m) { return m >> f1 >> f2; };
	}

	friend auto operator>>=(const M_function& f1, const M_function&& f2) -> const M_function {
		return f1 >>= f2;
	}

	friend auto operator>>(const M_default& monad, const M_function& f) -> M_default {
		if (!monad.has_value())
			return monad;

		return f(monad);
	}

	friend auto operator>>(M_default&& monad, const M_function& f) -> M_default {
		return monad >> f;
	}

	auto operator>>=(const M_function& f) -> M_default& {
		return *this = *this >> f;
	}

	auto operator>>=(const M_function&& f) -> M_default& {
		return *this >>= f;
	}

	////////////////////////////////////// Operator Overloads //////////////////////////////////////

	// This works really nicely with `M_function`s
	operator T() const {
		return get_value();
	}

	friend auto operator<<(std::ostream& os, const M_default& monad) -> std::ostream& {
		if (monad.has_value())
			os << monad.get_value();
		else
			os << monad.data_.error_;
		return os;
	}
	
	friend auto operator==(const M_default& monad, T value) -> bool {
		return monad.has_value() && monad.get_value() == value;
	}

	friend auto operator==(T value, const M_default& monad) -> bool {
		return monad == value;
	}

private:
	bool has_value_;
	Data data_;
};

// Equality of Monads
// Arbitrary: Let's say that two monads are equal if they store the same value, or they are both
// errors - doesn't even have to be the same error type
template<typename T, typename E1, typename E2>
auto operator==(const Monad<T, E1>& lhs, const Monad<T, E2>& rhs) -> bool {
	return (!lhs.has_value() && !rhs.has_value()) || lhs.get_value() == rhs.get_value();
}
