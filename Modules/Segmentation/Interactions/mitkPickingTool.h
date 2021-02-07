/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPickingTool_h_Included
#define mitkPickingTool_h_Included

#include "itkImage.h"
#include "mitkAutoSegmentationTool.h"
#include "mitkCommon.h"
#include "mitkDataStorage.h"
#include "mitkPointSet.h"
#include "mitkSinglePointDataInteractor.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
  \brief Extracts a single region from a segmentation image and creates a new image with same geometry of the input
  image.

  The region is extracted in 3D space. This is done by performing region growing within the desired region.
  Use shift click to add the seed point.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT PickingTool : public AutoSegmentationTool
  {
  public:
    mitkClassMacro(PickingTool, AutoSegmentationTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const char **GetXPM() const override;
    const char *GetName() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void Deactivated() override;

    bool CanHandle(const BaseData* referenceData, const BaseData* workingData) const override;

    virtual DataNode::Pointer GetPointSetNode();

    mitk::DataNode *GetReferenceData();
    mitk::DataNode *GetWorkingData();
    mitk::DataStorage *GetDataStorage();

    void ConfirmSegmentation();

  protected:
    PickingTool(); // purposely hidden
    ~PickingTool() override;

    // Callback for point add event of PointSet
    void OnPointAdded();

    // Observer id
    long m_PointSetAddObserverTag;

    mitk::DataNode::Pointer m_ResultNode;

    // itk regrowing
    template <typename TPixel, unsigned int VImageDimension>
    void StartRegionGrowing(itk::Image<TPixel, VImageDimension> *itkImage,
                            mitk::BaseGeometry *imageGeometry,
                            mitk::PointSet::PointType seedPoint);

    // seed point
    PointSet::Pointer m_PointSet;
    SinglePointDataInteractor::Pointer m_SeedPointInteractor;
    DataNode::Pointer m_PointSetNode;

    DataNode *m_WorkingData;
  };

} // namespace

#endif
