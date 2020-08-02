/* Copyright (C) 2014
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_DBUS_H
#define BLACKMISC_DBUS_H

#include "blackmisc/metaclass.h"
#include "blackmisc/inheritancetraits.h"
#include "blackmisc/typetraits.h"
#include "blackmisc/blackmiscexport.h"
#include <QDBusArgument>
#include <type_traits>

namespace BlackMisc
{
    class CEmpty;

    /*!
     * Tag type signifying overloaded marshalling methods that preserve data at the expense of size.
     */
    class LosslessTag {};

    // *INDENT-OFF*
    namespace Private
    {
        //! \cond PRIVATE
        template <class T, std::enable_if_t<THasMarshallMethods<T>::value, int> = 0>
        void marshallMember(QDBusArgument &arg, const T &value, std::false_type) { value.marshallToDbus(arg); }
        template <class T, std::enable_if_t<THasMarshallMethods<T>::value, int> = 0>
        void marshallMember(QDBusArgument &arg, const T &value, std::true_type) { value.marshallToDbus(arg, LosslessTag()); }
        template <class T, std::enable_if_t<!THasMarshallMethods<T>::value, int> = 0>
        void marshallMember(QDBusArgument &arg, const T &value, std::false_type) { arg << value; }

        template <class T, std::enable_if_t<THasMarshallMethods<T>::value, int> = 0>
        void unmarshallMember(const QDBusArgument &arg, T &value, std::false_type) { value.unmarshallFromDbus(arg); }
        template <class T, std::enable_if_t<THasMarshallMethods<T>::value, int> = 0>
        void unmarshallMember(const QDBusArgument &arg, T &value, std::true_type) { value.unmarshallFromDbus(arg, LosslessTag()); }
        template <class T, std::enable_if_t<!THasMarshallMethods<T>::value, int> = 0>
        void unmarshallMember(const QDBusArgument &arg, T &value, std::false_type) { arg >> value; }
        //! \endcond
    }
    // *INDENT-ON*

    namespace Mixin
    {
        /*!
         * CRTP class template which will generate marshalling operators for a derived class with its own marshalling implementation.
         *
         * \tparam Derived Must implement public methods void marshallToDbus(QDBusArgument &arg) const and void unmarshallFromDbus(const QDBusArgument &arg).
         */
        template <class Derived, class...>
        class DBusOperators
        {
        public:
            //! Unmarshalling operator >>, DBus to object
            friend const QDBusArgument &operator>>(const QDBusArgument &arg, Derived &obj)
            {
                arg.beginStructure();
                obj.unmarshallFromDbus(arg);
                arg.endStructure();
                return arg;
            }

            //! Marshalling operator <<, object to DBus
            friend QDBusArgument &operator<<(QDBusArgument &arg, const Derived &obj)
            {
                arg.beginStructure();
                obj.marshallToDbus(arg);
                arg.endStructure();
                return arg;
            }
        };
        template <class Derived>
        class DBusOperators<Derived, LosslessTag> {};

        /*!
         * CRTP class template from which a derived class can inherit common methods dealing with marshalling instances by metaclass.
         *
         * \see BLACKMISC_DECLARE_USING_MIXIN_DBUS
         */
        template <class Derived, class... Tags>
        class DBusByMetaClass : public DBusOperators<Derived, Tags...>
        {
        public:
            //! Marshall without begin/endStructure, for when composed within another object
            void marshallToDbus(QDBusArgument &arg, Tags...) const
            {
                baseMarshall(static_cast<const TBaseOfT<Derived> *>(derived()), arg);
                constexpr auto meta = introspect<Derived>().without(MetaFlags<DisabledForMarshalling>());
                meta.forEachMember([ &, this ](auto member)
                {
                    using lossless = std::integral_constant<bool, member.has(MetaFlags<LosslessMarshalling>())>;
                    Private::marshallMember(arg, member.in(*this->derived()), lossless());
                });
            }

            //! Unmarshall without begin/endStructure, for when composed within another object
            void unmarshallFromDbus(const QDBusArgument &arg, Tags...)
            {
                baseUnmarshall(static_cast<TBaseOfT<Derived> *>(derived()), arg);
                constexpr auto meta = introspect<Derived>().without(MetaFlags<DisabledForMarshalling>());
                meta.forEachMember([ &, this ](auto member)
                {
                    using lossless = std::integral_constant<bool, member.has(MetaFlags<LosslessMarshalling>())>;
                    Private::unmarshallMember(arg, member.in(*this->derived()), lossless());
                });
            }

        private:
            const Derived *derived() const { return static_cast<const Derived *>(this); }
            Derived *derived() { return static_cast<Derived *>(this); }

            template <typename T> static void baseMarshall(const T *base, QDBusArgument &arg) { base->marshallToDbus(arg, Tags()...); }
            template <typename T> static void baseUnmarshall(T *base, const QDBusArgument &arg) { base->unmarshallFromDbus(arg, Tags()...); }
            static void baseMarshall(const void *, QDBusArgument &) {}
            static void baseUnmarshall(void *, const QDBusArgument &) {}
            static void baseMarshall(const CEmpty *, QDBusArgument &) {}
            static void baseUnmarshall(CEmpty *, const QDBusArgument &) {}
        };

        // *INDENT-OFF*
        /*!
         * When a derived class and a base class both inherit from Mixin::DBusByTuple,
         * the derived class uses this macro to disambiguate the inherited members.
         */
#       define BLACKMISC_DECLARE_USING_MIXIN_DBUS(DERIVED, ...)                                                         \
            using ::BlackMisc::Mixin::DBusByMetaClass<DERIVED BLACK_TRAILING_VA_ARGS(__VA_ARGS__)>::marshallToDbus;     \
            using ::BlackMisc::Mixin::DBusByMetaClass<DERIVED BLACK_TRAILING_VA_ARGS(__VA_ARGS__)>::unmarshallFromDbus;
        // *INDENT-ON*

    } // Mixin
} // BlackMisc

/*!
 * Non member non-friend streaming for std::string
 */
QDBusArgument &operator <<(QDBusArgument &arg, const std::string &s);

/*!
 * Operator for std::string from QDBusArgument.
 */
const QDBusArgument &operator >>(const QDBusArgument &arg, std::string &s);

// *INDENT-OFF*
/*!
 * Operator for streaming enums to QDBusArgument.
 */
template <class E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
QDBusArgument &operator <<(QDBusArgument &arg, const E &value)
{
    arg.beginStructure();
    arg << static_cast<int>(value);
    arg.endStructure();
    return arg;
}

/*!
 * Operator for streaming enums from QDBusArgument.
 */
template <class E, std::enable_if_t<std::is_enum<E>::value, int> = 0>
const QDBusArgument &operator >>(const QDBusArgument &arg, E &value)
{
    int temp;
    arg.beginStructure();
    arg >> temp;
    arg.endStructure();
    value = static_cast<E>(temp);
    return arg;
}

/*!
 * Operator for streaming QFlags to QDBusArgument.
 */
template <class T>
QDBusArgument &operator <<(QDBusArgument &arg, const QFlags<T> &value)
{
    arg.beginStructure();
    arg << static_cast<typename QFlags<T>::Int>(value);
    arg.endStructure();
    return arg;
}

/*!
 * Operator for streaming QFlags from QDBusArgument.
 */
template <class T>
const QDBusArgument &operator >>(const QDBusArgument &arg, QFlags<T> &value)
{
    typename QFlags<T>::Int temp = 0;
    arg.beginStructure();
    arg >> temp;
    arg.endStructure();
    value = static_cast<QFlags<T>>(temp);
    return arg;
}

/*!
 * Operator for streaming pairs to QDBusArgument.
 */
template <class A, class B>
QDBusArgument &operator <<(QDBusArgument &arg, const std::pair<A, B> &pair)
{
    arg.beginStructure();
    arg << pair.first << pair.second;
    arg.endStructure();
    return arg;
}

/*!
 * Operator for streaming pairs from QDBusArgument.
 */
template <class A, class B>
const QDBusArgument &operator >>(const QDBusArgument &arg, std::pair<A, B> &pair)
{
    arg.beginStructure();
    arg >> pair.first >> pair.second;
    arg.endStructure();
    return arg;
}
// *INDENT-ON*

//! Windows: prevents unloading of QtDBus shared library until the process is terminated.
//! QtDBus must have been loaded already by the calling process.
//! Does nothing on non-Windows platforms.
BLACKMISC_EXPORT void preventQtDBusDllUnload();

#endif // guard
