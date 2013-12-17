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
#include "SegmentationExports.h"
//#include "mitkStateEvent.h"
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

    /// \brief Executes the tool.
    void Run();

    /// \brief Sets the number of new labels to generate.
    void SetNumberOfConnectedRegionsToKeep(int);

  protected:

    SplitConnectedRegionsTool3D();
    virtual ~SplitConnectedRegionsTool3D();

    int m_NumberOfConnectedRegionsToKeep;

    mitk::ColorSequenceRainbow::Pointer m_ColorSequenceRainbow;

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image<TPixel, VDimension>* input );

    template <typename ImageType1, typename ImageType2>
    void InternalAcceptPreview( ImageType1* targetImage, const ImageType2* sourceImage );
};

} // namespace

#endif
