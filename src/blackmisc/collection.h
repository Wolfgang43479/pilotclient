/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*!
    \file
*/

#ifndef BLACKMISC_COLLECTION_H
#define BLACKMISC_COLLECTION_H

#include "iterator.h"
#include "containerbase.h"
#include <QScopedPointer>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <utility>

namespace BlackMisc
{

    /*!
     * \brief Generic type-erased unsequenced container with value semantics.
     * \tparam T the type of elements contained.
     *
     * Can take any suitable container class as its implementation at runtime.
     */
    template <class T>
    class CCollection : public CContainerBase<CCollection, T>
    {
    public:
        //! \brief STL compatibility
        //! @{
        typedef T key_type;
        typedef T value_type;
        typedef T &reference;
        typedef const T &const_reference;
        typedef T *pointer;
        typedef const T *const_pointer;
        typedef typename Iterators::ConstForwardIterator<T> const_iterator;
        typedef const_iterator iterator; // can't modify elements in-place
        typedef ptrdiff_t difference_type;
        typedef int size_type;
        //! @}

        /*!
         * \brief Default constructor.
         */
        CCollection() : m_pimpl(new Pimpl<QSet<T>>(QSet<T>())) {}

        /*!
         * \brief Constructor.
         * \tparam C Becomes the collection's implementation type.
         * \param c Initial value for the collection; typically empty, but could contain elements.
         */
        template <class C> CCollection(C c) : m_pimpl(new Pimpl<C>(std::move(c))) {}

        /*!
         * \brief Copy constructor.
         * \param other
         */
        CCollection(const CCollection &other) : m_pimpl(other.pimpl() ? other.pimpl()->clone() : nullptr) {}

        /*!
         * \brief Move constructor.
         * \param other
         */
        CCollection(CCollection &&other) : m_pimpl(other.m_pimpl.take()) {}

        /*!
         * \brief Assignment.
         * \tparam C Becomes the collection's new implementation type.
         * \param c New value for the collection; typically empty, but could contain elements.
         */
        template <class C> CCollection &operator =(C c) { m_pimpl.reset(new Pimpl<C>(std::move(c))); return *this; }

        /*!
         * \brief Copy assignment.
         * \param other
         * \return
         */
        CCollection &operator =(const CCollection &other) { m_pimpl.reset(other.pimpl() ? other.pimpl()->clone() : nullptr); return *this; }

        /*!
         * \brief Move assignment.
         * \param other
         * \return
         */
        CCollection &operator =(CCollection && other) { m_pimpl.reset(other.m_pimpl.take()); return *this; }

        /*!
         * \brief Change the implementation type but keep all the same elements, by copying them into the new implementation.
         * \tparam C Becomes the collection's new implementation type.
         */
        template <class C> void changeImpl(C = C()) { CCollection c = C(); for (auto i = cbegin(); i != cend(); ++i) c.insert(*i); *this = std::move(c); }

        /*!
         * \brief Like changeImpl, but uses the implementation type of another collection.
         * \param other
         * \pre The other collection must be initialized.
         */
        void useImplOf(const CCollection &other) { PimplPtr p = other.pimpl()->cloneEmpty(); for (auto i = cbegin(); i != cend(); ++i) p->insert(*i); m_pimpl.reset(p.take()); }

        /*!
         * \brief Returns iterator at the beginning of the collection.
         * \return
         */
        iterator begin() { return pimpl() ? pimpl()->begin() : iterator(); }

        /*!
         * \brief Returns iterator at the beginning of the collection.
         * \return
         */
        const_iterator begin() const { return pimpl() ? pimpl()->begin() : const_iterator(); }

        /*!
         * \brief Returns iterator at the beginning of the collection.
         * \return
         */
        const_iterator cbegin() const { return pimpl() ? pimpl()->cbegin() : const_iterator(); }

        /*!
         * \brief Returns iterator one past the end of the collection.
         * \return
         */
        iterator end() { return pimpl() ? pimpl()->end() : iterator(); }

        /*!
         * \brief Returns iterator one past the end of the collection.
         * \return
         */
        const_iterator end() const { return pimpl() ? pimpl()->end() : const_iterator(); }

        /*!
         * \brief Returns iterator one past the end of the collection.
         * \return
         */
        const_iterator cend() const { return pimpl() ? pimpl()->cend() : const_iterator(); }

        /*!
         * \brief Swap this collection with another.
         * \param other
         */
        void swap(CCollection &other) { m_pimpl.swap(other.m_pimpl); }

        /*!
         * \brief Returns number of elements in the collection.
         * \return
         */
        size_type size() const { return pimpl() ? pimpl()->size() : 0; }

        /*!
         * \brief Returns true if the collection is empty.
         * \return
         */
        bool empty() const { return pimpl() ? pimpl()->empty() : true; }

        /*!
         * \brief Synonym for empty.
         * \return
         */
        bool isEmpty() const { return empty(); }

        /*!
         * \brief Removes all elements in the collection.
         */
        void clear() { if (pimpl()) pimpl()->clear(); }

        /*!
         * \brief Inserts an element into the collection.
         * \param value
         * \return An iterator to the position where value was inserted.
         * \pre The collection must be initialized.
         */
        iterator insert(const T &value) { Q_ASSERT(pimpl()); return pimpl()->insert(value); }

        /*!
         * \brief Synonym for insert.
         * \param value
         * \return An iterator to the position where value was inserted.
         * \pre The collection must be initialized.
         */
        iterator push_back(const T &value) { return insert(value); }

        /*!
         * \brief Remove the element pointed to by the given iterator.
         * \param pos
         * \return An iterator to the position of the next element after the one removed.
         * \pre The collection must be initialized.
         */
        iterator erase(iterator pos) { Q_ASSERT(pimpl()); return pimpl()->erase(pos); }

        /*!
         * \brief Remove the range of elements between two iterators.
         * \param it1
         * \param it2
         * \return An iterator to the position of the next element after the one removed.
         * \pre The sequence must be initialized.
         */
        iterator erase(iterator it1, iterator it2) { Q_ASSERT(pimpl()); return pimpl()->erase(it1, it2); }

        /*!
         * \brief Test for equality.
         * \param other
         * \return
         * \todo Improve inefficient implementation.
         */
        bool operator ==(const CCollection &other) const { return (empty() && other.empty()) ? true : (size() != other.size() ? false : *pimpl() == *other.pimpl()); }

        /*!
         * \brief Test for inequality.
         * \param other
         * \return
         * \todo Improve inefficient implementation.
         */
        bool operator !=(const CCollection &other) const { return !(*this == other); }

    private:
        class PimplBase
        {
        public:
            virtual ~PimplBase() {}
            virtual PimplBase *clone() const = 0;
            virtual PimplBase *cloneEmpty() const = 0;
            virtual iterator begin() = 0;
            virtual const_iterator begin() const = 0;
            virtual const_iterator cbegin() const = 0;
            virtual iterator end() = 0;
            virtual const_iterator end() const = 0;
            virtual const_iterator cend() const = 0;
            virtual size_type size() const = 0;
            virtual bool empty() const = 0;
            virtual void clear() = 0;
            virtual iterator insert(const T &value) = 0;
            virtual iterator erase(iterator pos) = 0;
            virtual iterator erase(iterator it1, iterator it2) = 0;
            virtual bool operator ==(const PimplBase &other) const = 0;
        protected:
            // using SFINAE to choose whether to implement insert() in terms of either push_back() or insert(), depending on which is available
            // https://groups.google.com/forum/#!original/comp.lang.c++.moderated/T3x6lvmvvkQ/mfY5VTDJ--UJ
            class yes { char x; }; class no { yes x[2]; }; template <class X, X V> struct typecheck {};
            struct base { void push_back(); }; template <class C> struct derived : public C, public base {};
            static yes hasPushHelper(...); template <class D> static no hasPushHelper(D *, typecheck<void (base::*)(), &D::push_back> * = 0);
            template <class C> struct hasPush : public std::integral_constant<bool, sizeof(hasPushHelper((derived<C>*)0)) == sizeof(yes)> {};
            template <class C> static iterator insertImpl(typename std::enable_if< hasPush<C>::value, C>::type &c, const T &value) { c.push_back(value); return c.end() - 1; }
            template <class C> static iterator insertImpl(typename std::enable_if < !hasPush<C>::value, C >::type &c, const T &value) { return c.insert(value); }
        };

        template <class C> class Pimpl : public PimplBase
        {
        public:
            static_assert(std::is_same<T, typename C::value_type>::value, "CCollection must be initialized from a container with the same value_type.");
            Pimpl(C &&c) : m_impl(std::move(c)) {}
            PimplBase *clone() const { return new Pimpl(*this); }
            PimplBase *cloneEmpty() const { return new Pimpl(C()); }
            iterator begin() { return m_impl.begin(); }
            const_iterator begin() const { return m_impl.cbegin(); }
            const_iterator cbegin() const { return m_impl.cbegin(); }
            iterator end() { return m_impl.end(); }
            const_iterator end() const { return m_impl.cend(); }
            const_iterator cend() const { return m_impl.cend(); }
            size_type size() const { return m_impl.size(); }
            bool empty() const { return m_impl.empty(); }
            void clear() { m_impl.clear(); }
            iterator insert(const T &value) { return PimplBase::insertImpl<C>(m_impl, value); }
            iterator erase(iterator pos) { return m_impl.erase(*static_cast<const typename C::iterator *>(pos.getImpl())); }
            iterator erase(iterator it1, iterator it2) { return m_impl.erase(*static_cast<const typename C::iterator *>(it1.getImpl(), it2.getImpl())); }
            bool operator ==(const PimplBase &other) const { Pimpl copy = C(); for (auto i = other.cbegin(); i != other.cend(); ++i) copy.insert(*i); return m_impl == copy.m_impl; }
        private:
            C m_impl;
        };

        typedef QScopedPointer<PimplBase> PimplPtr;
        PimplPtr m_pimpl;

        // using these methods to access m_pimpl.data() eases the cognitive burden of correctly forwarding const
        PimplBase *pimpl() { return m_pimpl.data(); }
        const PimplBase *pimpl() const { return m_pimpl.data(); }
    };

} //namespace BlackMisc

Q_DECLARE_METATYPE(BlackMisc::CCollection<int>)
Q_DECLARE_METATYPE(BlackMisc::CCollection<uint>)
Q_DECLARE_METATYPE(BlackMisc::CCollection<qlonglong>)
Q_DECLARE_METATYPE(BlackMisc::CCollection<qulonglong>)
// CCollection<double> not instantiated because QSet<double> is not supported due to hashing constraints

#endif // guard
