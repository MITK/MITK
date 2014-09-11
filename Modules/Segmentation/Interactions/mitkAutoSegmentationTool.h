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

#ifndef mitkAutoSegmentationTool_h_Included
#define mitkAutoSegmentationTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkTool.h"

namespace mitk
{

class Image;

/**
  \brief Superclass for tool that create a new segmentation without user interaction in render windows

  This class is undocumented. Ask the creator ($Author$) to supply useful comments.
*/
class MitkSegmentation_EXPORT AutoSegmentationTool : public Tool
{
  public:

    mitkClassMacro(AutoSegmentationTool, Tool);

    void SetOverwriteExistingSegmentation(bool overwrite);

    /**
     * @brief Gets the name of the currently selected segmentation node
     * @return the name of the segmentation node or an empty string if
     *         none is selected
     */
    std::string GetCurrentSegmentationName();

    /**
     * @brief Depending on the selected mode either returns the currently selected segmentation
     *        or creates a new one from the selected reference data and adds the new segmentation
     *        to the datastorage
     * @return a mitk::DataNode which contains a segmentation image
     */
    virtual mitk::DataNode* GetTargetSegmentationNode();

  protected:

    AutoSegmentationTool(); // purposely hidden
    virtual ~AutoSegmentationTool();

    virtual const char* GetGroup() const;

    virtual itk::SmartPointer<Image> Get3DImage(itk::SmartPointer<Image> image, unsigned int timestep);

    bool m_OverwriteExistingSegmentation;
 };

} // namespace

#endif

