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

#ifndef mitkClippingPlaneTranslationTool_h_Included
#define mitkClippingPlaneTranslationTool_h_Included

#include "ClippingToolsExports.h"

#include "mitkAffineInteractor3D.h"
#include "mitkCommon.h"
#include "mitkDataNode.h"
#include "mitkTool.h"
#include "mitkVector.h"

namespace mitk
{

  /**
  \brief A tool which allows you to move planes.
  */
  class ClippingTools_EXPORT ClippingPlaneTranslationTool : public Tool
  {
  public:

    mitkClassMacro(ClippingPlaneTranslationTool, Tool);
    itkNewMacro(ClippingPlaneTranslationTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    virtual const char* GetGroup() const;


  protected:

    ClippingPlaneTranslationTool(); // purposely hidden
    virtual ~ClippingPlaneTranslationTool();

    virtual void Activated();
    virtual void Deactivated();

    mitk::DataNode::Pointer           m_ClippingPlaneNode;
    mitk::AffineInteractor3D::Pointer m_AffineInteractor;

  private:

    void ClippingPlaneChanged();
  };

} //end namespace mitk

#endif


