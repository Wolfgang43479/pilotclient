/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

// Workaround for "unexpected token 'identifier'" in combaseapi.h when compiling with /permissive-
// https://developercommunity.visualstudio.com/solutions/187139/view.html
struct IUnknown;
