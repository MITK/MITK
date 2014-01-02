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
#ifndef mitkTriangleThresholdingTool3D_H
#define mitkTriangleThresholdingTool3D_H

#include "mitkSegTool3D.h"
#include "SegmentationExports.h"
#include "mitkColorSequenceRainbow.h"

#include "itkImage.h"

namespace us {
class ModuleResource;
}

namespace mitk{
  class Segmentation_EXPORT TriangleThresholdingTool3D : public SegTool3D
  {
    public:

      mitkClassMacro(TriangleThresholdingTool3D, SegTool3D);
      itkNewMacro(TriangleThresholdingTool3D);

      /* icon stuff */
      virtual const char** GetXPM() const;
      virtual const char* GetName() const;
      us::ModuleResource GetIconResource() const;

      /// \brief Executes the tool.
      void Run();

    protected:
      TriangleThresholdingTool3D();
      virtual ~TriangleThresholdingTool3D();

      template < typename TPixel, unsigned int VDimension >
      void InternalRun( itk::Image<TPixel, VDimension>* input );

  };

}//namespace

#endif //mitkTriangleThresholdingTool3D_H
