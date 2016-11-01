#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <memory>
#include <random>
#include <vector>
#include <chrono>

struct deleter
{
    void operator()(void* ptr) const
    {
        free(ptr);
    }
};

using raw_ptr = std::unique_ptr<void, deleter>;

size_t round_to_power_of_two(size_t n)
{
    size_t r = 1;
    while (r < n)
        r <<= 1;
    return r;
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3 && argc != 2)
        {
            std::cerr << argv[0] << " <#slots> <#allocations>\n";
            return EXIT_FAILURE;
        }

        size_t number_of_slots_in = std::stoi(argv[1]);
        size_t number_of_slots = number_of_slots_in;
        if (number_of_slots > 1e9)
            number_of_slots = 1e9;
        number_of_slots = round_to_power_of_two(number_of_slots);
        if (number_of_slots != number_of_slots_in)
            std::cout << "adjusting number of slots " << number_of_slots_in << " -> " << number_of_slots << std::endl;
        
        size_t number_of_allocs;
        if (argc == 3)
            number_of_allocs = std::stoi(argv[2]);
        else
            number_of_allocs = std::max(number_of_slots, static_cast<size_t>(16 * 1024)) * 50;

        std::minstd_rand engine;
        
        size_t max_alloc_size = 128;
        using clock = std::chrono::high_resolution_clock;
        clock::time_point start = clock::now();
        {
            std::vector<raw_ptr> v(number_of_slots);
            for (size_t i = 0; i != number_of_allocs; ++i)
                v[engine() & (number_of_slots - 1)].reset(malloc((engine() & (max_alloc_size - 1)) + 1));
        }
        clock::time_point end = clock::now();
        uint64_t allocs_per_second = static_cast<uint64_t>(number_of_allocs) * clock::duration::period::den / clock::duration::period::num / (end - start).count();
        std::cout << allocs_per_second / 1000 << " K allocations per second\n";
        std::cout << allocs_per_second * (max_alloc_size / 2) / 1000000 << " MB per second\n";
    }
    catch (std::exception const& e)
    {
        std::cerr << "error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "unknown error: catch (...)\n";
        return EXIT_FAILURE;
    }
}
