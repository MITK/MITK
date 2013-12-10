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

#ifndef mitkSplitConnectedRegionsTool3D_h_Included
#define mitkSplitConnectedRegionsTool3D_h_Included

#include "mitkAutoSegmentationTool.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"
#include "mitkDataNode.h"
#include "mitkLabelSetImage.h"
#include "mitkToolCommand.h"
#include "mitkStateEvent.h"
#include "mitkColorSequenceRainbow.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Splits the active label into N connected region.

  This tool splits the active label into N connected regions and generates a new label for each one.
*/
class Segmentation_EXPORT SplitConnectedRegionsTool3D : public AutoSegmentationTool
{
  public:

    mitkClassMacro(SplitConnectedRegionsTool3D, AutoSegmentationTool)
    itkNewMacro(SplitConnectedRegionsTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    /// \brief Replaces the active label with the preview image.
    virtual void AcceptPreview();

    void Run();

    void SetNumberOfConnectedRegionsToKeep(int);

  protected:

    SplitConnectedRegionsTool3D();
    virtual ~SplitConnectedRegionsTool3D();

    int m_NumberOfConnectedRegionsToKeep;

    mitk::ColorSequenceRainbow::Pointer m_ColorSequenceRainbow;

    template < typename TPixel, unsigned int VDimension >
    void InternalProcessing( itk::Image<TPixel, VDimension>* input );

    template < typename ImageType >
    void InternalAcceptPreview( ImageType* targetImage, const mitk::Image* sourceImage );
};

} // namespace

#endif
