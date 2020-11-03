/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ICHANGETEXT_H_
#define ICHANGETEXT_H_

#include <berryObject.h>

#include "org_mitk_example_gui_extensionpointdefinition_Export.h"

struct org_mitk_example_gui_extensionpointdefinition_EXPORT IChangeText : public virtual berry::Object
{
  berryObjectMacro(IChangeText);

    ~IChangeText() override;

  virtual QString ChangeText(const QString &s) = 0;
};

Q_DECLARE_INTERFACE(IChangeText, "org.mitk.example.IChangeText")

#endif /*ICHANGETEXT_H_*/
