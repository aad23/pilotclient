/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKINPUT_MACROS_H
#define BLACKINPUT_MACROS_H

#include <QtGlobal>

/*!
 * \def BLACKINPUT_EXPORT
 * BlackInput Export Macro
 */

#ifndef WITH_STATIC
#  if defined(BUILD_BLACKINPUT_LIB)
#    define BLACKINPUT_EXPORT Q_DECL_EXPORT
#  else
#    define BLACKINPUT_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define BLACKINPUT_EXPORT
#endif

#endif // BLACKINPUT_MACROS_H
