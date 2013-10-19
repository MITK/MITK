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

#ifndef mitkMedianTool3D_h_Included
#define mitkMedianTool3D_h_Included

#include "mitkAutoSegmentationTool.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"
#include "mitkDataNode.h"
#include "mitkLabelSetImage.h"
#include "mitkToolCommand.h"
#include "mitkStateEvent.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Median segmentation tool.

  The segmentation smooths all labels by median filtering.
*/
class Segmentation_EXPORT MedianTool3D : public AutoSegmentationTool
{
  public:

    mitkClassMacro(MedianTool3D, AutoSegmentationTool)
    itkNewMacro(MedianTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    void Run();

  protected:

    MedianTool3D();
    virtual ~MedianTool3D();

    mitk::ToolCommand::Pointer m_ProgressCommand;

    template < typename ImageType >
    void ITKProcessing( typename ImageType* input );
};

} // namespace

#endif
