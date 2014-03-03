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

#ifndef mitkOpenTool3D_h_Included
#define mitkOpenTool3D_h_Included

#include "mitkSegTool3D.h"
#include "SegmentationExports.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Morphologic opening tool.

  The segmentation smooths the active label by morphologic opening.
*/
class Segmentation_EXPORT OpenTool3D : public SegTool3D
{
  public:

    mitkClassMacro(OpenTool3D, SegTool3D);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    /// \brief Runs the tool.
    void Run();

    /// \brief Sets the radius of the kernel used in the morphologic operation.
    void SetRadius(int);

    /// \brief Sets the radius of the kernel used in the morphologic operation.
    int GetRadius();

  protected:

    OpenTool3D();
    virtual ~OpenTool3D();

    template < typename TPixel, unsigned int VDimension >
    void InternalRun( itk::Image< TPixel, VDimension>* input );

    int m_Radius;
};

} // namespace

#endif
