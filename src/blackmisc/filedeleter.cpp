/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/filedeleter.h"

#include <QFile>
#include <QtGlobal>

class QTimerEvent;

namespace BlackMisc
{
    void CFileDeleter::addFileForDeletion(const QString &file)
    {
        if (file.isEmpty()) { return; }
        if (!this->m_fileNames.contains(file)) this->m_fileNames.append(file);
    }

    void CFileDeleter::addFilesForDeletion(const QStringList &files)
    {
        if (files.isEmpty()) { return; }
        this->m_fileNames.append(files);
    }

    CFileDeleter::~CFileDeleter()
    {
        this->deleteFiles();
    }

    void CFileDeleter::deleteFiles()
    {
        const QStringList files(m_fileNames);
        m_fileNames.clear();

        for (const QString &fn : files)
        {
            if (fn.isEmpty()) { continue; }
            QFile f(fn);
            f.remove();
        }
    }

    CTimedFileDeleter::CTimedFileDeleter(const QString &file, int deleteAfterMs, QObject *parent) :
        QObject(parent)
    {
        Q_ASSERT_X(!file.isEmpty(), Q_FUNC_INFO, "No file name");
        if (deleteAfterMs < 100) { deleteAfterMs = 100; } // makes sure timer is started properly
        this->m_fileDeleter.addFileForDeletion(file);
        m_timerId = startTimer(deleteAfterMs);
    }

    void CTimedFileDeleter::timerEvent(QTimerEvent *event)
    {
        Q_UNUSED(event);
        if (m_timerId > 0) { this->killTimer(m_timerId); }
        m_timerId = -1;
        m_fileDeleter.deleteFiles();
        this->deleteLater();
    }

} // ns
