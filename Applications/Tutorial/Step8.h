/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef STEP8_H
#define STEP8_H

#include "Step6.h"

#ifndef DOXYGEN_IGNORE
class Step8 : public Step6
{
  Q_OBJECT
public:
  Step8( int argc, char* argv[], QWidget *parent=0 );
  ~Step8() {};

protected:
  virtual void SetupWidgets();
protected slots:
};
#endif // DOXYGEN_IGNORE

#endif // STEP8_H

/**
\example Step8.h
*/
