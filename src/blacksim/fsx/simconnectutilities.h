/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKSIM_FSX_SIMCONNECTUTILITIES_H
#define BLACKSIM_FSX_SIMCONNECTUTILITIES_H

#include <QString>
#include <QMetaEnum>

// apart from the below definitions, the following code is OS independent,
// though it does not make sense to be used on non WIN machines.
// But it allows parts as GUI to comile
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
typedef unsigned long   DWORD; //!< Fake Windows DWORD
#endif

namespace BlackSim
{
    namespace Fsx
    {
        //! Utilities for SimConnect
        class CSimConnectUtilities : public QObject
        {
            Q_OBJECT
            Q_ENUMS(SIMCONNECT_EXCEPTION)
            Q_ENUMS(SIMCONNECT_SURFACE)

        public:
            //! Path to local config file
            static QString getLocalSimConnectCfgFilename();

            /*!
             * Content for FSX simconnect.cfg file
             * \param ip IP address of FSX
             * \param port Port of FSX (e.g. 500)
             * \return content for simconnect.cfg
             */
            static QString simConnectCfg(const QString &ip, quint16 port = 500);

            /*!
             * Create a FSX simconnect.cfg file
             * \param fileName and path
             * \param ip IP address of FSX
             * \param port Port of FSX (e.g. 500)
             * \return success
             */
            static bool writeSimConnectCfg(const QString &fileName, const QString &ip, quint16 port = 500);

            /*!
             * Resolve SimConnect exception (based on Qt metadata).
             * \param id enum element
             * \return enum element's name
             */
            static const QString simConnectExceptionToString(const DWORD id);

            /*!
             * Resolve SimConnect surface (based on Qt metadata).
             * \param type enum element
             * \param beautify remove "_"
             * \return
             */
            static const QString simConnectSurfaceTypeToString(const DWORD type, bool beautify = true);

            //! SimConnect surfaces.
            // http://msdn.microsoft.com/en-us/library/cc526981.aspx#AircraftFlightInstrumentationData
            enum SIMCONNECT_SURFACE
            {
                Concrete,
                Grass,
                Water,
                Grass_bumpy,
                Asphalt,
                Short_grass,
                Long_grass,
                Hard_turf,
                Snow,
                Ice,
                Urban,
                Forest,
                Dirt,
                Coral,
                Gravel,
                Oil_treated,
                Steel_mats,
                Bituminus,
                Brick,
                Macadam,
                Planks,
                Sand,
                Shale,
                Tarmac,
                Wright_flyer_track
            };

            //! SimConnect exceptions.
            enum SIMCONNECT_EXCEPTION
            {
                SIMCONNECT_EXCEPTION_NONE,
                SIMCONNECT_EXCEPTION_ERROR,
                SIMCONNECT_EXCEPTION_SIZE_MISMATCH,
                SIMCONNECT_EXCEPTION_UNRECOGNIZED_ID,
                SIMCONNECT_EXCEPTION_UNOPENED,
                SIMCONNECT_EXCEPTION_VERSION_MISMATCH,
                SIMCONNECT_EXCEPTION_TOO_MANY_GROUPS,
                SIMCONNECT_EXCEPTION_NAME_UNRECOGNIZED,
                SIMCONNECT_EXCEPTION_TOO_MANY_EVENT_NAMES,
                SIMCONNECT_EXCEPTION_EVENT_ID_DUPLICATE,
                SIMCONNECT_EXCEPTION_TOO_MANY_MAPS,
                SIMCONNECT_EXCEPTION_TOO_MANY_OBJECTS,
                SIMCONNECT_EXCEPTION_TOO_MANY_REQUESTS,
                SIMCONNECT_EXCEPTION_WEATHER_INVALID_PORT,
                SIMCONNECT_EXCEPTION_WEATHER_INVALID_METAR,
                SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_GET_OBSERVATION,
                SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_CREATE_STATION,
                SIMCONNECT_EXCEPTION_WEATHER_UNABLE_TO_REMOVE_STATION,
                SIMCONNECT_EXCEPTION_INVALID_DATA_TYPE,
                SIMCONNECT_EXCEPTION_INVALID_DATA_SIZE,
                SIMCONNECT_EXCEPTION_DATA_ERROR,
                SIMCONNECT_EXCEPTION_INVALID_ARRAY,
                SIMCONNECT_EXCEPTION_CREATE_OBJECT_FAILED,
                SIMCONNECT_EXCEPTION_LOAD_FLIGHTPLAN_FAILED,
                SIMCONNECT_EXCEPTION_OPERATION_INVALID_FOR_OBJECT_TYPE,
                SIMCONNECT_EXCEPTION_ILLEGAL_OPERATION,
                SIMCONNECT_EXCEPTION_ALREADY_SUBSCRIBED,
                SIMCONNECT_EXCEPTION_INVALID_ENUM,
                SIMCONNECT_EXCEPTION_DEFINITION_ERROR,
                SIMCONNECT_EXCEPTION_DUPLICATE_ID,
                SIMCONNECT_EXCEPTION_DATUM_ID,
                SIMCONNECT_EXCEPTION_OUT_OF_BOUNDS,
                SIMCONNECT_EXCEPTION_ALREADY_CREATED,
                SIMCONNECT_EXCEPTION_OBJECT_OUTSIDE_REALITY_BUBBLE,
                SIMCONNECT_EXCEPTION_OBJECT_CONTAINER,
                SIMCONNECT_EXCEPTION_OBJECT_AI,
                SIMCONNECT_EXCEPTION_OBJECT_ATC,
                SIMCONNECT_EXCEPTION_OBJECT_SCHEDULE
            };

            //! Register metadata
            static void registerMetadata();

        private:
            /*!
             * Resolve enum value to its cleartext (based on Qt metadata).
             * \param id enum element
             * \param enumName name of the resolved enum
             * \return enum element's name
             */
            static const QString resolveEnumToString(const DWORD id, const char *enumName);

            //! Hidden constructor
            CSimConnectUtilities();

        };
    }
}

Q_DECLARE_METATYPE(BlackSim::Fsx::CSimConnectUtilities::SIMCONNECT_EXCEPTION)
Q_DECLARE_METATYPE(BlackSim::Fsx::CSimConnectUtilities::SIMCONNECT_SURFACE)

#endif // guard
