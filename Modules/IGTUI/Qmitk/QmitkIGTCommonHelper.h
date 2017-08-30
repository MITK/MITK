/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
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
