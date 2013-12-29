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

#ifndef mitkKeepNConnectedRegionsTool3D_h_Included
#define mitkKeepNConnectedRegionsTool3D_h_Included

#include "mitkSegTool3D.h"
#include "SegmentationExports.h"

#include "itkImage.h"


namespace us {
class ModuleResource;
}


namespace mitk
{

/**
  \brief Keeps the N largest connected regions.

  This tool keeps the N largest connected regions in the active label.
*/
class Segmentation_EXPORT KeepNConnectedRegionsTool3D : public SegTool3D
{
  public:

    mitkClassMacro(KeepNConnectedRegionsTool3D, SegTool3D)
    itkNewMacro(KeepNConnectedRegionsTool3D)

    /* icon stuff */
    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;

    void Run();

    void SetNumberOfConnectedRegionsToKeep(int);

  protected:

    KeepNConnectedRegionsTool3D();
    virtual ~KeepNConnectedRegionsTool3D();

    int m_NumberOfConnectedRegionsToKeep;

    template < typename TPixel, unsigned int VDimension >
    void InternalProcessing( itk::Image< TPixel, VDimension>* input );
};

} // namespace

#endif
