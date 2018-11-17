/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/application.h"
#include "blackcore/setupreader.h"
#include "blackmisc/verify.h"
#include "blackmisc/compare.h"
#include "blackmisc/fileutils.h"
#include "blackmisc/directoryutils.h"
#include "blackmisc/logcategory.h"
#include "blackmisc/logcategorylist.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/network/networkutils.h"
#include "blackmisc/network/url.h"
#include "blackmisc/statusmessage.h"

#include <QByteArray>
#include <QFile>
#include <QNetworkReply>
#include <QPointer>
#include <QScopedPointer>
#include <QScopedPointerDeleteLater>
#include <QTimer>
#include <QUrl>
#include <QtGlobal>

using namespace BlackMisc;
using namespace BlackMisc::Db;
using namespace BlackMisc::Network;
using namespace BlackCore;
using namespace BlackCore::Data;

namespace BlackCore
{
    CSetupReader::CSetupReader(QObject *parent) : QObject(parent),
        m_cmdBootstrapUrl
    {
        { "url", "bootstrapurl" },
        QCoreApplication::translate("application", "Bootstrap URL, e.g. https://datastore.swift-project.org/shared"),
        "bootstrapurl", (sApp->getApplicationInfo().isUnitTest()) ? unitTestBootstrapUrl() : ""
    },  m_cmdBootstrapMode
    {
        { "bmode", "bootstrapmode" },
        QCoreApplication::translate("application", "Bootstrap mode: explicit, implicit, cache(-only)"),
        "bootstrapmode", "explicit"
    }
    { }

    QList<QCommandLineOption> CSetupReader::getCmdLineOptions() const
    {
        return QList<QCommandLineOption> {{ m_cmdBootstrapUrl, m_cmdBootstrapMode }};
    }

    CStatusMessageList CSetupReader::asyncLoad()
    {
        CStatusMessageList msgs;
        if (!m_localSetupFileValue.isEmpty())
        {
            msgs = this->readLocalBootstrapFile(m_localSetupFileValue);
            msgs.push_back(this->manageSetupAvailability(false, msgs.isSuccess()));
            return msgs;
        }

        m_setup.synchronize(); // make sure it is loaded
        const CGlobalSetup cachedSetup = m_setup.get();
        const bool cacheAvailable = cachedSetup.wasLoaded();
        msgs.push_back(cacheAvailable ?
                       CStatusMessage(this, CStatusMessage::SeverityInfo, "Cached setup synchronized and contains data") :
                       CStatusMessage(this, CStatusMessage::SeverityInfo, "Cached setup synchronized, but no data in cache"));

        if (m_bootstrapMode == CacheOnly)
        {
            m_updateInfoUrls = cachedSetup.getSwiftUpdateInfoFileUrls(); // we use the info URLs from cached setup
            msgs.push_back(cacheAvailable ?
                           CStatusMessage(this, CStatusMessage::SeverityInfo, "Cache only setup, using it as it is") :
                           CStatusMessage(this, CStatusMessage::SeverityError, "Cache only setup, but cache is empty"));
            msgs.push_back(this->manageSetupAvailability(false, false)); // treat cache as local read
            return msgs;
        }

        m_bootstrapUrls.clear(); // clean up previous values

        // web URL
        if (!m_bootstrapUrlFileValue.isEmpty())
        {
            // start with the one from cmd args
            m_bootstrapUrls.push_front(CUrl(m_bootstrapUrlFileValue));
        }

        // if ever loaded add those URLs
        if (cacheAvailable)
        {
            if (m_bootstrapMode != Explicit)
            {
                // also use previously cached URLs
                const CUrlList bootstrapCacheUrls(cachedSetup.getSwiftBootstrapFileUrls());
                m_bootstrapUrls.push_back(bootstrapCacheUrls);
                msgs.push_back(bootstrapCacheUrls.isEmpty() ?
                               CStatusMessage(this, CStatusMessage::SeverityWarning, "No bootstrap URLs in cache") :
                               CStatusMessage(this, CStatusMessage::SeverityInfo, "Adding " + QString::number(bootstrapCacheUrls.size()) + " bootstrap URLs from cache"));
            }
        }
        else
        {
            msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityInfo, "Empty cache, will not add URLs from cache"));
        }

        m_bootstrapUrls.removeDuplicates(); // clean up
        if (m_bootstrapUrls.isEmpty())
        {
            // after all still empty
            msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityInfo, "Your log files are here: " + CDirectoryUtils::logDirectory()));
            msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityError, "No bootstrap URLs, cannot load setup"));
        }
        else
        {
            CStatusMessageList readMsgs = this->triggerReadSetup(); // async loading
            if (cacheAvailable && readMsgs.isFailure())
            {
                // error, but cache is available, we can continue
                readMsgs.clampSeverity(CStatusMessage::SeverityWarning);
                msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityWarning, "Loading setup failed, but cache is available, will continue"));
                msgs.push_back(readMsgs);
            }
            else
            {
                msgs.push_back(readMsgs);
            }
        }
        return msgs;
    }

    bool CSetupReader::parseCmdLineArguments()
    {
        // copy vars at beginning to simplify a threadsafe version in the future
        const QString cmdLineBootstrapUrl = this->getCmdLineBootstrapUrl();
        BootstrapMode bootstrapMode = stringToEnum(sApp->getParserValue(m_cmdBootstrapMode));
        const bool ignoreCmdBootstrapUrl = m_ignoreCmdBootstrapUrl;
        const bool checkCmdBootstrapUrl = m_checkCmdBootstrapUrl;
        const QString bootstrapUrlFileValue = CGlobalSetup::buildBootstrapFileUrl(cmdLineBootstrapUrl);
        QString localSetupFileValue;
        const QUrl url(bootstrapUrlFileValue);
        const QString urlString(url.toString());
        bool ok = false;

        if (urlString.isEmpty() && bootstrapMode == Explicit)
        {
            bootstrapMode = Implicit; // no URL, we use implicit mode
        }

        do
        {
            // check on local file
            if (url.isLocalFile())
            {
                localSetupFileValue = url.toLocalFile();
                const QFile f(localSetupFileValue);
                if (!f.exists())
                {
                    sApp->cmdLineErrorMessage(QString("File '%1' does not exist)").arg(localSetupFileValue));
                    break;
                }
            }

            // check on explicit URL
            if (bootstrapMode == Explicit)
            {
                if (!url.isLocalFile())
                {
                    bool retry = false;

                    // "retry" possible in some cases
                    do
                    {
                        if (ignoreCmdBootstrapUrl || !checkCmdBootstrapUrl || CNetworkUtils::canConnect(url, CNetworkUtils::getLongTimeoutMs()))
                        {
                            ok = true;
                            break;
                        }
                        retry = sApp->cmdLineErrorMessage(QString("URL '%1' not reachable").arg(urlString), true);
                    }
                    while (retry);
                }
            }
        }
        while (false);

        m_localSetupFileValue = localSetupFileValue;
        m_bootstrapUrlFileValue = bootstrapUrlFileValue;
        m_bootstrapMode = bootstrapMode;
        return ok;
    }

    void CSetupReader::gracefulShutdown()
    {
        m_shutdown = true;
    }

    void CSetupReader::readSetup()
    {
        const CStatusMessageList msgs(this->triggerReadSetup());
        if (!msgs.isSuccess())
        {
            CLogMessage::preformatted(msgs);
        }
    }

    CStatusMessageList CSetupReader::triggerReadSetup()
    {
        if (!sApp || m_shutdown) { return CStatusMessage(this, CStatusMessage::SeverityError, "shutdown"); }
        if (m_bootstrapUrls.isEmpty())
        {
            const CStatusMessage m(this, CStatusMessage::SeverityError, "No bootstrap URLs");
            CStatusMessageList msgs(m);
            this->setLastSetupReadErrorMessages(msgs);
            return msgs;
        }

        // same web load within 5000ms
        const CGlobalSetup setup = m_setup.get();
        if (m_setup.lastUpdatedAge() < 5000 && setup.wasLoadedFromWeb())
        {
            const CStatusMessage m(this, CStatusMessage::SeverityInfo, "Update info just updated, skip read");
            CStatusMessageList msgs(m);
            this->setLastSetupReadErrorMessages(msgs);
            return msgs;
        }

        // trigger two reads, the file size is small
        CStatusMessage m1, m2;
        const CUrlList randomUrls = m_bootstrapUrls.randomElements(2);
        CUrl url = randomUrls.front();
        if (!url.isEmpty())
        {
            m1 = CStatusMessage(this, CStatusMessage::SeverityInfo, "Start reading bootstrap 1st URL: " + url.toQString());
            sApp->getFromNetwork(url.toNetworkRequest(), { this, &CSetupReader::parseBootstrapFile });
        }
        else
        {
            m1 = CStatusMessage(this, CStatusMessage::SeverityError, "First bootstrap URL is empty");
        }


        url = randomUrls.back();
        if (!url.isEmpty())
        {
            m2 = CStatusMessage(this, CStatusMessage::SeverityInfo, "Will also trigger deferred bootstrap reading 2nd URL: " + url.toQString());
            QPointer<CSetupReader> myself(this);
            QTimer::singleShot(2000, this, [ = ]
            {
                if (!myself || !sApp || sApp->isShuttingDown()) { return; }
                if (!m_lastSuccessfulSetupUrl.isEmpty())
                {
                    // already read
                    CLogMessage(this).info("Cancel second bootstrap read ('%1'), as there was a 1st read: '%2'") << url.toQString() << m_lastSuccessfulSetupUrl;
                    return;
                }
                sApp->getFromNetwork(url.toNetworkRequest(), { this, &CSetupReader::parseBootstrapFile });
            });
        }
        else
        {
            m2 = CStatusMessage(this, CStatusMessage::SeverityWarning, "2nd bootstrap URL is empty");
        }

        CStatusMessageList msgs({m1, m2});
        this->setLastSetupReadErrorMessages(msgs);
        return msgs;
    }

    void CSetupReader::readUpdateInfo()
    {
        const CUrlList randomUrls = m_updateInfoUrls.randomElements(2);
        if (randomUrls.isEmpty())
        {
            CLogMessage(this).warning("Cannot read update info, no URLs");
            this->manageUpdateInfoAvailability(false);
            return;
        }

        if (m_updateInfo.lastUpdatedAge() < 5000)
        {
            CLogMessage(this).info("Update info just updated, skip read");
            return;
        }

        if (!sApp || m_shutdown) { return; }
        CUrl url = randomUrls.front();
        const CStatusMessage m1(this, CStatusMessage::SeverityInfo, "Start reading update info 1st URL: " + url.toQString());
        sApp->getFromNetwork(url.toNetworkRequest(), { this, &CSetupReader::parseUpdateInfoFile });

        url = randomUrls.back();
        const CStatusMessage m2(this, CStatusMessage::SeverityInfo, "Will also trigger deferred update info reading 2nd URL: " + url.toQString());
        QPointer<CSetupReader> myself(this);
        QTimer::singleShot(2000, this, [ = ]
        {
            if (!myself || !sApp || sApp->isShuttingDown()) { return; }
            if (!m_lastSuccessfulUpdateInfoUrl.isEmpty())
            {
                // already read
                CLogMessage(this).info("Cancel second update info read ('%1'), as there was a 1st read '%2'") << url.toQString() << m_lastSuccessfulUpdateInfoUrl;
                return;
            }
            sApp->getFromNetwork(url.toNetworkRequest(), { this, &CSetupReader::parseUpdateInfoFile });
        });
    }

    CSetupReader::BootstrapMode CSetupReader::stringToEnum(const QString &s)
    {
        const QString bsm(s.toLower().trimmed());
        if (bsm.startsWith("expl")) { return Explicit; }
        if (bsm.startsWith("cache")) { return CacheOnly; }
        return Implicit;
    }

    const QString &CSetupReader::unitTestBootstrapUrl()
    {
        static const QString url("https://datastore.swift-project.org/shared");
        return url;
    }

    CStatusMessageList CSetupReader::readLocalBootstrapFile(const QString &fileName)
    {
        if (fileName.isEmpty()) { return CStatusMessage(this).error("No file name for local bootstrap file"); }
        if (!sApp || sApp->isShuttingDown()) { return CStatusMessage(this).error("No sApp, shutting down?"); }
        QString fn;
        const QFile file(fileName);
        if (!file.exists())
        {
            // relative name?
            const QString dir(sApp->getCmdSwiftPrivateSharedDir());
            if (dir.isEmpty()) { return CStatusMessage(this).error("Empty shared directory '%1' for bootstrap file") << dir; }

            // no version for local files, as those come with the current code
            fn = CFileUtils::appendFilePaths(dir, "bootstrap/" + CDirectoryUtils::bootstrapFileName());
        }
        else
        {
            fn = fileName;
        }

        const QString content(CFileUtils::readFileToString(fn));
        if (content.isEmpty()) { return CStatusMessage(this).error("File '%1' not existing or empty") << fn; }

        try
        {
            CGlobalSetup s;
            s.convertFromJson(content);
            s.markAsLoadedFromFile(true);
            const CStatusMessage setMsg = m_setup.set(s);
            const CStatusMessage setInfo = CStatusMessage(this).info("Setup cache updated from local file '%1'") << fn;
            return setMsg.isSuccess() ? setInfo : setMsg;
        }
        catch (const CJsonException &ex)
        {
            return ex.toStatusMessage(this, QString("Parsing local setup file '%1'").arg(fn));
        }
    }

    void CSetupReader::parseBootstrapFile(QNetworkReply *nwReplyPtr)
    {
        // wrap pointer, make sure any exit cleans up reply
        // required to use delete later as object is created in a different thread
        QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> nwReply(nwReplyPtr);
        if (m_shutdown) { return; }

        const QUrl url(nwReply->url());
        const QString urlString(url.toString());
        const QString replyMessage(nwReply->errorString());

        if (nwReply->error() == QNetworkReply::NoError)
        {
            qint64 lastModified = CNetworkUtils::lastModifiedMsSinceEpoch(nwReply.data());
            QString setupJson(nwReplyPtr->readAll());
            nwReplyPtr->close();
            if (setupJson.isEmpty())
            {
                CLogMessage(this).info("No bootstrap setup file at '%1'") << urlString;
            }
            else
            {
                try
                {
                    const CGlobalSetup currentSetup = m_setup.get();
                    CGlobalSetup loadedSetup;
                    loadedSetup.convertFromJson(setupJson);
                    loadedSetup.markAsLoadedFromWeb(true);
                    const CUrl sharedUrl(loadedSetup.getCorrespondingSharedUrl(url));
                    if (!sharedUrl.isEmpty()) { emit this->successfullyReadSharedUrl(sharedUrl); }

                    if (lastModified > 0 && lastModified > loadedSetup.getMSecsSinceEpoch()) { loadedSetup.setMSecsSinceEpoch(lastModified); }
                    bool sameVersionLoaded = (loadedSetup == currentSetup);
                    if (sameVersionLoaded)
                    {
                        m_updateInfoUrls = currentSetup.getSwiftUpdateInfoFileUrls(); // defaults
                        CLogMessage(this).info("Same setup version loaded from '%1' as already in data cache '%2'") << urlString << m_setup.getFilename();
                        CLogMessage::preformatted(this->manageSetupAvailability(true));
                        return; // success
                    }

                    // in the past I used to do a timestamp comparison here and skipped further setting
                    // with changed files from a different URL this was wrongly assuming outdated loaded files and was removed
                    const CStatusMessage m = m_setup.set(loadedSetup, loadedSetup.getMSecsSinceEpoch());
                    CLogMessage::preformatted(m);
                    if (m.isSeverityInfoOrLess())
                    {
                        // no issue with cache
                        m_updateInfoUrls = loadedSetup.getSwiftUpdateInfoFileUrls();
                        CLogMessage(this).info("Loaded setup from '%1'") << urlString;
                        CLogMessage(this).info("Setup: Updated data cache in '%1'") << m_setup.getFilename();
                        {
                            QWriteLocker l(&m_lockSetup);
                            m_lastSuccessfulSetupUrl = urlString;
                        }
                    }
                    CLogMessage::preformatted(this->manageSetupAvailability(true));
                    return; // success
                }
                catch (const CJsonException &ex)
                {
                    // we downloaded an unparsable JSON file.
                    // as we control those files something is wrong
                    const QString errorMsg = QString("Setup file loaded from '%1' cannot be parsed").arg(urlString);
                    const CStatusMessage msg = ex.toStatusMessage(this, errorMsg);
                    CLogMessage::preformatted(msg);

                    // in dev. I get notified, in productive code I try next URL by falling thru
                    BLACK_VERIFY_X(false, Q_FUNC_INFO, errorMsg.toLocal8Bit().constData());
                    m_bootstrapReadErrors++;
                }
            } // json empty
        } // no error
        else
        {
            // network error, log as warning as we will read again if possible
            // however, store as error because this will be a possible root cause if nothing else is
            nwReply->abort();
            m_bootstrapReadErrors++;
            if (m_bootstrapReadErrors > 1)
            {
                // getting here means at least 2 URLs failes
                this->manageSetupAvailability(false); // this updates also the cache availability so we can contine with cache only
            }
        }
    }

    void CSetupReader::parseUpdateInfoFile(QNetworkReply *nwReplyPtr)
    {
        // wrap pointer, make sure any exit cleans up reply
        // required to use delete later as object is created in a different thread
        QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> nwReply(nwReplyPtr);
        if (m_shutdown) { return; }

        const QUrl url(nwReply->url());
        const QString urlString(url.toString());
        const QString replyMessage(nwReply->errorString());

        if (nwReply->error() == QNetworkReply::NoError)
        {
            const qint64 lastModified = CNetworkUtils::lastModifiedMsSinceEpoch(nwReply.data());
            const QString updateInfoJsonString(nwReplyPtr->readAll());
            nwReplyPtr->close();
            if (updateInfoJsonString.isEmpty())
            {
                CLogMessage(this).info("No update info file content");
                // try next URL
            }
            else
            {
                try
                {
                    const CUpdateInfo updateInfo = CUpdateInfo::fromDatabaseJson(updateInfoJsonString);
                    if (updateInfo.isEmpty())
                    {
                        CLogMessage(this).error("Loading of update info yielded no data");
                        this->manageUpdateInfoAvailability(false);
                    }
                    else
                    {
                        CStatusMessage m = m_updateInfo.set(updateInfo, lastModified);
                        if (m.isFailure())
                        {
                            m.addCategories(getLogCategories());
                            CLogMessage::preformatted(m);
                            this->manageUpdateInfoAvailability(false);
                        }
                        else
                        {
                            {
                                QWriteLocker l(&m_lockUpdateInfo);
                                m_lastSuccessfulUpdateInfoUrl = urlString;
                            }
                            CLogMessage(this).info("Update info loaded from '%1") << urlString;
                            CLogMessage(this).info("Update info: Updated data cache in '%1', artifacts: %2, distributions: %3") << m_updateInfo.getFilename() << updateInfo.getArtifactsPilotClient().size() << updateInfo.getDistributions().size();
                            this->manageUpdateInfoAvailability(true);
                        } // cache
                    }
                    return;
                }
                catch (const CJsonException &ex)
                {
                    // we downloaded an unparsable JSON file.
                    // as we control those files something is wrong
                    const QString errorMsg = QString("Update info file loaded from '%1' cannot be parsed").arg(urlString);
                    const CStatusMessage msg = ex.toStatusMessage(this, errorMsg);
                    CLogMessage::preformatted(msg);

                    // in dev. I get notified, in productive code I try next URL by falling thru
                    BLACK_VERIFY_X(false, Q_FUNC_INFO, errorMsg.toLocal8Bit().constData());
                    m_updateInfoReadErrors++;
                }
            } // json empty
        } // no error
        else
        {
            // network error, try next URL
            nwReply->abort();
            m_updateInfoReadErrors++;
        }
    }

    const CLogCategoryList &CSetupReader::getLogCategories()
    {
        static const CLogCategoryList cats({ CLogCategory("swift.setupreader"), CLogCategory::webservice(), CLogCategory::startup()});
        return cats;
    }

    bool CSetupReader::hasCmdLineBootstrapUrl() const
    {
        return !this->getCmdLineBootstrapUrl().isEmpty();
    }

    QString CSetupReader::getCmdLineBootstrapUrl() const
    {
        if (m_ignoreCmdBootstrapUrl) return "";
        return sApp->getParserValue(m_cmdBootstrapUrl);
    }

    void CSetupReader::setIgnoreCmdLineBootstrapUrl(bool ignore)
    {
        m_ignoreCmdBootstrapUrl = ignore;
        this->parseCmdLineArguments(); // T156 this part not threadsafe, currently not a real problem as setup reader runs in main thread
    }

    CGlobalSetup CSetupReader::getSetup() const
    {
        return m_setup.get();
    }

    bool CSetupReader::hasCachedSetup() const
    {
        const CGlobalSetup cachedSetup = m_setup.get();
        const bool cacheAvailable = cachedSetup.wasLoaded();
        return cacheAvailable;
    }

    QDateTime CSetupReader::getSetupCacheTimestamp() const
    {
        return m_setup.getTimestamp();
    }

    bool CSetupReader::prefillCacheWithLocalResourceBootstrapFile()
    {
        if (m_shutdown) { return false; }
        m_setup.synchronize(); // make sure it is loaded
        const CGlobalSetup cachedSetup = m_setup.get();
        const bool cacheAvailable = cachedSetup.wasLoaded();
        if (cacheAvailable)
        {
            CLogMessage(this).info("Setup cache prefill (bootstrap already cached, no prefill needed");
            return false;
        }
        const QString fn = CDirectoryUtils::bootstrapResourceFilePath();
        const CStatusMessageList msgs = this->readLocalBootstrapFile(fn);
        CLogMessage::preformatted(msgs);
        return true;
    }

    QString CSetupReader::getLastSuccessfulSetupUrl() const
    {
        QReadLocker l(&m_lockSetup);
        return m_lastSuccessfulSetupUrl;
    }

    CUpdateInfo CSetupReader::getUpdateInfo() const
    {
        return m_updateInfo.get();
    }

    bool CSetupReader::hasCachedUpdateInfo() const
    {
        const CUpdateInfo updateInfo = m_updateInfo.get();
        return !updateInfo.isEmpty();
    }

    QDateTime CSetupReader::getUpdateInfoCacheTimestamp() const
    {
        return m_updateInfo.getTimestamp();
    }

    QString CSetupReader::getLastSuccessfulUpdateInfoUrl() const
    {
        QReadLocker l(&m_lockUpdateInfo);
        return m_lastSuccessfulUpdateInfoUrl;
    }

    void CSetupReader::synchronize()
    {
        m_setup.synchronize();
        m_updateInfo.synchronize();
    }

    CStatusMessageList CSetupReader::getLastSetupReadErrorMessages() const
    {
        QReadLocker l(&m_lockSetup);
        return m_setupReadErrorMsgs;
    }

    const QString &CSetupReader::getBootstrapUrlFile() const
    {
        if (!m_localSetupFileValue.isEmpty()) { return m_localSetupFileValue; }
        return m_bootstrapUrlFileValue;
    }

    QString CSetupReader::getBootstrapModeAsString() const
    {
        switch (m_bootstrapMode)
        {
        case CacheOnly: return "cache only";
        case Explicit:  return "explicit";
        case Implicit:  return "implicit";
        default: break;
        }
        return "";
    }

    void CSetupReader::setLastSetupReadErrorMessages(const CStatusMessageList &messages)
    {
        QWriteLocker l(&m_lockSetup);
        m_setupReadErrorMsgs = messages.getErrorMessages();
    }

    CStatusMessageList CSetupReader::manageSetupAvailability(bool webRead, bool localRead)
    {
        Q_ASSERT_X(!(webRead && localRead), Q_FUNC_INFO, "Local and web read together seems to be wrong");
        CStatusMessageList msgs;
        QPointer<CSetupReader> myself(this);

        if (webRead)
        {
            msgs.push_back(CStatusMessage(this).info("Setup loaded from web, will trigger read of update information"));
            QTimer::singleShot(500, this, [ = ]
            {
                if (!myself) { return; }
                this->readUpdateInfo();
            });
        }

        if (localRead)
        {
            msgs.push_back(CStatusMessage(this).info("Setup loaded locally, will trigger read of update information"));
            QTimer::singleShot(500, this, [ = ]
            {
                if (!myself) { return; }
                this->readUpdateInfo();
            });
        }

        bool available = false;
        if (webRead || localRead)
        {
            available = true;
        }
        else
        {
            const bool cacheAvailable = m_setup.get().wasLoadedFromWeb();
            available = cacheAvailable && m_bootstrapMode != Explicit;
        }

        if (available && !webRead && !localRead)
        {
            msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityInfo, "Setup available, but not updated this time"));
        }
        else if (!available)
        {
            msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityError, "Setup not available"));
            if (m_bootstrapMode == Explicit)
            {
                msgs.push_back(CStatusMessage(this, CStatusMessage::SeverityError, "Mode is 'explicit', likely URL '" + m_bootstrapUrlFileValue + "' is not reachable"));
            }
        }
        m_setupAvailable = available;
        emit this->setupHandlingCompleted(available);

        if (!webRead && !localRead)
        {
            msgs.push_back(CStatusMessage(this).warning("Since setup was not updated this time, will not start loading of update information"));
            this->manageUpdateInfoAvailability(false);
        }
        return msgs;
    }

    void CSetupReader::manageUpdateInfoAvailability(bool webRead)
    {
        if (webRead)
        {
            m_updateInfoAvailable = true;
            emit updateInfoAvailable(true);
        }
        else
        {
            const bool cached = m_updateInfo.isSaved();
            m_updateInfoAvailable = cached;
            emit updateInfoAvailable(cached);
        }
    }
} // namespace
