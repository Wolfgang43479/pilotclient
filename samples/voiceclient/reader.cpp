/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "reader.h"
#include <QFile>
#include <iostream>

void LineReader::run()
{
    QFile file;
    file.open(stdin, QIODevice::ReadOnly | QIODevice::Text);
    forever
    {
        std::cout << "voice> ";
        QString line = file.readLine().trimmed();
        if (! line.isEmpty())
        {
            emit command(line);
        }
    }
}
