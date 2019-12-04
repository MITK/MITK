/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkIGTCommonHelper_H
#define QmitkIGTCommonHelper_H

#include "MitkIGTUIExports.h"
#include <QString>

 /** Documentation:
  *   \brief Simple and fast access to a pre-configured TrackingDeviceSource.
  *
  *   \ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkIGTCommonHelper
{

public:

  static const std::string VIEW_ID;

  static void SetLastFileSavePath(const QString& str);

  static void SetLastFileSavePathByFileName(const QString& str);

  static const QString GetLastFileSavePath();

  static void SetLastFileLoadPath(const QString& str);

  static void SetLastFileLoadPathByFileName(const QString& str);

  static const QString GetLastFileLoadPath();
};
#endif
