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
#ifndef MITKMORPHOLOGICTOOL_H
#define MITKMORPHOLOGICTOOL_H

#include "mitkTool.h"
#include "MitkExports.h"

namespace mitk
{
  /**
  \brief Superclass for tools using morphologic operations to modify a segmentation
  */
  class MitkExt_EXPORT MorphologicTool : public Tool
  {
  public:

    mitkClassMacro(MorphologicTool, Tool);
    const char* GetGroup() const;

    itkGetMacro(Radius, unsigned int);
    void SetRadius(unsigned int);

    itkGetMacro(Preview, bool);
    itkSetMacro(Preview, bool);
    itkBooleanMacro(Preview);

    void SetStructuringElementType(int id);
    virtual void Activated();
    virtual void Deactivated();
    void AcceptPreview(const std::string& name, const Color& color);
    virtual void CancelPreviewing();

  protected:

    MorphologicTool();
    MorphologicTool(const char*);
    virtual ~MorphologicTool();

    virtual void UpdatePreview();
    virtual mitk::Image::Pointer ApplyFilter(mitk::Image::Pointer image);

    unsigned int m_Radius;
    bool m_Preview;

    enum StructuringElementType{BALL, CROSS};
    StructuringElementType m_StructElement;

    mitk::DataNode::Pointer m_FeedbackNode;
    mitk::DataNode::Pointer m_NodeToProceed;
    mitk::DataNode::Pointer m_OriginalNode;
  };//class
}//namespace

#endif
