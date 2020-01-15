/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef STEP8_H
#define STEP8_H

#include "Step6.h"

#ifndef DOXYGEN_IGNORE
class Step8 : public Step6
{
  Q_OBJECT
public:
  Step8(int argc, char *argv[], QWidget *parent = nullptr);
  ~Step8() override {}
protected:
  void SetupWidgets() override;
protected slots:
};
#endif // DOXYGEN_IGNORE

#endif // STEP8_H

/**
\example Step8.h
*/
