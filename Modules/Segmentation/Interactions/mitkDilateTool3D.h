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

#ifndef mitkDilateTool3D_h_Included
#define mitkDilateTool3D_h_Included

#include "mitkAutoSegmentationTool.h"
#include "SegmentationExports.h"

#include "itkImage.h"

namespace us {
class ModuleResource;
}

namespace mitk
{

/**
  \brief Segmentation dilating tool

  This tool applies morphologic dilating on the active label.
*/
class Segmentation_EXPORT DilateTool3D : public AutoSegmentationTool
{
  public:

    mitkClassMacro(DilateTool3D, AutoSegmentationTool)
    itkNewMacro(DilateTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    void Run();

    void SetRadius(int);
    int GetRadius();

  protected:

    DilateTool3D();
    virtual ~DilateTool3D();

    int m_Radius;

    template < typename TPixel, unsigned int VDimension >
    void InternalProcessing( itk::Image< TPixel, VDimension>* input );
};

} // namespace

#endif
