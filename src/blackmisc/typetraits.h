/* Copyright (C) 2016
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_TYPETRAITS_H
#define BLACKMISC_TYPETRAITS_H

#include <type_traits>
#include <utility> // for std::swap

class QDBusArgument;

namespace BlackMisc
{

    class CPropertyIndex;

    //! \cond PRIVATE
    // Own implementation of C++17 std::void_t, simple variadic alias
    // template which is always void. Useful for expression SFINAE.
    template <typename...>
    using void_t = void;
    //! \endcond

    namespace Private
    {
        //! \private Own implementation of C++17 std::is_nothrow_swappable.
        template <typename T, typename U>
        struct is_nothrow_swappable
        {
            static constexpr bool impl()
            {
                using std::swap;
                return noexcept(swap(std::declval<T>(), std::declval<U>()))
                    && noexcept(swap(std::declval<U>(), std::declval<T>()));
            }
            static constexpr bool value = impl();
        };

        //! \private Dummy that derives from T if T is a class.
        template <typename T, bool = std::is_class<T>::value>
        struct SyntheticDerived : public T {};
        //! \cond
        template <typename T>
        struct SyntheticDerived<T, false> {};
        //! \endcond
    }

    /*!
     * Whether an input parameter type should be passed by value or by const reference.
     */
    enum class ParameterPassBy
    {
        Value,
        ConstRef
    };

    /*!
     * Trait to detect properties of function parameter types.
     */
    template <typename T>
    struct TParameter
    {
        //! Whether the input parameter type T should be passed by value or by const reference.
        static constexpr ParameterPassBy passBy = (sizeof(T) <= 16 && std::is_trivially_copy_constructible<T>::value && std::is_trivially_destructible<T>::value) ? ParameterPassBy::Value : ParameterPassBy::ConstRef;
    };

    /*!
     * Trait to detect whether T contains a member function toQString.
     */
    template <typename T, typename = void_t<>>
    struct THasToQString : public std::false_type {};
    //! \cond
    template <typename T>
    struct THasToQString<T, void_t<decltype(std::declval<T>().toQString())>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait which is true if the expression a.push_back(v) is valid when a and v are instances of T and T::value_type.
     */
    template <typename T, typename = void_t<>>
    struct THasPushBack : public std::false_type {};
    //! \cond
    template <typename T>
    struct THasPushBack<T, void_t<decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait to detect whether a class T has a static member function named getLogCategories.
     */
    template <typename T, typename = void_t<>>
    struct THasGetLogCategories : public std::false_type {};
    //! \cond
    template <typename T>
    struct THasGetLogCategories<T, void_t<decltype(T::getLogCategories())>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait to detect whether a class T can be used as a key in a QHash.
     */
    template <typename T, typename = void_t<>>
    struct TModelsQHashKey : public std::false_type {};
    //! \cond
    template <typename T>
    struct TModelsQHashKey<T, void_t<decltype(std::declval<T>() == std::declval<T>(), qHash(std::declval<T>()))>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait to detect whether a class T can be used as a key in a QMap.
     */
    template <typename T, typename = void_t<>>
    struct TModelsQMapKey : public std::false_type {};
    //! \cond
    template <typename T>
    struct TModelsQMapKey<T, void_t<decltype(std::declval<T>() < std::declval<T>())>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait which is true if the expression compare(a, b) is valid when a and b are instances of T and U.
     */
    template <typename T, typename U, typename = void_t<>>
    struct THasCompare : public std::false_type {};
    //! \cond
    template <typename T, typename U>
    struct THasCompare<T, U, void_t<decltype(compare(std::declval<T>(), std::declval<U>()))>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait which is true if the expression a.compareByPropertyIndex(b, i) is valid when a and b are instances of T,
     * and i is an instance of CPropertyIndex.
     */
    template <typename T, typename = void_t<>>
    struct THasComparePropertyByIndex : public std::false_type {};
    //! \cond
    template <typename T>
    struct THasComparePropertyByIndex<T, void_t<decltype(std::declval<T>().comparePropertyByIndex(std::declval<const CPropertyIndex &>(), std::declval<const T &>()))>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait which is true if the expression a.propertyByIndex(i) is valid with a is an instance of T and i is an
     * instance of CPropertyIndex.
     */
    template <typename T, typename = void_t<>>
    struct THasPropertyByIndex : public std::false_type {};
    //! \cond
    template <typename T>
    struct THasPropertyByIndex<T, void_t<decltype(std::declval<T>().propertyByIndex(std::declval<CPropertyIndex>()))>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait which is true if the expression a == b is valid when a and b are instances of T and U.
     */
    template <typename T, typename U, typename = void_t<>>
    struct TIsEqualityComparable : public std::false_type {};
    //! \cond
    template <typename T, typename U>
    struct TIsEqualityComparable<T, U, void_t<decltype(std::declval<T>() == std::declval<U>())>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait which is true if T has methods marshallToDbus and unmarshallFromDbus.
     */
    template <typename T, typename = void_t<>>
    struct THasMarshallMethods : public std::false_type {};
    //! \cond
    template <typename T>
    struct THasMarshallMethods<T, void_t<decltype(std::declval<const T &>().marshallToDbus(std::declval<QDBusArgument &>()),
                                                  std::declval<T &>().unmarshallFromDbus(std::declval<const QDBusArgument &>()))>> : public std::true_type {};
    //! \endcond

    /*!
     * Trait that detects if a type is a member of a parameter pack.
     */
    template <typename T, typename... Ts>
    struct TIsOneOf : public std::false_type {};
    //! \cond
    template <typename T, typename... Ts>
    struct TIsOneOf<T, T, Ts...> : public std::true_type {};
    template <typename T, typename T2, typename... Ts>
    struct TIsOneOf<T, T2, Ts...> : public TIsOneOf<T, Ts...> {};
    //! \endcond

    /*!
     * Trait that detects if a type is QPrivateSignal.
     */
    template <typename T, typename = void_t<>>
    struct TIsQPrivateSignal : public std::false_type {};
    //! \cond
    template <typename T>
    struct TIsQPrivateSignal<T, void_t<typename Private::SyntheticDerived<T>::QPrivateSignal>> : public std::is_same<T, typename Private::SyntheticDerived<T>::QPrivateSignal> {};
    //! \endcond

}

#endif
