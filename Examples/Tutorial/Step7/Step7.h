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

#ifndef STEP7_H
#define STEP7_H

#include "Step6.h"

#ifndef DOXYGEN_IGNORE
class Step7 : public Step6
{
  Q_OBJECT
public:
  Step7( int argc, char* argv[], QWidget *parent=0 );
  ~Step7() {}

protected slots:
  virtual void StartRegionGrowing();
};
#endif // DOXYGEN_IGNORE

#endif // STEP7_H

/**
\example Step7.h
*/
