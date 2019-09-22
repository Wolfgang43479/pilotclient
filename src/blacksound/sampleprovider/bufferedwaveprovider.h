/* Copyright (C) 2019
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKSOUND_BUFFEREDWAVEPROVIDER_H
#define BLACKSOUND_BUFFEREDWAVEPROVIDER_H

#include "blacksound/blacksoundexport.h"
#include "blacksound/sampleprovider/sampleprovider.h"

#include <QAudioFormat>
#include <QByteArray>
#include <QVector>

namespace BlackSound
{
    namespace SampleProvider
    {
        //! Buffered wave generator
        class BLACKSOUND_EXPORT CBufferedWaveProvider : public ISampleProvider
        {
            Q_OBJECT

        public:
            //! Ctor
            CBufferedWaveProvider(const QAudioFormat &format, QObject *parent = nullptr);

            void addSamples(const QVector<qint16> &samples);

            //! ISampleProvider::readSamples
            virtual int readSamples(QVector<qint16> &samples, qint64 count) override;

            int getBufferedBytes() const { return m_audioBuffer.size(); }


            void clearBuffer();

        private:
            QVector<qint16> m_audioBuffer;
            qint32 m_maxBufferSize;
        };
    } // ns
} // ns

#endif // guard