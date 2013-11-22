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

#ifndef mitkFillHolesTool3D_h_Included
#define mitkFillHolesTool3D_h_Included

#include "mitkAutoSegmentationTool.h"
#include "mitkLegacyAdaptors.h"
#include "SegmentationExports.h"
#include "mitkDataNode.h"
#include "mitkLabelSetImage.h"
#include "mitkStateEvent.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Segmentation hole filling tool

  This tool fills holes on the active label.
*/
class Segmentation_EXPORT FillHolesTool3D : public AutoSegmentationTool
{
  public:

    mitkClassMacro(FillHolesTool3D, AutoSegmentationTool)
    itkNewMacro(FillHolesTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    void Run();

  protected:

    FillHolesTool3D();
    virtual ~FillHolesTool3D();

    template < typename TPixel, unsigned int VDimension >
    void InternalProcessing( itk::Image< TPixel, VDimension>* input );
};

} // namespace

#endif
