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

#include "mitkSegTool3D.h"
#include "MitkSegmentationExports.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief A tool for smoothing a label by median filtering.

  This tool applies a median smoothing filter on the active label
*/
class MitkSegmentation_EXPORT MedianTool3D : public SegTool3D
{
  public:

    mitkClassMacro(MedianTool3D, SegTool3D)
    itkNewMacro(MedianTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    /// \brief Runs the tool.
    void Run();

    /// \brief Sets the radius of the kernel used in the morphologic operation.
    void SetRadius(int);

    /// \brief Gets the radius of the kernel used in the morphologic operation.
    int GetRadius();

  protected:

    MedianTool3D();
    virtual ~MedianTool3D();

    int m_Radius;

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image< TPixel, VDimension>* input );
};

} // namespace

#endif
