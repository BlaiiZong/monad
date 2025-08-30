#include "monad.h"
#include <iostream>
using M = Monad<int, const char*>;

// Don't know how to get it working with the commented out function notation
// M add1(M& m) {
//     return m.get_value() + 1;
// }
const std::function<M(const int&)> add1 = [](const int& m) { return 1 + m; };

// M times3(M& m) {
//     return m.get_value() * 3;
// }
const std::function<M(const int&)> times3 = [](const int& m) { return 3*m; };

// M div0(M& m) {
//     return "division by zero error!";
// }
const std::function<M(const int&)> div0 = [](const int& m) { return "division by zero error!"; };

int main() {
	M my_monad = 1;
	std::cout << (my_monad >> add1 >> times3 >> times3) << "\n";
	my_monad >>= add1 >>= div0 >>= times3;
	std::cout << my_monad << "\n";

	// This will output false
	M default_monad;
	std::cout << std::boolalpha << default_monad.has_value() << "\n";

	// Monad<T, T> can only construct value_, not error_;
	// use set_error() to set error_;
	Monad<int, int> specialised = 1;
	std::cout << specialised.has_value() << "\n";
}
