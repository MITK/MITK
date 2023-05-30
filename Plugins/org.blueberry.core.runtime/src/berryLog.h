/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_LOG_H__
#define __BERRY_LOG_H__

#include <mitkLog.h>

#include <org_blueberry_core_runtime_Export.h>

#define BERRY_INFO MITK_INFO("BlueBerry")
#define BERRY_WARN MITK_WARN("BlueBerry")
#define BERRY_ERROR MITK_ERROR("BlueBerry")
#define BERRY_FATAL MITK_FATAL("BlueBerry")

#define BERRY_DEBUG MITK_DEBUG("BlueBerry")

org_blueberry_core_runtime_EXPORT std::ostream& operator<<(std::ostream& os, const QString& str);

#endif /*__BERRY_LOG_H__*/
