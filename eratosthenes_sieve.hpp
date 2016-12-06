#ifndef _ERATOSTHENES_SIEVE_HPP
#define _ERATOSTHENES_SIEVE_HPP

#include <vector>
#include <algorithm>

namespace eratosthenes_sieve
{
    // Description: https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
    // Returns prime numbers from 2 to @n
    std::vector<unsigned int> primes(unsigned int n)
    {
        std::vector<unsigned int> result;
        // Create a vector of consecutive integers from 2 through n: (2, 3, 4, ..., n)
        for( unsigned int number = 2; number <= n; ++number )
            result.push_back(number);

        auto smallest_prime = result.begin();

        while( (*smallest_prime) * (*smallest_prime) <= n )
        {
            unsigned int i = (*smallest_prime);
            while( i * (*smallest_prime) <= n )
            {
                unsigned int number = i * (*smallest_prime);
                auto it = std::find(result.begin(), result.end(), number);
                if( it != result.end() )
                    result.erase(it);
                i++;
            }

            ++smallest_prime;
        }

        return result;
    }
}

#endif // _ERATOSTHENES_SIEVE_HPP