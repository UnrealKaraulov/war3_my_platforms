/*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef MAKE_UNIQUE_H
#define MAKE_UNIQUE_H

#include "common/setup_before.h"

#ifndef HAVE_MAKE_UNIQUE

#include <memory>
#include <type_traits>
#include <utility>

namespace pvpgn
{
	//https://isocpp.org/files/papers/N3656.txt

	template<class T> struct _Unique_if {
		typedef std::unique_ptr<T> _Single_object;
	};

	template<class T> struct _Unique_if<T[]> {
		typedef std::unique_ptr<T[]> _Unknown_bound;
	};

	template<class T, size_t N> struct _Unique_if<T[N]> {
		typedef void _Known_bound;
	};

	template<class T, class... Args>
	typename _Unique_if<T>::_Single_object
		make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<class T>
	typename _Unique_if<T>::_Unknown_bound
		make_unique(size_t n) {
		typedef typename remove_extent<T>::type U;
		return std::unique_ptr<T>(new U[n]());
	}

	template<class T, class... Args>
	typename _Unique_if<T>::_Known_bound
		make_unique(Args&&...) = delete;
}
#else
#define make_unique std::make_unique
#endif //HAVE_MAKE_UNIQUE
#endif //MAKE_UNIQUE_H