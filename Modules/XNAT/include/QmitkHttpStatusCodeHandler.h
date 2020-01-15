/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKHTTPSTATUSCODEHANDLER_H
#define QMITKHTTPSTATUSCODEHANDLER_H

#include <MitkXNATExports.h>

#include <string>

class MITKXNAT_EXPORT QmitkHttpStatusCodeHandler final
{
public:
  static bool HandleErrorMessage(const char *_errorMsg);

private:
  QmitkHttpStatusCodeHandler();
  ~QmitkHttpStatusCodeHandler();
};

#endif // QMITKHTTPSTATUSCODEHANDLER_H
