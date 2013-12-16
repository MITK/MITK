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

#ifndef mitkPickingTool_h_Included
#define mitkPickingTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkDataStorage.h"
#include "mitkPointSetInteractor.h"
#include "mitkPointSet.h"

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class Segmentation_EXPORT PickingTool : public AutoSegmentationTool
  {
  public:

    mitkClassMacro(PickingTool, AutoSegmentationTool);
    itkNewMacro(PickingTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;


    virtual void Activated();
    virtual void Deactivated();

    virtual DataNode::Pointer GetPointSetNode();

    mitk::DataNode* GetReferenceData();
    mitk::DataNode* GetWorkingData();
    mitk::DataStorage* GetDataStorage();


  protected:

    PickingTool(); // purposely hidden
    virtual ~PickingTool();

    void OnPointAdded();
    long m_PointSetAddObserverTag;

  private:
    PointSet::Pointer m_PointSet;
    PointSetInteractor::Pointer m_SeedPointInteractor;
    DataNode::Pointer m_PointSetNode;


  };

} // namespace

#endif
