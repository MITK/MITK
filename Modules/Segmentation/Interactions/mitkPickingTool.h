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
#include <MitkSegmentationExports.h>
#include "mitkAutoSegmentationTool.h"
#include "mitkDataStorage.h"
#include "mitkSinglePointDataInteractor.h"
#include "mitkPointSet.h"
#include "itkImage.h"

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief Extracts a single region from a segmentation image and creates a new image with same geometry of the input image.

  The region is extracted in 3D space. This is done by performing region growing within the desired region.
  Use shift click to add the seed point.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MitkSegmentation_EXPORT PickingTool : public AutoSegmentationTool
  {
  public:

    mitkClassMacro(PickingTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;
    us::ModuleResource GetIconResource() const;


    virtual void Activated();
    virtual void Deactivated();

    virtual DataNode::Pointer GetPointSetNode();

    mitk::DataNode* GetReferenceData();
    mitk::DataNode* GetWorkingData();
    mitk::DataStorage* GetDataStorage();

    void ConfirmSegmentation();


  protected:

    PickingTool(); // purposely hidden
    virtual ~PickingTool();

    //Callback for point add event of PointSet
    void OnPointAdded();

    //Observer id
    long m_PointSetAddObserverTag;

    mitk::DataNode::Pointer m_ResultNode;

    //itk regrowing
    template < typename TPixel, unsigned int VImageDimension >
    void StartRegionGrowing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::BaseGeometry* imageGeometry, mitk::PointSet::PointType seedPoint );

    //seed point
    PointSet::Pointer m_PointSet;
    SinglePointDataInteractor::Pointer m_SeedPointInteractor;
    DataNode::Pointer m_PointSetNode;

    DataNode* m_WorkingData;
  };

} // namespace

#endif
