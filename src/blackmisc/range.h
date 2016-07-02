/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_RANGE_H
#define BLACKMISC_RANGE_H

#include "blackmiscexport.h"
#include "iterator.h"
#include "predicates.h"
#include "algorithm.h"
#include <QtGlobal>
#include <QDebug>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <initializer_list>

namespace BlackMisc
{
    template <class> class CRange;

    /*!
     * Own implementation of C++17 std::as_const. Adds const to any lvalue.
     * Useful with non-mutating range-for loops to avoid unnecessary detachment of Qt implicitly shared containers.
     * Does not allow rvalues, as that could easily lead to undefined behaviour.
     */
    //! @{
    template <class T>
    constexpr std::add_const_t<T> &as_const(T &v) noexcept { return v; }
    template <class T>
    void as_const(const T &&) = delete;
    //! @}

    /*!
     * Any container class with begin and end iterators can inherit from this CRTP class
     * to gain some useful algorithms as member functions.
     * \tparam Derived The most derived container class inheriting from this instantiation.
     * \tparam CIt the const_iterator of Derived.
     */
    template <class Derived, class CIt>
    class CRangeBase
    {
        using value_type = typename std::iterator_traits<CIt>::value_type;
        using const_reference = typename std::iterator_traits<CIt>::reference;

    public:
        //! Return a new container generated by applying some transformation function to all elements of this one.
        template <class F>
        inline auto transform(F function) const
            -> CRange<Iterators::TransformIterator<CIt, F>>;

        //! Return a copy containing only those elements for which a given predicate returns true.
        template <class Predicate>
        inline auto findBy(Predicate p) const
            -> CRange<Iterators::ConditionalIterator<CIt, Predicate>>;

        //! Return a copy containing only those elements matching some particular key/value pair(s).
        //! \param k0 A pointer to a member function of T.
        //! \param v0 A value to compare against the value returned by k0.
        //! \param keysValues Zero or more additional pairs of { pointer to member function of T, value to compare it against }.
        template <class K0, class V0, class... KeysValues>
        inline auto findBy(K0 k0, V0 v0, KeysValues... keysValues) const
            -> CRange<Iterators::ConditionalIterator<CIt, decltype(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...))>>;

        //! Return a reference to the first element for which a given predicate returns true. Undefined if there is none.
        template <class Predicate>
        const_reference findFirstBy(Predicate p) const { return findBy(p).front(); }

        //! Return a reference to the first element matching some particular key/value pair(s). Undefined if there is none.
        template <class K, class V>
        const_reference findFirstBy(K key, V value) const { return findBy(key, value).front(); }

        //! Return a copy of the first element for which a given predicate returns true, or a default value if there is none.
        template <class Predicate>
        value_type findFirstByOrDefault(Predicate p, const value_type &def = value_type{}) const { return findBy(p).frontOrDefault(def); }

        //! Return a copy of the first element matching some particular key/value pair(s), or a default value if there is none.
        template <class K, class V>
        value_type findFirstByOrDefault(K key, V value, const value_type &def = value_type{}) const { return findBy(key, value).frontOrDefault(def); }

        //! Return true if there is an element for which a given predicate returns true.
        template <class Predicate>
        bool containsBy(Predicate p) const
        {
            return std::any_of(derived().cbegin(), derived().cend(), p);
        }

        //! Return true if there is an element equal to given object. Uses the most efficient implementation available in the derived container.
        template <class T>
        bool contains(const T &object) const
        {
            return derived().find(object) != derived().cend();
        }

        //! Return a copy containing only those elements matching some particular key/value pair(s).
        //! \param k0 A pointer to a member function of T.
        //! \param v0 A value to compare against the value returned by k0.
        //! \param keysValues Zero or more additional pairs of { pointer to member function of T, value to compare it against }.
        template <class K0, class V0, class... KeysValues>
        bool contains(K0 k0, V0 v0, KeysValues... keysValues) const
        {
            return containsBy(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...));
        }

        //! Copy n elements from the container at random.
        Derived randomElements(int n) const
        {
            Derived result;
            BlackMisc::copyRandomElements(derived().begin(), derived().end(), std::inserter(result, result.end()), n);
            return result;
        }

        //! Copy n elements from the container, randomly selected but evenly distributed.
        Derived sampleElements(int n) const
        {
            Derived result;
            BlackMisc::copySampleElements(derived().begin(), derived().end(), std::inserter(result, result.end()), n);
            return result;
        }

    private:
        Derived &derived() { return static_cast<Derived &>(*this); }
        const Derived &derived() const { return static_cast<const Derived &>(*this); }
    };

    /*!
     * A range is a conceptual container which does not contain any elements of its own,
     * but is constructed from a begin iterator and an end iterator of another container.
     *
     * By using iterator wrappers, it is possible to use CRange to iterate over the results of predicate methods without copying elements.
     *
     * \warning Remember that the iterators in the range refer to elements in the original container,
     *          so take care that the original container remains valid and does not invalidate its iterators
     *          during the lifetime of the range.
     */
    template <class I>
    class CRange : public CRangeBase<CRange<I>, I>
    {
    public:
        //! STL compatibility
        //! @{
        typedef typename std::iterator_traits<I>::value_type key_type;
        typedef typename std::iterator_traits<I>::value_type value_type;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef I const_iterator;
        typedef I iterator;
        typedef typename std::iterator_traits<I>::difference_type difference_type;
        //! @}

        //! Constructor.
        CRange(I begin, I end) : m_begin(begin), m_end(end) { check(&begin, &end); }

        //! Begin and end iterators.
        //! @{
        const_iterator begin() const { return m_begin; }
        const_iterator cbegin() const { return m_begin; }
        const_iterator end() const { return m_end; }
        const_iterator cend() const { return m_end; }
        //! @}

        //! Implicit conversion to any container of value_type which supports push_back. This will copy elements.
        template <class T, class = std::enable_if_t<std::is_convertible<value_type, typename T::value_type>::value>>
        operator T() const
        {
            return to<T>();
        }

        //! Explicit conversion to any container of value_type which supports push_back. This will copy elements.
        template <class T>
        T to() const
        {
            T container;
            std::copy(begin(), end(), std::back_inserter(container));
            return container;
        }

        //! Returns true if the range is empty.
        //! @{
        bool empty() const { return begin() == end(); }
        bool isEmpty() const { return empty(); }
        //! @}

        //! Returns the number of elements in the range.
        difference_type size() const { return std::distance(begin(), end()); }

        //! Returns the element at the beginning of the range. Undefined if the range is empty.
        const_reference front() const { Q_ASSERT(!empty()); return *begin(); }

        //! Returns the element at the beginning of the range, or a default value if the range is empty.
        const_reference frontOrDefault() const
        {
            static const value_type def{};
            return empty() ? def : *begin();
        }

        //! Returns the element at the beginning of the range, or a default value if the range is empty.
        value_type frontOrDefault(value_type def) const
        {
            return empty() ? def : *begin();
        }

    private:
        I m_begin;
        I m_end;

        void check(...) {};
        template <class I2, class F> void check(Iterators::ConditionalIterator<I2, F> *begin, Iterators::ConditionalIterator<I2, F> *end)
        {
            begin->checkEnd(*end);
        }
    };

    /*!
     * Streaming operators for CRange to qDebug.
     */
    //! @{
    template <class I>
    QDebug operator <<(QDebug d, const CRange<I> &range)
    {
        for (const auto &v : range) { d << v; }
        return d;
    }
    template <class I>
    QNoDebug operator <<(QNoDebug d, const CRange<I> &)
    {
        return d;
    }
    //! @}

    /*!
     * Returns a CRange constructed from begin and end iterators of deduced types.
     * \param begin The begin iterator.
     * \param end The end iterator, which can be any iterator type convertible to I.
     */
    template <class I, class I2>
    auto makeRange(I begin, I2 end) -> CRange<I>
    {
        return { begin, static_cast<I>(end) };
    }

    /*!
     * Returns a CRange constructed from the begin and end iterators of the given container.
     *
     * The returned CRange may or may not be const, depending on the constness of the container.
     */
    template <class T>
    auto makeRange(T &&container) -> CRange<decltype(container.begin())>
    {
        return { container.begin(), container.end() };
    }

    /*!
     * Returns a const CRange constructed from the cbegin and cend iterators of the given container.
     */
    template <class T>
    auto makeConstRange(T &&container) -> CRange<decltype(container.cbegin())>
    {
        return { container.cbegin(), container.cend() };
    }

    /*
     * Member functions of CRangeBase template defined out of line, because they depend on CRange etc.
     */
    template <class Derived, class CIt>
    template <class F>
    auto CRangeBase<Derived, CIt>::transform(F function) const
        -> CRange<Iterators::TransformIterator<CIt, F>>
    {
        return makeRange(Iterators::makeTransformIterator(derived().cbegin(), function), derived().cend());
    }

    template <class Derived, class CIt>
    template <class Predicate>
    auto CRangeBase<Derived, CIt>::findBy(Predicate p) const
        -> CRange<Iterators::ConditionalIterator<CIt, Predicate>>
    {
        return makeRange(makeConditionalIterator(derived().cbegin(), derived().cend(), p), derived().cend());
    }

    template <class Derived, class CIt>
    template <class K0, class V0, class... KeysValues>
    auto CRangeBase<Derived, CIt>::findBy(K0 k0, V0 v0, KeysValues... keysValues) const
        -> CRange<Iterators::ConditionalIterator<CIt, decltype(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...))>>
    {
        return findBy(BlackMisc::Predicates::MemberEqual(k0, v0, keysValues...));
    }

}

#endif // guard
