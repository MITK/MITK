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
#include "SegmentationExports.h"
#include "mitkColorSequenceRainbow.h"

#include "itkImage.h"

namespace us {
class ModuleResource;
}

namespace mitk{
  class Segmentation_EXPORT OtsuTool3D : public SegTool3D
  {
    public:

      mitkClassMacro(OtsuTool3D, SegTool3D);
      itkNewMacro(OtsuTool3D);

      /* icon stuff */
      virtual const char** GetXPM() const;
      virtual const char* GetName() const;
      us::ModuleResource GetIconResource() const;

      /// \brief Replaces the active label with the preview image.
      virtual void AcceptPreview(int region);

      /// \brief Executes the tool.
      void Run();

      /// \brief Sets the number of new labels to generate.
      void SetNumberOfRegions(int);

    protected:
      OtsuTool3D();
      virtual ~OtsuTool3D();

      int m_NumberOfRegions;

      std::map< int, mitk::Image::Pointer > m_PreviewImages;
      std::map< int, mitk::DataNode::Pointer > m_PreviewNodes;
      mitk::ColorSequenceRainbow::Pointer m_ColorSequenceRainbow;

      template < typename TPixel, unsigned int VDimension >
      void InternalRun( itk::Image<TPixel, VDimension>* input );

      template <typename ImageType1, typename ImageType2>
      void InternalAcceptPreview( ImageType1* targetImage, const ImageType2* sourceImage );

  };

}//namespace

#endif //mitkOtsuTool3D_H
