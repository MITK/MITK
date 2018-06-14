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

#ifndef QMITKSEMANTICRELATIONSUIHELPER_H
#define QMITKSEMANTICRELATIONSUIHELPER_H

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

#endif // QMITKSEMANTICRELATIONSUIHELPER_H
