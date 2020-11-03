/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef STEP7_H
#define STEP7_H

#include "Step6.h"

#ifndef DOXYGEN_IGNORE
class Step7 : public Step6
{
  Q_OBJECT
public:
  Step7(int argc, char *argv[], QWidget *parent = nullptr);
  ~Step7() override {}
protected slots:
  void StartRegionGrowing() override;
};
#endif // DOXYGEN_IGNORE

#endif // STEP7_H

/**
\example Step7.h
*/
