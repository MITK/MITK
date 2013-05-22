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

#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"

namespace mitk{
  class Segmentation_EXPORT OtsuTool3D : public AutoSegmentationTool
  {
    public:

    mitkClassMacro(OtsuTool3D, AutoSegmentationTool);
    itkNewMacro(OtsuTool3D);

      virtual const char* GetName() const;
      virtual const char** GetXPM() const;

      virtual void Activated();
      virtual void Deactivated();

      void RunSegmentation( int regions);

    protected:
      OtsuTool3D();
      virtual ~OtsuTool3D();

      mitk::DataNode::Pointer m_OriginalImageNode;

  };//class
}//namespace
#endif