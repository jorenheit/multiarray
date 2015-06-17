/*Author: Joren Heit (jorenheit@gmail.com)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MULTIARRAY_H
#define MULTIARRAY_H

#include <vector>    // default implementation
#include <algorithm> // std::fill

template <typename T, std::size_t D, template <typename...> class Container = std::vector>
class MultiArray
{
    // Container must provide: 
    // * constructor taking a size-variable: Container<T>(size_t)
    // * begin()
    // * end()
    // * operator[]

    Container<T> d_data;
    std::size_t d_dimensions[D];

public:
    // Constructor
    template <typename ... Args>
    MultiArray(Args ... args):
        d_data(product(args...)),
        d_dimensions{static_cast<std::size_t>(args)...}
    {
        static_assert(sizeof ... (args) == D, "Number of dimensions does not match number of constructor-args");
    }

    // Index operators
    template <typename ... Args>
    T &operator()(Args ... args)
    {
        return d_data[indexMap(static_cast<std::size_t>(args) ...)];
    }

    template <typename ... Args>
    T const &operator()(Args ... args) const
    {
        return d_data[indexMap(static_cast<std::size_t>(args) ...)];
    }

    // size information
    constexpr std::size_t dim() const
    {
        return D;
    }

    std::size_t size(std::size_t dim) const
    {
        return d_dimensions[dim];
    }

    // Range assignment
    MultiArray &assign(size_t const (&ranges)[D][2], T const &val)
    {
        Assign<0>::assign(*this, ranges, val);
        return *this;
    }

    MultiArray &assign(T const &val)
    {
        std::fill(d_data.begin(), d_data.end(), val);
        return *this;
    }

    // raw access
    Container<T> &data()
    {
        return d_data;
    }

    Container<T> const &data() const
    {
        return d_data;
    }

    template <typename ... Args>
    std::size_t index(Args ... args) const
    {
        return indexMap(static_cast<size_t>(args) ...);
    }

private:
    // Helper functions
    template <typename ... Tail>
    static std::size_t product(std::size_t head, Tail ... tail)
    {
        return head * (sizeof ... (Tail) == 0 ? 1 : product(tail ...));
    }

    template <typename ... Tail>
    std::size_t indexMap(std::size_t head, Tail ... tail) const
    {
        enum { count = sizeof ... (tail) };

        std::size_t fac = 1;
        for (std::size_t idx = D - count; idx != D; ++idx)
            fac *= d_dimensions[idx];

        return head * fac + ((count == 0) ? 0 : indexMap(tail ...));
    }

    // Range assignment
    template <size_t Level, typename Dummy = void>
    struct Assign
    {
        template <typename Self, typename ... Indices>
        static void assign(Self &self, size_t const (&ranges)[D][2], T const &val, Indices ... indices)
        {
            for (size_t i = ranges[Level][0]; i != ranges[Level][1]; ++i)
                Assign<Level + 1>::assign(self, ranges, val, indices..., i);
        }
    };

    template <typename Dummy>
    struct Assign<D - 1, Dummy>
    {
        template <typename Self, typename ... Indices>
        static void assign(Self &self, size_t const (&ranges)[D][2], T const &val, Indices ... indices)
        {
            size_t const begin = self.indexMap(indices ..., ranges[D - 1][0]);
            size_t const end = begin + (ranges[D - 1][1] - ranges[D - 1][0]);
            std::fill(&self.d_data[begin], &self.d_data[end], val);
        }
    };

    // Only declared, not defined (to allow the recursion to compile)
    static std::size_t product();
    std::size_t indexMap() const;
};

// convenient factory
template <typename T, template <typename ...> class C = std::vector, typename ... Args>
inline MultiArray<T, sizeof ... (Args), C> makeMultiArray(Args ... args)
{
    return MultiArray<T, sizeof ... (args), C>(args ...);
}

#endif
