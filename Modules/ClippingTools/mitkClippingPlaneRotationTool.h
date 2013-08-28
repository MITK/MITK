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

#ifndef mitkClippingPlaneRotationTool_h_Included
#define mitkClippingPlaneRotationTool_h_Included

#include "ClippingToolsExports.h"

#include "mitkAffineDataInteractor3D.h"
#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkTool.h"

namespace mitk
{

  /**
  \brief A tool which allows you to rotate planes.
  */
  class ClippingTools_EXPORT ClippingPlaneRotationTool : public Tool
  {
  public:

    mitkClassMacro(ClippingPlaneRotationTool, Tool);
    itkNewMacro(ClippingPlaneRotationTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual const char* GetGroup() const;


  protected:

    ClippingPlaneRotationTool(); // purposely hidden
    virtual ~ClippingPlaneRotationTool();

    virtual void Activated();
    virtual void Deactivated();

    mitk::DataNode::Pointer               m_ClippingPlaneNode;
    mitk::AffineDataInteractor3D::Pointer m_AffineDataInteractor;

  private:

    void ClippingPlaneChanged();
  };

} //end namespace mitk

#endif


