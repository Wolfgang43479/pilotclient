/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

#include "blackmisc/threadutils.h"

#include <QCoreApplication>
#include <QObject>
#include <QThread>
#include <QtGlobal>
#include <QPointer>
#include <QTimer>

namespace BlackMisc
{
    bool CThreadUtils::isCurrentThreadObjectThread(const QObject *toBeTested)
    {
        Q_ASSERT_X(toBeTested, Q_FUNC_INFO, "missing QObject");
        Q_ASSERT_X(toBeTested->thread(), Q_FUNC_INFO, "missing thread");
        return (QThread::currentThread() == toBeTested->thread());
    }

    bool CThreadUtils::isApplicationThreadObjectThread(const QObject *toBeTested)
    {
        Q_ASSERT_X(toBeTested, Q_FUNC_INFO, "missing QObject");
        Q_ASSERT_X(toBeTested->thread(), Q_FUNC_INFO, "missing thread");
        if (!QCoreApplication::instance() || !QCoreApplication::instance()->thread()) { return false; }
        return (QCoreApplication::instance()->thread() == toBeTested->thread());
    }

    bool CThreadUtils::isApplicationThread(const QThread *toBeTested)
    {
        if (!toBeTested || !QCoreApplication::instance() || !QCoreApplication::instance()->thread()) { return false; }
        return (QCoreApplication::instance()->thread() == toBeTested);
    }

    bool CThreadUtils::isCurrentThreadApplicationThread()
    {
        if (!QCoreApplication::instance()) { return false; }
        if (!QCoreApplication::instance()->thread()) { return false; }
        return (QCoreApplication::instance()->thread() == QThread::currentThread());
    }

    const QString &CThreadUtils::priorityToString(QThread::Priority priority)
    {
        static const QString idle("idle");
        static const QString lowest("lowest");
        static const QString low("low");
        static const QString normal("normal");
        static const QString high("high");
        static const QString highest("highest");
        static const QString time("time critical");
        static const QString inherit("inherit");

        switch (priority)
        {
        case QThread::IdlePriority:    return idle;
        case QThread::LowestPriority:  return lowest;
        case QThread::LowPriority:     return low;
        case QThread::NormalPriority:  return normal;
        case QThread::HighPriority:    return high;
        case QThread::HighestPriority: return highest;
        case QThread::InheritPriority: return inherit;
        case QThread::TimeCriticalPriority: return time;
        default: break;
        }

        static const QString unknown("unknown");
        return unknown;
    }

    const QString CThreadUtils::threadToString(const void *t)
    {
        return QStringLiteral("0x%1").arg(reinterpret_cast<long long>(t), 0, 16);
    }

    const QString CThreadUtils::threadInfo(const QThread *thread)
    {
        static const QString info("thread: %1 name: '%2' priority: '%3'");
        if (!thread) { return QString("no thread"); }
        return info.arg(threadToString(thread), thread->objectName(), priorityToString(thread->priority()));
    }

    const QString CThreadUtils::currentThreadInfo()
    {
        return threadInfo(QThread::currentThread());
    }

    bool CThreadUtils::callInObjectThread(QObject *object, std::function<void()> callFunct)
    {
        if (!object) { return false; }
        if (CThreadUtils::isCurrentThreadObjectThread(object)) {  return false; }

        QPointer<QObject> myself(object);
        QTimer::singleShot(0, object, [ = ]
        {
            if (!myself) { return; }
            callFunct();
        });
        return true;
    }
} // ns
