#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <chrono>

#include <boost/signals2.hpp>

#include "lsignal.h"

using lsignal::connection;
using lsignal::signal;
using lsignal::slot;

void print_sum(int x, int y)
{
	std::cout << "sum(" << x << ", " << y << ") = " << (x + y) << "\n";
}

void print_mul(int x, int y)
{
	std::cout << "mul(" << x << ", " << y << ") = " << (x * y) << "\n";
}

int pow2(int x)
{
	return x * x;
}

int pow3(int x)
{
	return x * x * x;
}

int sum_agg(const std::vector<int>& v)
{
	return std::accumulate(v.cbegin(), v.cend(), 0);
}

void bar()
{
	std::cout << "function: bar\n";
}

struct baz
{
	void operator() ()
	{
		std::cout << "functor: baz\n";
	}
};

struct qux
{
	void print()
	{
		std::cout << "class member: qux\n";
	}
};

struct demo
	: public lsignal::slot
{
	int value;

	demo(int v) : value(v)
	{
	}
};

std::chrono::duration<double> get_performance(const std::function<void()>& fn)
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;

	start = std::chrono::steady_clock::now();

	fn();

	end = std::chrono::steady_clock::now();

	return end - start;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	// example 1
	std::cout << "example #1:\n";

	signal<void(int, int)> data;

	data.connect(print_sum);
	data.connect(print_mul);

	data(3, 4);

	// example 2
	std::cout << "\nexample #2:\n";

	signal<int(int)> worker;

	worker.connect(pow2);
	worker.connect(pow3);

	std::cout << "last slot = " << worker(2) << "\n";
	std::cout << "agg value = " << worker(2, sum_agg) << "\n";

	// example 3
	std::cout << "\nexample #3:\n";

	signal<void()> news;

	connection conn_one = news.connect([]() { std::cout << "news #1\n"; });
	connection conn_two = news.connect([]() { std::cout << "news #2\n"; });
	news.connect([]() { std::cout << "news #3\n"; });

	std::cout << "(all connections)\n";
	news();

	std::cout << "(lock connection one)\n";
	conn_one.set_lock(true);
	news();

	std::cout << "(disconnect connection two)\n";
	conn_two.disconnect();
	news();

	// example 4
	std::cout << "\nexample #4:\n";

	signal<void()> dummy;

	auto foo = []() { std::cout << "lambda: foo\n"; };

	dummy.connect(foo);
	dummy.connect(bar);

	baz b;
	qux q;

	dummy.connect(b);
	dummy.connect(&q, &qux::print);

	dummy();

	// example 5
	std::cout << "\nexample #5:\n";

	signal<void()> printer;

	{
		demo dm(42);

		auto print_value = [&dm]() { std::cout << "value = " << dm.value << "\n"; };

		printer.connect(std::move(print_value), &dm);

		printer();
	}

	printer();

	// example 6
	std::cout << "\nexample #6:\n";

	signal<void(int)> first;
	signal<void(int)> second;

	first.connect(&second);

	second.connect([](int x)
	{
		std::cout << "x = " << x << "\n";
	});

	first(10);

	// check performance
	lsignal::signal<void()> ls;
	boost::signals2::signal_type<void(), boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>>::type bs;

	ls.connect([](){});
	bs.connect([](){});

	auto lsignal_func = [&ls]() { ls(); };
	auto bsignal_func = [&bs]() { bs(); };

	std::chrono::nanoseconds elapsed;

	// lsignal performance
	std::cout << "\nlsignal performance:\n";

	elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(get_performance(lsignal_func));

	std::cout << elapsed.count() << " ns\n";

	// boost signal2 perfromance (with dummy mutex)
	std::cout << "\nboost::signals2 performance:\n";

	elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(get_performance(bsignal_func));

	std::cout << elapsed.count() << " ns\n";

	return 0;
}
