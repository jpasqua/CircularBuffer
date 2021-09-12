/*
 CircularBuffer.h - Circular buffer library for Arduino.
 Copyright (c) 2017 Roberto Lo Giacco.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as 
 published by the Free Software Foundation, either version 3 of the 
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_
#include <stdint.h>
#include <stddef.h>

#ifdef CIRCULAR_BUFFER_DEBUG
#include <Print.h>
#endif

namespace Helper {
	template<bool FITS8, bool FITS16> struct Index {
		using Type = uint32_t;
	};

	template<> struct Index<false, true> {
		using Type = uint16_t;
	};

	template<> struct Index<true, true> {
		using Type = uint8_t;
	};
}


/**
 * For the push() and unshift() functions, you may choose whether to pass-by-value, pass-by-reference,
 * or have the template choose at compile time base on the element type. This is the default and in this case
 * we pass-by-value for fundamental types and pass-by-reference otherwise. To always use pass-by-value, which
 * is what the original library did, #define ARG_TYPE_VAL *before* including CircularBuffer.h. To always
 * use pass-by-reference #define ARG_TYPE_REF. Don't define either to get the default behavior.
 */
#include <type_traits>
template<typename T>
#if defined(ARG_TYPE_REF)
	using choose_arg_type = typename std::conditional<false, T, const T &>::type;
#elif defined(ARG_TYPE_VAL)
	using choose_arg_type = typename std::conditional<true, T, const T &>::type;
#else
	using choose_arg_type = typename std::conditional<std::is_fundamental<T>::value, T, const T &>::type;
#endif


template<typename T, size_t S, typename IT = typename Helper::Index<(S <= UINT8_MAX), (S <= UINT16_MAX)>::Type> class CircularBuffer {
public:
	/**
	 * The buffer capacity: read only as it cannot ever change.
	 */
	static constexpr IT capacity = static_cast<IT>(S);

	/**
	 * Aliases the index type, can be used to obtain the right index type with `decltype(buffer)::index_t`.
	 */
	using index_t = IT;

	constexpr CircularBuffer();

	/**
	 * Disables copy constructor
	 */
	CircularBuffer(const CircularBuffer&) = delete;
	CircularBuffer(CircularBuffer&&) = delete;

	/**
	 * Disables assignment operator
	 */
	CircularBuffer& operator=(const CircularBuffer&) = delete;
	CircularBuffer& operator=(CircularBuffer&&) = delete;

	/**
	 * Adds an element to the beginning of buffer: the operation returns `false` if the addition caused overwriting an existing element.
	 */
	bool unshift(choose_arg_type<T> value);

	/**
	 * Adds an element to the end of buffer: the operation returns `false` if the addition caused overwriting an existing element.
	 */
	bool push(choose_arg_type<T> value);

	/**
	 * Removes an element from the beginning of the buffer.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
	T shift();

	/**
	 * Removes an element from the end of the buffer.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
	T pop();

	/**
	 * Returns the element at the beginning of the buffer.
	 */
	T inline first() const;

	/**
	 * Returns the element at the end of the buffer.
	 */
	T inline last() const;

	/**
	 * Array-like access to buffer.
	 * Calling this operation using and index value greater than `size - 1` returns the tail element.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
	T operator [] (IT index) const;

	/**
	 * Similar to operator [], but returns a const ref to the stored element rather than a copy.
	 * Similar to std::vector::at, but always returns a const reference. 
	 * Calling this operation using and index value greater than `size - 1` returns the tail element.
	 * *WARNING* Calling this operation on an empty buffer has an unpredictable behaviour.
	 */
	const T& peekAt(IT index) const;

	/**
	 * Returns how many elements are actually stored in the buffer.
	 */
	IT inline size() const;

	/**
	 * Returns how many elements can be safely pushed into the buffer.
	 */
	IT inline available() const;

	/**
	 * Returns `true` if no elements can be removed from the buffer.
	 */
	bool inline isEmpty() const;

	/**
	 * Returns `true` if no elements can be added to the buffer without overwriting existing elements.
	 */
	bool inline isFull() const;

	/**
	 * Resets the buffer to a clean status, making all buffer positions available.
	 */
	void inline clear();

	#ifdef CIRCULAR_BUFFER_DEBUG
	void inline debug(Print* out);
	void inline debugFn(Print* out, void (*printFunction)(Print*, T));
	#endif

private:
	T buffer[S];
	T *head;
	T *tail;
#ifndef CIRCULAR_BUFFER_INT_SAFE
	IT count;
#else
	volatile IT count;
#endif
};

#include <CircularBuffer.tpp>
#endif
