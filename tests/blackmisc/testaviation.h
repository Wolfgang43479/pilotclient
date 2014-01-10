/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISCTEST_TESTAVIATIONBASE_H
#define BLACKMISCTEST_TESTAVIATIONBASE_H

#include <QtTest/QtTest>

namespace BlackMiscTest
{

    /*!
     * \brief Aviation classes basic tests
     */
    class CTestAviation : public QObject
    {
        Q_OBJECT

    public:
        /*!
         * \brief Standard test case constructor
         * \param parent
         */
        explicit CTestAviation(QObject *parent = nullptr);

    private slots:
        /*!
         * \brief Basic unit tests for physical units
         */
        void headingBasics();
        /*!
         * \brief COM and NAV units
         */
        void comAndNav();

        /*!
         * \brief Transponder
         */
        void transponder();

        /*!
         * \brief Callsigns
         */
        void callsign();

        /*!
         * \brief Testing copying and equality of objects
         */
        void copyAndEqual();
    };

} // namespace

#endif // guard
