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
