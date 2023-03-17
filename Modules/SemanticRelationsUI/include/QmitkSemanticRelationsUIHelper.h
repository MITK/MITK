/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSemanticRelationsUIHelper_h
#define QmitkSemanticRelationsUIHelper_h

// semantic relations ui module
#include "MitkSemanticRelationsUIExports.h"

// mitk core
#include <mitkDataNode.h>

// qt
#include <QPixmap>

/**
* @brief Provides a helper function to generate a pixmap from a given image node.
*/
namespace QmitkSemanticRelationsUIHelper
{
  /*
  * @brief Generates a QPixmap of a DICOM image.
  *
  *       The center sagittal image slice is extracted and used as the thumbnail image.
  *
  * @par dataNode    The data node that holds the image data.
  */
  MITKSEMANTICRELATIONSUI_EXPORT QPixmap GetPixmapFromImageNode(const mitk::DataNode* dataNode);

} // namespace QmitkSemanticRelationsUIHelper

#endif
