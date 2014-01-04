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
#ifndef mitkAutoThresholdTool3D_H
#define mitkAutoThresholdTool3D_H

#include "mitkSegTool3D.h"
#include "SegmentationExports.h"

#include "itkImage.h"

namespace us {
class ModuleResource;
}

namespace mitk{
  class Segmentation_EXPORT AutoThresholdTool3D : public SegTool3D
  {
    public:

      mitkClassMacro(AutoThresholdTool3D, SegTool3D);
      itkNewMacro(AutoThresholdTool3D);

      /* icon stuff */
      virtual const char** GetXPM() const;
      virtual const char* GetName() const;
      us::ModuleResource GetIconResource() const;

      enum AutoThresholdType
      {
        AT_HUANG,
        AT_INTERMODES,
        AT_ISODATA,
        AT_LI,
        AT_MAXENTROPY,
        AT_MINERROR,
        AT_MOMENTS,
        AT_OTSU,
        AT_RENYIENTROPY,
        AT_SHANBHAG,
        AT_TRIANGLE,
        AT_YEN
      };

      /// \brief Executes the tool.
      void Run();

      /// \brief Sets the thresholding algorithm to use.
      void SetMethod(AutoThresholdType value);

    protected:
      AutoThresholdTool3D();
      virtual ~AutoThresholdTool3D();

      AutoThresholdType m_ActiveMethod;

      template < typename TPixel, unsigned int VDimension >
      void InternalRun( itk::Image<TPixel, VDimension>* input );

  };

}//namespace

#endif //mitkAutoThresholdTool3D_H
