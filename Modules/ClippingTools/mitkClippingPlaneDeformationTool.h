/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: $
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkClippingPlaneDeformationTool_h_Included
#define mitkClippingPlaneDeformationTool_h_Included

#include "ClippingToolsExports.h"

#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkSurfaceDeformationInteractor3D.h"
#include "mitkTool.h"
#include "mitkVector.h"

namespace mitk
{
  /**
  \brief A tool with whom you can deform planes.
  */
  class ClippingTools_EXPORT ClippingPlaneDeformationTool : public Tool
  {
  public:

    mitkClassMacro(ClippingPlaneDeformationTool, Tool);
    itkNewMacro(ClippingPlaneDeformationTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual const char* GetGroup() const;


  protected:

    ClippingPlaneDeformationTool(); // purposely hidden
    virtual ~ClippingPlaneDeformationTool();

    virtual void Activated();
    virtual void Deactivated();

    mitk::DataNode::Pointer                       m_ClippingPlaneNode;
    mitk::SurfaceDeformationInteractor3D::Pointer m_SurfaceInteractor;  

  private:

    void ClippingPlaneChanged();
  };

} //end namespace mitk

#endif


