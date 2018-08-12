/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "aircraftmatchersetup.h"
#include <QStringBuilder>

namespace BlackMisc
{
    namespace Simulation
    {
        CAircraftMatcherSetup::CAircraftMatcherSetup()
        {
            this->reset(MatchingScoreBased);
        }

        CAircraftMatcherSetup::CAircraftMatcherSetup(CAircraftMatcherSetup::MatchingAlgorithm algorithm)
        {
            this->reset(algorithm);
        }

        CAircraftMatcherSetup::CAircraftMatcherSetup(CAircraftMatcherSetup::MatchingAlgorithm algorithm, MatchingMode mode, PickSimilarStrategy pickStrategy)
        {
            this->setPickStrategy(pickStrategy);
            this->setMatchingAlgorithm(algorithm, false);
            this->setMatchingMode(mode);
        }

        bool CAircraftMatcherSetup::setMatchingAlgorithm(CAircraftMatcherSetup::MatchingAlgorithm algorithm, bool reset)
        {
            if (this->getMatchingAlgorithm() == algorithm) { return false; }
            if (reset)
            {
                this->reset(algorithm);
            }
            else
            {
                m_algorithm = static_cast<int>(algorithm);
            }
            return true;
        }

        QString CAircraftMatcherSetup::convertToQString(bool i18n) const
        {
            Q_UNUSED(i18n);
            return QStringLiteral("algorithm: '") % this->getMatchingAlgorithmAsString() %
                   QStringLiteral("' mode: '") % this->getMatchingModeAsString() %
                   QStringLiteral("' strategy: ") % this->getPickStrategyAsString() %
                   QStringLiteral("'");
        }

        CVariant CAircraftMatcherSetup::propertyByIndex(const CPropertyIndex &index) const
        {
            if (index.isMyself()) { return CVariant::from(*this); }
            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexMatchingAlgorithm: return CVariant::fromValue(m_algorithm);
            case IndexMatchingMode: return CVariant::fromValue(m_mode);
            case IndexPickStrategy: return CVariant::fromValue(m_strategy);
            default: break;
            }
            return CValueObject::propertyByIndex(index);
        }

        void CAircraftMatcherSetup::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
        {
            if (index.isMyself()) { (*this) = variant.to<CAircraftMatcherSetup>(); return; }
            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexMatchingAlgorithm: m_algorithm = variant.toInt(); break;
            case IndexMatchingMode: m_mode = variant.toInt(); break;
            case IndexPickStrategy: m_strategy = variant.toInt(); break;
            default: break;
            }
            CValueObject::setPropertyByIndex(index, variant);
        }

        void CAircraftMatcherSetup::reset(CAircraftMatcherSetup::MatchingAlgorithm algorithm)
        {
            m_algorithm = static_cast<int>(algorithm);
            MatchingMode mode = ModeNone;
            switch (algorithm)
            {
            case MatchingStepwiseReduce:
                mode = ModeDefaultReduce;
                break;
            case MatchingScoreBased:
            default:
                mode = ModeDefaultScore;
                break;
            }
            this->setMatchingMode(mode);
            this->setPickStrategy(PickByOrder);
        }

        const QString &CAircraftMatcherSetup::algorithmToString(CAircraftMatcherSetup::MatchingAlgorithm algorithm)
        {
            static const QString s("score based");
            static const QString r("stepwise reduce");
            switch (algorithm)
            {
            case MatchingStepwiseReduce: return r;
            case MatchingScoreBased:
            default: break;
            }
            return s;
        }

        const QString &CAircraftMatcherSetup::modeFlagToString(MatchingModeFlag modeFlag)
        {
            static const QString ms("by model string");
            static const QString icao("by ICAO");
            static const QString icaoAircraft("by ICAO, aircraft first");
            static const QString icaoAirline("by ICAO, airline first");
            static const QString family("by family");
            static const QString livery("by livery");
            static const QString combined("by combined combined");
            static const QString noZeros("scoring, ignore zero scores");
            static const QString preferColorLiveries("scoring, prefer color liveries");

            switch (modeFlag)
            {
            case ByModelString:  return ms;
            case ByIcaoData:     return icao;
            case ByFamily:       return family;
            case ByLivery:       return livery;
            case ByCombinedType: return combined;
            case ByIcaoOrderAircraftFirst: return icaoAircraft;
            case ByIcaoOrderAirlineFirst: return icaoAirline;
            case ScoreIgnoreZeros: return noZeros;
            case ScorePreferColorLiveries: return preferColorLiveries;
            default: break;
            }

            static const QString unknown("unknown");
            return unknown;
        }

        QString CAircraftMatcherSetup::modeToString(MatchingMode mode)
        {
            QStringList modes;
            if (mode.testFlag(ByModelString))  { modes << modeFlagToString(ByModelString); }
            if (mode.testFlag(ByIcaoData))     { modes << modeFlagToString(ByIcaoData); }
            if (mode.testFlag(ByIcaoOrderAircraftFirst)) { modes << modeFlagToString(ByIcaoOrderAircraftFirst); }
            if (mode.testFlag(ByIcaoOrderAirlineFirst))  { modes << modeFlagToString(ByIcaoOrderAirlineFirst); }
            if (mode.testFlag(ByFamily))         { modes << modeFlagToString(ByFamily); }
            if (mode.testFlag(ByLivery))         { modes << modeFlagToString(ByLivery); }
            if (mode.testFlag(ByCombinedType))   { modes << modeFlagToString(ByCombinedType); }
            if (mode.testFlag(ScoreIgnoreZeros)) { modes << modeFlagToString(ScoreIgnoreZeros); }
            if (mode.testFlag(ScorePreferColorLiveries)) { modes << modeFlagToString(ScorePreferColorLiveries); }
            return modes.join(", ");
        }

        const QString &CAircraftMatcherSetup::strategyToString(CAircraftMatcherSetup::PickSimilarStrategy strategy)
        {
            static const QString f("first");
            static const QString o("order");
            static const QString r("random");

            switch (strategy)
            {
            case PickFirst:  return f;
            case PickByOrder:  return o;
            case PickRandom: return r;
            default: break;
            }

            static const QString unknown("unknown");
            return unknown;
        }

        CAircraftMatcherSetup::MatchingMode CAircraftMatcherSetup::matchingMode(
            bool byModelString, bool byIcaoDataAircraft1st, bool byIcaoDataAirline1st, bool byFamily, bool byLivery, bool byCombinedType,
            bool scoreIgnoreZeros, bool scorePreferColorLiveries)
        {
            MatchingMode mode = byModelString ? ByModelString : ModeNone;
            if (byIcaoDataAircraft1st) { mode |= ByIcaoOrderAircraftFirst; }
            if (byIcaoDataAirline1st)  { mode |= ByIcaoOrderAirlineFirst; }
            if (byFamily)         { mode |= ByFamily; }
            if (byLivery)         { mode |= ByLivery; }
            if (byCombinedType)   { mode |= ByCombinedType; }
            if (scoreIgnoreZeros) { mode |= ScoreIgnoreZeros; }
            if (scorePreferColorLiveries) { mode |= ScorePreferColorLiveries; }
            return mode;
        }
    } // namespace
} // namespace