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

#ifndef mitkSegTool3D_h_Included
#define mitkSegTool3D_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkTool.h"

namespace mitk
{

/**
  \brief Superclass for tools that operate in 3D

*/
class Segmentation_EXPORT SegTool3D : public Tool
{
  public:

    mitkClassMacro(SegTool3D, Tool);

  protected:

    SegTool3D(); // purposely hidden
    SegTool3D(const char*); // purposely hidden
    virtual ~SegTool3D();

    virtual const char* GetGroup() const;

    void PasteSegmentation( Image* targetImage, Image* sourceImage, int pixelvalue, int timestep );

    template<typename TPixel, unsigned int VImageDimension>
    void ItkPasteSegmentation( itk::Image<TPixel,VImageDimension>* targetImage, const mitk::Image* sourceImage, int pixelvalue );
 };

} // namespace

#endif
