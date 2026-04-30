/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCustomVariants_h
#define QmitkCustomVariants_h

/**
 * \file QmitkCustomVariants.h
 * \brief Declares custom Qt metatypes for MITK data node pointers.
 *
 * This header registers mitk::DataNode::Pointer (smart pointer) and
 * mitk::DataNode* (raw pointer) as Qt metatypes so they can be stored
 * in QVariant and used with the Qt signal/slot mechanism.
 *
 * \sa QmitkEnums
 */

#include <mitkDataNode.h>

/** \brief Convenience typedef for mitk::DataNode::Pointer used as a Qt metatype. */
typedef mitk::DataNode::Pointer mitkDataNodePtr;

Q_DECLARE_METATYPE(mitkDataNodePtr)
Q_DECLARE_METATYPE(mitk::DataNode *)

#endif
