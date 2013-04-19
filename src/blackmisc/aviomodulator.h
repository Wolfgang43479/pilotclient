/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKMISC_AVIOMODULATORUNIT_H
#define BLACKMISC_AVIOMODULATORUNIT_H
#include "blackmisc/aviobase.h"

namespace BlackMisc
{
namespace Aviation
{

/*!
 * \brief Base class for COM, NAV, Squawk units.
 */
template <class AVIO> class CModulator : public CAvionicsBase
{
private:
    BlackMisc::PhysicalQuantities::CFrequency m_frequencyActive; //!< active frequency
    BlackMisc::PhysicalQuantities::CFrequency m_frequencyStandby; //!< standby frequency

protected:
    int m_digits; //!< digits used

    /*!
     * \brief Default constructor
     */
    CModulator() : CAvionicsBase("default") {}

    /*!
     * \brief Copy constructor
     * \param otherUnit
     */
    CModulator(const CModulator &otherUnit) :
        m_frequencyActive(otherUnit.m_frequencyActive), m_frequencyStandby(otherUnit.m_frequencyStandby),
        m_digits(otherUnit.m_digits), CAvionicsBase(otherUnit.getName()) {}

    /*!
     * \brief Constructor
     * \param name
     * \param activeFrequency
     * \param standbyFrequency
     * \param digits
     */
    CModulator(const QString &name, const BlackMisc::PhysicalQuantities::CFrequency &activeFrequency, const BlackMisc::PhysicalQuantities::CFrequency &standbyFrequency, int digits) :
        m_frequencyActive(activeFrequency),
        m_digits(digits), m_frequencyStandby(standbyFrequency), CAvionicsBase(name) { }

    /*!
     * \brief String for converter
     * \return
     */
    virtual QString stringForConverter() const;

    /*!
     * \brief Set active frequency
     * \param frequencyKHz
     */
    void setFrequencyActiveKHz(double frequencyKHz)
    {
        this->m_frequencyActive = CFrequency(frequencyKHz, CFrequencyUnit::kHz());
    }

    /*!
     * \brief Set standby frequency
     * \param frequencyKHz
     */
    void setFrequencyStandbyKHz(double frequencyKHz)
    {
        this->m_frequencyStandby = CFrequency(frequencyKHz, CFrequencyUnit::kHz());
    }

    /*!
     * \brief Set active frequency
     * \param frequencyMHz
     */
    void setFrequencyActiveMHz(double frequencyMHz)
    {
        this->m_frequencyActive = BlackMisc::PhysicalQuantities::CFrequency(frequencyMHz, BlackMisc::PhysicalQuantities::CFrequencyUnit::MHz());
    }

    /*!
     * \brief Set standby frequency
     * \param frequencyMHz
     */
    void setFrequencyStandbyMHz(double frequencyMHz)
    {
        this->m_frequencyStandby = BlackMisc::PhysicalQuantities::CFrequency(frequencyMHz, BlackMisc::PhysicalQuantities::CFrequencyUnit::MHz());
    }

    /*!
     * \brief Assigment operator =
     * \param otherModulator
     * \return
     */
    CModulator &operator =(const CModulator &otherModulator);

    /*!
     * \brief operator ==
     * \param otherModulator
     * \return
     */
    bool operator ==(const CModulator &otherModulator) const;

    /*!
     * \brief operator !=
     * \param otherModulator
     * \return
     */
    bool operator !=(const CModulator &otherModulator) const;
    /*!
     * \brief COM1
     * \return
     */
    static const QString &NameCom1()
    {
        static QString n("COM1");
        return n;
    }

    /*!
     * \brief COM2
     * \return
     */
    static const QString &NameCom2()
    {
        static QString n("COM2");
        return n;
    }

    /*!
     * \brief COM3
     * \return
     */
    static const QString &NameCom3()
    {
        static QString n("COM3");
        return n;
    }

    /*!
     * \brief NAV1
     * \return
     */
    static const QString &NameNav1()
    {
        static QString n("NAV1");
        return n;
    }

    /*!
     * \brief NAV2
     * \return
     */
    static const QString &NameNav2()
    {
        static QString n("NAV2");
        return n;
    }

    /*!
     * \brief NAV2
     * \return
     */
    static const QString &NameNav3()
    {
        static QString n("NAV2");
        return n;
    }

    /*!
     * \brief ADF1
     * \return
     */
    static const QString &NameAdf1()
    {
        static QString n("ADF1");
        return n;
    }

    /*!
     * \brief ADF2
     * \return
     */
    static const QString &NameAdf2()
    {
        static QString n("ADF2");
        return n;
    }

    /*!
     * \brief Frequency not set
     * \return
     */
    static const BlackMisc::PhysicalQuantities::CFrequency &FrequencyNotSet()
    {
        static BlackMisc::PhysicalQuantities::CFrequency f;
        return f;
    }



public:
    /*!
     * \brief Virtual destructor
     */
    virtual ~CModulator() {}

    /*!
     * \brief Default value
     * \return
     */
    virtual bool isDefaultValue() const
    {
        return this->m_frequencyActive == CModulator::FrequencyNotSet();
    }
    /*!
     * \brief Toggle active and standby frequencies
     */
    void toggleActiveStandby();

    /*!
     * \brief Active frequency
     * \return
     */
    BlackMisc::PhysicalQuantities::CFrequency getFrequencyActive() const
    {
        return this->m_frequencyActive;
    }

    /*!
     * \brief Standby frequency
     * \return
     */
    BlackMisc::PhysicalQuantities::CFrequency getFrequencyStandby() const
    {
        return this->m_frequencyActive;
    }

    /*!
     * \brief Set active frequency
     * \param frequency
     */
    void setFrequencyActive(const BlackMisc::PhysicalQuantities::CFrequency &frequency)
    {
        this->m_frequencyActive = frequency;
    }

    /*!
     * \brief Set standby frequency
     * \param frequency
     */
    void setFrequencyStandby(const BlackMisc::PhysicalQuantities::CFrequency &frequency)
    {
        this->m_frequencyStandby = frequency;
    }
};

} // namespace
} // namespace

#endif // guard
