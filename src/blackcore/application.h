/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_APPLICATION_H
#define BLACKCORE_APPLICATION_H

#include "corefacadeconfig.h"
#include "blackmisc/logcategorylist.h"
#include "blackmisc/filelogger.h"
#include "blackmisc/slot.h"
#include "blackcoreexport.h"
#include <QObject>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QCommandLineParser>

namespace BlackCore
{
    class CCoreFacade;
    class CSetupReader;

    class IContextApplication;
    class IContextAudio;
    class IContextNetwork;
    class IContextOwnAircraft;
    class IContextSimulator;

    /*!
     * Our runtime. Normally one instance is to be initialized at the beginning of main, and thereafter
     * it can be used everywhere via QApplication::instance
     *
     * - A swift standard cmd line parser is part of the application.
     *   Hence cmd arguments can be optained any time / everywhere when required.
     *   Also some standard swift cmd arguments do not need to be re-implemented for each swift application.
     * - The core facade (aka core runtime) is now part of the application. It can be started via cmd line arguments.
     * - Settings are loaded
     * - Setup is loaded (load the so called bootsrap file) to find servers and other resources
     *
     * \sa BlackGui::CGuiApplication for the GUI version of application
     */
    class BLACKCORE_EXPORT CApplication : public QObject
    {
        Q_OBJECT

    public:
        //! Similar to \sa QCoreApplication::instance() returns the single instance
        static CApplication *instance();

        //! Constructor
        CApplication(const QString &applicationName = executable());

        //! Destructor
        virtual ~CApplication();

        //! Application name and version
        QString getApplicationNameAndVersion() const;

        //! Start services, if not yet parsed call CApplication::parse
        virtual bool start();

        //! Request to get network reply
        QNetworkReply *requestNetworkResource(const QNetworkRequest &request,
                                              const BlackMisc::CSlot<void(QNetworkReply *)> &callback);

        //! Setup already syncronized
        bool isSetupSyncronized() const;

        //! Reload setup and version
        BlackMisc::CStatusMessage requestReloadOfSetupAndVersion();

        //! Run event loop
        static int exec();

        //! Exit application, perform graceful shutdown and exit
        static void exit(int retcode = 0);

        //! Similar to QCoreApplication::arguments
        static QStringList arguments();

        // ----------------------- parsing ----------------------------------------

        //! \name parsing of command line options
        //! @{

        //! \copydoc QCommandLineParser::addOption(const QCommandLineOption &)
        bool addParserOption(const QCommandLineOption &option);

        //! CMD line argument for DBus address
        void addDBusAddressOption();

        //! DBus address from CMD line, otherwise ""
        QString getCmdDBusAddressValue() const;

        //! Delegates to QCommandLineParser::isSet
        bool isParserOptionSet(const QString &option) const;

        //! Delegates to QCommandLineParser::isSet
        bool isParserOptionSet(const QCommandLineOption &option) const;

        //! Delegates to QCommandLineParser::value
        QString getParserOptionValue(const QString &option) const;

        //! Delegates to QCommandLineParser::value
        QString getParserOptionValue(const QCommandLineOption &option) const;

        //! Display parser error message
        virtual void errorMessage(const QString &errorMessage) const;

        //! Parses and handles the standard options such as help, version, parse error
        //! \note in some cases (error, version, help) application is terminated during this step
        //! \sa parsingHookIn
        bool parse();
        //! @}

        // ----------------------- contexts ----------------------------------------

        //! \name Context / core facade related
        //! @{

        //! Supports contexts
        bool supportsContexts() const;

        //! Init the contexts part and start core facade
        //! \sa coreFacadeStarted
        void useContexts(const CCoreFacadeConfig &coreConfig);

        //! Get the facade
        CCoreFacade *getCoreFacade() { return m_coreFacade.data(); }

        //! Get the facade
        const CCoreFacade *getCoreFacade() const { return m_coreFacade.data(); }
        //! @}

        //! \name Direct access to contexts if a CCoreFacade has been initialized
        //! @{
        const IContextNetwork *getIContextNetwork() const;
        const IContextAudio *getIContextAudio() const;
        const IContextApplication *getIContextApplication() const;
        const IContextOwnAircraft *getIContextOwnAircraft() const;
        const IContextSimulator *getIContextSimulator() const;
        IContextNetwork *getIContextNetwork();
        IContextAudio *getIContextAudio();
        IContextApplication *getIContextApplication();
        IContextOwnAircraft *getIContextOwnAircraft();
        IContextSimulator *getIContextSimulator();
        //! @}

    public slots:
        //! Graceful shutdown
        virtual void gracefulShutdown();

    signals:
        //! Facade started
        void coreFacadeStarted();

    protected:
        //! Constructor
        CApplication(const QString &applicationName, QCoreApplication *app);

        //! executable name
        static const QString &executable();

        //! Own log categories
        static const BlackMisc::CLogCategoryList &cats();

        //! Display help message
        virtual void helpMessage();

        //! Display version message
        virtual void versionMessage() const;

        //! Can be used to parse specialized arguments
        virtual bool parsingHookIn() { return true; }

        //! Can be used to start special services
        virtual bool startHookIn() { return true; }

        // cmd parsing
        QCommandLineParser m_parser;                    //!< cmd parser
        QCommandLineOption m_cmdHelp {"help"};          //!< help option
        QCommandLineOption m_cmdVersion { "version" };  //!< version option
        QCommandLineOption m_cmdDBusAddress {"empty"};  //!< DBus address
        bool               m_parsed = false;            //!< Parsing accomplished?
        bool               m_started = false;           //!< started?
        bool               m_startSetupReader = false;  //!< start the setup reader

    private:
        //! init logging system
        void initLogging();

        //! Init parser
        void initParser();

        //! static init part
        static void initEnvironment();

        QScopedPointer<CCoreFacade>            m_coreFacade;             //!< core facade if any
        QScopedPointer<CSetupReader>           m_setupReader;            //!< setup reader
        QScopedPointer<BlackMisc::CFileLogger> m_fileLogger;             //!< file logger
        QNetworkAccessManager                  m_accessManager { this }; //!< single network access manager
        QString                                m_applicationName;        //!< application name
        bool                                   m_shutdown = false;       //!< is being shut down
    };
} // namespace

//! Single instance of application object
extern BLACKCORE_EXPORT BlackCore::CApplication *sApp;

#endif // guard
