/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef CHANGELABELTEXT_H_
#define CHANGELABELTEXT_H_

/// Qt
#include <QString.h>

class IChangeText {

public:
  virtual ~IChangeText();
  virtual void ChangeExtensionLabelText(QString s) = 0;

};

#endif /*BERRYIEDITORSITE_H_*/