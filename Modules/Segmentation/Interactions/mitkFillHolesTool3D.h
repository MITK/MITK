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
#include "SegmentationExports.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief A tool for filling holes in a label

  This tool fills holes on the active label. There are two types of holes.
  One is a region belonging to the "exterior" that is completly surrounded
  by the active label. The default behavior of this tool is to fill such
  holes. The other type of holes is one that has some parts surrounded
  by other labels. SetCondiderAllLabels(true) enables this mode.
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

    /// \brief Runs the tool.
    void Run();

    void SetConsiderAllLabels(bool);

  protected:

    FillHolesTool3D();
    virtual ~FillHolesTool3D();

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image< TPixel, VDimension>* input );

    bool m_ConsiderAllLabels;
};

} // namespace

#endif
