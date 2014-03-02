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
#ifndef MITKOTSUTOOL3D_H
#define MITKOTSUTOOL3D_H

#include <MitkSegmentationExports.h>
#include "mitkAutoSegmentationTool.h"

namespace us {
class ModuleResource;
}

namespace mitk{
  class MitkSegmentation_EXPORT OtsuTool3D : public AutoSegmentationTool
  {
    public:

    mitkClassMacro(OtsuTool3D, AutoSegmentationTool);
    itkNewMacro(OtsuTool3D);

      virtual const char* GetName() const;
      virtual const char** GetXPM() const;
    us::ModuleResource GetIconResource() const;

      virtual void Activated();
      virtual void Deactivated();

      void RunSegmentation( int regions);
      void ConfirmSegmentation();
      void UpdateBinaryPreview(int regionID);
      void UpdateVolumePreview(bool volumeRendering);
      void ShowMultiLabelResultNode(bool);

    protected:
      OtsuTool3D();
      virtual ~OtsuTool3D();

      mitk::Image::Pointer m_OriginalImage;
      //holds the user selected binary segmentation
      mitk::DataNode::Pointer m_BinaryPreviewNode;
      //holds the multilabel result as a preview image
      mitk::DataNode::Pointer m_MultiLabelResultNode;
      //holds the user selected binary segmentation masked original image
      mitk::DataNode::Pointer m_MaskedImagePreviewNode;

  };//class
}//namespace
#endif
