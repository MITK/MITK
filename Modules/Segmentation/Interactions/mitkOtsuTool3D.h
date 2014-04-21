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
#ifndef mitkOtsuTool3D_H
#define mitkOtsuTool3D_H

#include "mitkSegTool3D.h"
#include "MitkSegmentationExports.h"

#include "itkImage.h"

namespace us {
class ModuleResource;
}

namespace mitk{
  class MitkSegmentation_EXPORT OtsuTool3D : public SegTool3D
  {
    public:

      mitkClassMacro(OtsuTool3D, SegTool3D);
      itkNewMacro(OtsuTool3D);

      /* icon stuff */
      virtual const char** GetXPM() const;
      virtual const char* GetName() const;
      us::ModuleResource GetIconResource() const;

      /// \brief Adds actions related to multi label preview image and node
      virtual void Activated();

      /// \brief Adds actions related to multi label preview image and node
      virtual void Deactivated();

      /// \brief Updates the preview image with the current region selection.
      void UpdatePreview(int region);

      /// \brief Executes the tool.
      void Run();

      /// \brief Sets\Gets the number of regions to extract.
      void SetNumberOfRegions(int);
      int GetNumberOfRegions();

    protected:
      OtsuTool3D();
      virtual ~OtsuTool3D();

      int m_NumberOfRegions;

      mitk::DataNode::Pointer m_MultiLabelNode;
      mitk::Image::Pointer m_MultiLabelImage;

      template < typename TPixel, unsigned int VDimension >
      void InternalRun( itk::Image<TPixel, VDimension>* input );

      template <typename ImageType1, typename ImageType2>
      void InternalAcceptPreview( ImageType1* targetImage, const ImageType2* sourceImage );

      template <typename ImageType>
      void InternalUpdatePreview( const ImageType* input, int region);

  };

}//namespace

#endif //mitkOtsuTool3D_H
