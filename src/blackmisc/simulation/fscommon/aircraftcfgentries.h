/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_FSCOMMON_AIRCRAFTCFGENTRY_H
#define BLACKMISC_SIMULATION_FSCOMMON_AIRCRAFTCFGENTRY_H

#include "blackmisc/blackmiscexport.h"
#include "blackmisc/simulation/aircraftmodel.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/variant.h"

namespace BlackMisc
{
    namespace Simulation
    {
        namespace FsCommon
        {
            //! Set of aircraft.cfg entries representing an aircraft (FSX)
            //! \remarks an entry in the aircraft.cfg is title, atc type, ...
            //!          This class already bundles relevant entries, hence the class is named Entries (plural)
            class BLACKMISC_EXPORT CAircraftCfgEntries: public BlackMisc::CValueObject<CAircraftCfgEntries>
            {
            public:
                //! Properties by index
                enum ColumnIndex
                {
                    IndexEntryIndex = BlackMisc::CPropertyIndex::GlobalIndexCAircraftCfgEntries,
                    IndexFileName,
                    IndexTitle,
                    IndexAtcType,
                    IndexAtcModel,
                    IndexParkingCode,
                    IndexDescription,
                    IndexTexture
                };

                //! Default constructor
                CAircraftCfgEntries() = default;

                //! Entries representing an aircraft
                CAircraftCfgEntries(const QString &fileName, int index);

                //! Entries representing an aircraft
                CAircraftCfgEntries(const QString &filePath, int index, const QString &title, const QString &atcType, const QString &atcModel, const QString &atcParkingCode, const QString &description);

                //! File name
                const QString &getFileName() const { return this->m_fileName; }

                //! Directory of entry
                QString getFileDirectory() const;

                //! Title
                const QString &getTitle() const { return this->m_title; }

                //! Index
                int getIndex() const { return this->m_index; }

                //! ATC model
                const QString &getAtcModel() const { return this->m_atcModel; }

                //! ATC type
                const QString &getAtcType() const { return this->m_atcType; }

                //! ATC airline
                const QString &getAtcAirline() const { return this->m_atcAirline; }

                //! Sim name
                const QString &getSimName() const { return this->m_simName; }

                //! Description
                const QString &getDescription() const { return this->m_description; }

                //! ATC parking code
                const QString &getAtcParkingCode() const { return this->m_atcParkingCode; }

                //! UI type (e.g. A321-231 IAE)
                const QString &getUiType() const { return this->m_uiType; }

                //! UI manufacturer (e.g. Airbus)
                const QString &getUiManufacturer() const { return this->m_uiManufacturer; }

                //! Texture
                const QString &getTexture() const { return this->m_texture; }

                //! Created by
                const QString &getCreatedBy() const { return this->m_createdBy; }

                //! Is Rotorcraft?
                bool isRotorcraft() const { return m_rotorcraft; }

                //! Manufacturer + type
                QString getUiCombinedDescription() const;

                //! Filepath
                void setFileName(const QString &filePath);

                //! Title
                void setTitle(const QString &title);

                //! Index
                void setIndex(int index) { this->m_index = index; }

                //! ATC model
                void setAtcModel(const QString &atcModel);

                //! ATC type
                void setAtcType(const QString &atcType);

                //! Parking code
                void setAtcParkingCode(const QString &parkingCode);

                //! Airline
                void setAtcAirline(const QString &airline);

                //! Simulator name
                void setSimName(const QString &simName);

                //! Description
                void setDescription(const QString &description);

                //! Created by
                void setCreatedBy(const QString &createdBy);

                //! Texture
                void setTexture(const QString &texture);

                //! UI type (e.g. A321-231 IAE)
                void setUiType(const QString &type);

                //! UI manufacturer (e.g. Airbus)
                void setUiManufacturer(const QString &manufacturer) { this->m_uiManufacturer = manufacturer.trimmed(); }

                //! Is Rotorcraft?
                void setRotorcraft(bool isRotorcraft) { m_rotorcraft = isRotorcraft; }

                //! To aircraft model
                BlackMisc::Simulation::CAircraftModel toAircraftModel() const;

                //! Thumbnail.jpg path if possible
                QString getThumbnailFileName() const;

                //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
                BlackMisc::CVariant propertyByIndex(const BlackMisc::CPropertyIndex &index) const;

                //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
                void setPropertyByIndex(const BlackMisc::CVariant &variant, const BlackMisc::CPropertyIndex &index);

                //! \copydoc BlackMisc::Mixin::String::toQString
                QString convertToQString(bool i18n = false) const;

            private:
                int m_index;               //!< current index in given config
                QString m_fileName;        //!< file name of .cfg
                QString m_title;           //!< Title in .cfg
                QString m_atcType;         //!< ATC type
                QString m_atcModel;        //!< ATC model
                QString m_atcAirline;      //!< ATC airline
                QString m_atcParkingCode;  //!< ATC parking code
                QString m_description;     //!< descriptive text
                QString m_uiType;          //!< e.g. A321-231 IAE
                QString m_uiManufacturer;  //!< e.g. Airbus
                QString m_texture;         //!< texture, needed to identify thumbnail.jpg
                QString m_simName;         //!< name in simulator
                QString m_createdBy;       //!< created by, "distributor"
                bool m_rotorcraft = false; //!< hint if rotorcraft

                BLACK_METACLASS(CAircraftCfgEntries,
                    BLACK_METAMEMBER(index),
                    BLACK_METAMEMBER(fileName),
                    BLACK_METAMEMBER(title),
                    BLACK_METAMEMBER(atcType),
                    BLACK_METAMEMBER(atcModel),
                    BLACK_METAMEMBER(atcParkingCode)
                );
            };
        } // ns
    } // ns
} // ns

Q_DECLARE_METATYPE(BlackMisc::Simulation::FsCommon::CAircraftCfgEntries)

#endif // guard
