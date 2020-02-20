/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKCUSTOMVARIANTS_H_
#define QMITKCUSTOMVARIANTS_H_

#include <mitkDataNode.h>

typedef mitk::DataNode::Pointer mitkDataNodePtr;

Q_DECLARE_METATYPE(mitkDataNodePtr)
Q_DECLARE_METATYPE(mitk::DataNode *)

#endif /* QMITKCUSTOMVARIANTS_H_ */
