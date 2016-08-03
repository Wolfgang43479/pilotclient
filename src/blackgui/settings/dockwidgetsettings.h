/* Copyright (C) 2016
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_SETTINGS_DOCKWIDGET_H
#define BLACKGUI_SETTINGS_DOCKWIDGET_H

#include "blackgui/blackguiexport.h"
#include "blackmisc/settingscache.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/dictionary.h"
#include "blackmisc/variant.h"

#include <QMap>
#include <QString>
#include <QMetaType>

namespace BlackGui
{
    namespace Settings
    {
        //! Settings for dockwidget
        class BLACKGUI_EXPORT CDockWidgetSettings :
            public BlackMisc::CValueObject<CDockWidgetSettings>
        {
        public:
            //! Properties by index
            enum ColumnIndex
            {
                IndexFloatingMargins = BlackMisc::CPropertyIndex::GlobalIndexCDockWidgetSettings,
                IndexFloatingFramelessMargins,
                IndexDockedMargins,
                IndexFrameless,
                IndexFloating
            };

            //! Default constructor
            CDockWidgetSettings();

            //! Destructor.
            ~CDockWidgetSettings() {}

            //! Reset margins to default
            void resetMarginsToDefault();

            //! Reset to defaults
            void reset();

            //! Set margins for given dock widget
            void setMarginsWhenFramelessFloating(const QMargins &margins);

            //! Margins for given dock widget
            QMargins getMarginsWhenFramelessFloating() const;

            //! Set margins for given dock widget
            void setMarginsWhenFloating(const QMargins &margins);

            //! Margins for given dock widget
            QMargins getMarginsWhenFloating() const;

            //! Set margins for given dock widget
            void setMarginsWhenDocked(const QMargins &margins);

            //! Margins for given dock widget
            QMargins getMarginsWhenDocked() const;

            //! Floating?
            bool isFloating() const { return m_floating; }

            //! Frameless?
            bool isFramless() const { return m_frameless; }

            //! Floating
            void setFloating(bool floating) { m_floating = floating; }

            //! Frameless
            void setFrameless(bool frameless) { m_frameless = frameless; }

            //! Geometry
            QByteArray getGeometry() const;

            //! Set geometry
            void setGeometry(const QByteArray &ba);

            //! \copydoc BlackMisc::Mixin::String::toQString
            QString convertToQString(bool i18n = false) const;

            //! To string
            QString convertToQString(const QString &separator, bool i18n = false) const;

            //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
            BlackMisc::CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

            //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
            void setPropertyByIndex(const BlackMisc::CPropertyIndex &index, const BlackMisc::CVariant &variant);

        private:
            QString m_floatingMargins {"0:0:0:0"};          //!< margins: when floating
            QString m_floatingFramelessMargins {"0:0:0:0"}; //!< margins, when floating and frameless
            QString m_dockedMargins {"0:0:0:0"};            //!< margins, when docked
            QString m_geometry;                             //!< geometry as HEX values
            bool m_floating = false;                        //!< floating
            bool m_frameless = false;                       //!< frameless

            BLACK_METACLASS(
                CDockWidgetSettings,
                BLACK_METAMEMBER(floatingMargins),
                BLACK_METAMEMBER(floatingFramelessMargins),
                BLACK_METAMEMBER(dockedMargins),
                BLACK_METAMEMBER(floating),
                BLACK_METAMEMBER(frameless),
                BLACK_METAMEMBER(geometry)
            );
        };

        //! Needed to compile properly with Q_DECLARE_METATYPE
        using CDockWidgetSettingsDictionary = BlackMisc::CDictionary<QString, CDockWidgetSettings, QMap>;

        //! Settings for all dock widgets
        class BLACKGUI_EXPORT CDockWidgetsSettings :
            public CDockWidgetSettingsDictionary,
            public BlackMisc::Mixin::MetaType<CDockWidgetsSettings>,
            public BlackMisc::Mixin::JsonOperators<CDockWidgetsSettings>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CDockWidgetsSettings)

            //! Default constructor.
            CDockWidgetsSettings() {}

            //! Get setting or init by estimated default values
            CDockWidgetSettings getByNameOrInitToDefault(const QString &name);

            //! Reset to defaults
            void resetToDefaults(const QString &name);
        };

        //! Trait for settings for dock widget
        struct TDockWidgets : public BlackMisc::TSettingTrait<CDockWidgetsSettings>
        {
            //! Key in data cache
            static const char *key() { return "guidockwidget"; }
        };
    } // ns
} // ns

Q_DECLARE_METATYPE(BlackGui::Settings::CDockWidgetSettings)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackGui::Settings::CDockWidgetSettings>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackGui::Settings::CDockWidgetSettings>)
Q_DECLARE_METATYPE(BlackGui::Settings::CDockWidgetSettingsDictionary)
Q_DECLARE_METATYPE(BlackGui::Settings::CDockWidgetsSettings)

#endif // guard
