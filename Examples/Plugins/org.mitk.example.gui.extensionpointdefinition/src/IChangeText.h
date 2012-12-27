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

#ifndef ICHANGETEXT_H_
#define ICHANGETEXT_H_

#include <berryObject.h>

#include "org_mitk_example_gui_extensionpointdefinition_Export.h"

struct org_mitk_example_gui_extensionpointdefinition_EXPORT IChangeText
    : public virtual berry::Object
{
  berryInterfaceMacro(IChangeText, ::)

  virtual ~IChangeText();

  virtual QString ChangeText(const QString& s) = 0;

};

Q_DECLARE_INTERFACE(IChangeText, "org.mitk.example.IChangeText")

#endif /*ICHANGETEXT_H_*/
