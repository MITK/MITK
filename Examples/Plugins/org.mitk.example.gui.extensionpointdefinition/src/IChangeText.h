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

#include <QString>

  class IChangeText : public berry::Object
  {
  public:
    virtual void ChangeExtensionLabelText(QString s) = 0;

    virtual ~IChangeText() {}
  };

#endif /*ICHANGETEXT_H_*/
