/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_AIRCRAFTMATCHERSETUP_H
#define BLACKMISC_SIMULATION_AIRCRAFTMATCHERSETUP_H

#include "blackmisc/valueobject.h"
#include "blackmisc/blackmiscexport.h"


namespace BlackMisc
{
    namespace Simulation
    {
        //! Matcher settings
        class BLACKMISC_EXPORT CAircraftMatcherSetup : public CValueObject<CAircraftMatcherSetup>
        {
        public:
            //! Matching algorithm
            enum MatchingAlgorithm
            {
                MatchingScoreBased,
                MatchingStepwiseReduce
            };

            //! Enabled matching mode flags
            enum MatchingModeFlag
            {
                ByModelString    = 1 << 0, //!< allow exact model string match
                ByIcaoData       = 1 << 1,
                ByFamily         = 1 << 2,
                ByLivery         = 1 << 3,
                ByCombinedType   = 1 << 4,
                ByIcaoOrderAircraftFirst = (1 << 5) | ByIcaoData,
                ByIcaoOrderAirlineFirst  = (1 << 6) | ByIcaoData,
                // --- score based matching ---
                ScoreIgnoreZeros         = 1 << 7, //!< zero scores are ignored
                ScorePreferColorLiveries = 1 << 8, //!< prefer color liveries
                // --- others ---
                ModeNone          = 0,
                ModeScoreDefault  = ScoreIgnoreZeros | ScorePreferColorLiveries,
                ModeDefaultScore  = ByIcaoOrderAircraftFirst | ByModelString | ByCombinedType | ModeScoreDefault,
                ModeDefaultReduce = ByModelString | ByFamily | ByLivery | ByCombinedType | ByIcaoOrderAircraftFirst
            };
            Q_DECLARE_FLAGS(MatchingMode, MatchingModeFlag)

            //! How to pick among similar candiates
            enum PickSimilarStrategy
            {
                PickFirst,
                PickRandom,
                PickByOrder
            };

            //! Properties by index
            enum ColumnIndex
            {
                IndexMatchingAlgorithm = CPropertyIndex::GlobalIndexCAircraftMatcherSetup,
                IndexMatchingMode,
                IndexPickStrategy
            };

            //! Constructor
            CAircraftMatcherSetup();

            //! Constructor
            CAircraftMatcherSetup(MatchingAlgorithm algorithm);

            //! Constructor
            CAircraftMatcherSetup(MatchingAlgorithm algorithm, MatchingMode mode, PickSimilarStrategy pickStrategy);

            //! Algorithm
            MatchingAlgorithm getMatchingAlgorithm() const { return static_cast<MatchingAlgorithm>(m_algorithm); }

            //! Algorithm as string
            const QString &getMatchingAlgorithmAsString() const { return algorithmToString(this->getMatchingAlgorithm()); }

            //! Algorithm
            bool setMatchingAlgorithm(MatchingAlgorithm algorithm, bool reset = true);

            //! Matching mode
            MatchingMode getMatchingMode() const { return static_cast<MatchingMode>(m_mode); }

            //! Matching mode as string
            QString getMatchingModeAsString() const { return modeToString(this->getMatchingMode()); }

            //! Dynamic offset values?
            void setMatchingMode(MatchingMode mode) { m_mode = static_cast<int>(mode); }

            //! Strategy among equally suitable models
            PickSimilarStrategy getPickStrategy() const { return static_cast<PickSimilarStrategy>(m_strategy); }

            //! Strategy as string
            const QString &getPickStrategyAsString() const { return strategyToString(this->getPickStrategy()); }

            //! Set the strategy
            void setPickStrategy(PickSimilarStrategy strategy) { m_strategy = static_cast<int>(strategy); }

            //! \copydoc BlackMisc::Mixin::String::toQString
            QString convertToQString(bool i18n = false) const;

            //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
            BlackMisc::CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const BlackMisc::CPropertyIndex &index, const BlackMisc::CVariant &variant);

            //! Reset
            void reset(MatchingAlgorithm algorithm);

            //! Algorithm to string
            static const QString &algorithmToString(MatchingAlgorithm algorithm);

            //! Enumeration as string
            static const QString &modeFlagToString(MatchingModeFlag modeFlag);

            //! Enumeration as string
            static QString modeToString(MatchingMode mode);

            //! Strategy to string
            static const QString &strategyToString(PickSimilarStrategy strategy);

            //! Mode by flags
            static MatchingMode matchingMode(
                bool byModelString, bool byIcaoDataAircraft1st, bool byIcaoDataAirline1st, bool byFamily, bool byLivery, bool byCombinedType,
                bool scoreIgnoreZeros, bool scorePreferColorLiveries);

        private:
            int m_algorithm = static_cast<int>(MatchingScoreBased);
            int m_mode = static_cast<int>(ModeDefaultScore);
            int m_strategy = static_cast<int>(PickByOrder);

            BLACK_METACLASS(
                CAircraftMatcherSetup,
                BLACK_METAMEMBER(algorithm),
                BLACK_METAMEMBER(mode),
                BLACK_METAMEMBER(strategy)
            );
        };
    } // ns
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Simulation::CAircraftMatcherSetup)
Q_DECLARE_METATYPE(BlackMisc::Simulation::CAircraftMatcherSetup::MatchingAlgorithm)
Q_DECLARE_METATYPE(BlackMisc::Simulation::CAircraftMatcherSetup::MatchingMode)
Q_DECLARE_METATYPE(BlackMisc::Simulation::CAircraftMatcherSetup::MatchingModeFlag)
Q_DECLARE_METATYPE(BlackMisc::Simulation::CAircraftMatcherSetup::PickSimilarStrategy)
Q_DECLARE_OPERATORS_FOR_FLAGS(BlackMisc::Simulation::CAircraftMatcherSetup::MatchingMode)

#endif // guard