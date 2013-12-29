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

#ifndef mitkCloseTool3D_h_Included
#define mitkCloseTool3D_h_Included

#include "mitkSegTool3D.h"
#include "SegmentationExports.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Morphologic closing tool.

  This tool smooths the active label by morphologic closing.
*/
class Segmentation_EXPORT CloseTool3D : public SegTool3D
{
  public:

    mitkClassMacro(CloseTool3D, SegTool3D)
    itkNewMacro(CloseTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    /// \brief Executes the tool.
    void Run();

    /// \brief Sets the radius of the kernel used in the morphologic operation.
    void SetRadius(int);

    /// \brief Gets the radius of the kernel used in the morphologic operation.
    int GetRadius();

  protected:

    CloseTool3D();
    virtual ~CloseTool3D();

    int m_Radius;

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image< TPixel, VDimension>* input );
};

} // namespace

#endif
