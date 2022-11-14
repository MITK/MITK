/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCROSSHAIRMANAGER_H
#define MITKCROSSHAIRMANAGER_H

#include <MitkCoreExports.h>

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkPlaneGeometry.h>
#include <mitkTimeGeometry.h>

#include <itkCommand.h>
#include <itkObject.h>

namespace mitk
{
  /**
   * \brief The CrosshairManager takes care of the correct settings for the plane geometries
   *        that form the crosshair.
   * 
   * The CrosshairManager keeps track of a TimeGeometry, which is used to compute
   * the three different plane geometries (axial, coronal, sagittal).
   * These three plane geometries can be added to / removed from the data storage
   * using this CrosshairManager. Adding / removing them to / from the data storage
   * will change the rendering behavior for the crosshair - only data nodes
   * inside the data storage will be rendered.
   * 
   * 
   */
  class MITKCORE_EXPORT CrosshairManager : public itk::Object
  {
  public:

    mitkClassMacroItkParent(CrosshairManager, itk::Object);
    mitkNewMacro2Param(Self, DataStorage*, BaseRenderer*);

    itkSetObjectMacro(DataStorage, DataStorage);
    itkSetObjectMacro(BaseRenderer, BaseRenderer);

    /**
     * \brief Set the input time geometry out of which the oriented geometries will be created.
     */
    void ComputeOrientedTimeGeometries(const TimeGeometry* geometry);
    
    void SetCrosshairPosition(const Point3D& selectedPoint);
    Point3D GetCrosshairPosition() const;

    void UpdateSlice(const SliceNavigationController* sliceNavigationController);

    void SetCrosshairVisibility(bool visible);
    bool GetCrosshairVisibility() const;
    void SetCrosshairGap(unsigned int gapSize);

    void AddPlanesToDataStorage();
    void RemovePlanesFromDataStorage();

  protected:

    CrosshairManager(DataStorage* dataStorage, BaseRenderer* baseRenderer);
    ~CrosshairManager();

    void InitializePlaneProperties(DataNode::Pointer planeNode, const std::string& planeName);
    void InitializePlaneData(DataNode::Pointer planeNode, const TimeGeometry* timeGeometry, unsigned int& slice);
    void UpdatePlaneSlice(DataNode::Pointer planeNode, const TimeGeometry* timeGeometry, unsigned int slice);
    void SetCrosshairPosition(const Point3D& selectedPoint,
                              DataNode::Pointer planeNode,
                              const TimeGeometry* timeGeometry,
                              unsigned int& slice);
    void AddPlaneToDataStorage(DataNode::Pointer planeNode, DataNode::Pointer parent);

    DataStorage* m_DataStorage;
    BaseRenderer* m_BaseRenderer;

    TimeGeometry::ConstPointer m_InputTimeGeometry;
    TimeGeometry::Pointer m_AxialTimeGeometry;
    TimeGeometry::Pointer m_CoronalTimeGeometry;
    TimeGeometry::Pointer m_SagittalTimeGeometry;

    unsigned int m_AxialSlice;
    unsigned int m_CoronalSlice;
    unsigned int m_SagittalSlice;

    /**
    * @brief The 3 helper objects which contain the plane geometry.
    */
    DataNode::Pointer m_AxialPlaneNode;
    DataNode::Pointer m_CoronalPlaneNode;
    DataNode::Pointer m_SagittalPlaneNode;
    DataNode::Pointer m_ParentNodeForGeometryPlanes;

  };
} // namespace mitk

#endif // MITKCROSSHAIRMANAGER_H
