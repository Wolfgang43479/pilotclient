/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/aviation/informationmessage.h"
#include "blackmisc/comparefunctions.h"

namespace BlackMisc
{
    namespace Aviation
    {
        QString CInformationMessage::convertToQString(bool i18n) const
        {
            Q_UNUSED(i18n);
            return m_message;
        }

        void CInformationMessage::registerMetadata()
        {
            CValueObject<CInformationMessage>::registerMetadata();
            qRegisterMetaType<CInformationMessage::InformationType>();
        }

        const CInformationMessage &CInformationMessage::unspecified()
        {
            static const CInformationMessage u(Unspecified);
            return u;
        }

        const QString &CInformationMessage::getTypeAsString() const
        {
            switch (m_type)
            {
            case ATIS:
                {
                    static const QString atis("ATIS");
                    return atis;
                }
            case METAR:
                {
                    static const QString metar("METAR");
                    return metar;
                }
            case TAF:
                {
                    static const QString taf("TAF");
                    return taf;
                }
            default:
                {
                    static const QString ds("unknown");
                    return ds;
                }
            }
        }

        CVariant CInformationMessage::propertyByIndex(const CPropertyIndex &index) const
        {
            if (index.isMyself()) { return CVariant::from(*this); }
            if (ITimestampBased::canHandleIndex(index)) { return ITimestampBased::propertyByIndex(index); }
            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexType: return CVariant::from(m_type);
            case IndexMessage: return CVariant::from(m_message);
            default: break;
            }
            return CValueObject::propertyByIndex(index);
        }

        void CInformationMessage::setPropertyByIndex(const CPropertyIndex &index, const CVariant &variant)
        {
            if (index.isMyself()) { (*this) = variant.to<CInformationMessage>(); return; }
            if (ITimestampBased::canHandleIndex(index)) { ITimestampBased::setPropertyByIndex(index, variant); return; }
            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexType:    m_type = static_cast<InformationType>(variant.toInt()); break;
            case IndexMessage: m_message = variant.toQString(); break;
            default: break;
            }
            CValueObject::setPropertyByIndex(index, variant);
        }

        int CInformationMessage::comparePropertyByIndex(const CPropertyIndex &index, const CInformationMessage &compareValue) const
        {
            if (index.isMyself())
            {
                const int c = Compare::compare(m_type, compareValue.m_type);
                if (c != 0) return c;
                return m_message.compare(compareValue.m_message, Qt::CaseInsensitive);
            }
            if (ITimestampBased::canHandleIndex(index)) { return ITimestampBased::comparePropertyByIndex(index, compareValue); }
            const ColumnIndex i = index.frontCasted<ColumnIndex>();
            switch (i)
            {
            case IndexMessage: return m_message.compare(compareValue.m_message, Qt::CaseInsensitive);
            case IndexType: return Compare::compare(this->getType(), compareValue.getType());
            default:
                return CValueObject::comparePropertyByIndex(index, *this);
            }
            Q_ASSERT_X(false, Q_FUNC_INFO, "Compare failed");
            return 0;
        }
    } // namespace
} // namespace
