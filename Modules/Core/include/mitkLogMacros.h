/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkLogMacros.h
 * \brief Convenience header that provides the MITK logging macros (MITK_INFO, MITK_WARN, MITK_ERROR, MITK_FATAL, MITK_DEBUG).
 *
 * This header simply includes \c mitkLog.h, which defines all logging macros and the
 * underlying \c PseudoLogStream infrastructure. It exists as a short, descriptive
 * include for translation units that only need the log macros.
 *
 * \deprecated Prefer including \c \<mitkLog.h\> directly.
 *
 * \sa mitkLog.h
 * \ingroup Core
 */

#ifndef mitkLogMacros_h
#define mitkLogMacros_h

#include <mitkLog.h>

#endif
