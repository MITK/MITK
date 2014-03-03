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

#ifndef mitkErodeTool3D_h_Included
#define mitkErodeTool3D_h_Included

#include "mitkSegTool3D.h"
#include "SegmentationExports.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Segmentation eroding tool

  This tool applies morphologic eroding on the active label.
*/
class Segmentation_EXPORT ErodeTool3D : public SegTool3D
{
  public:

    mitkClassMacro(ErodeTool3D, SegTool3D)
    itkNewMacro(ErodeTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    void Run();

    void SetRadius(int value);
    int GetRadius();

  protected:

    ErodeTool3D();
    virtual ~ErodeTool3D();

    int m_Radius;

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image< TPixel, VDimension>* input );
};

} // namespace

#endif
