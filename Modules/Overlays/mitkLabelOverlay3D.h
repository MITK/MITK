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

#ifndef LabelOverlay3D_H
#define LabelOverlay3D_H

#include <mitkVtkOverlay3D.h>
#include <mitkLocalStorageHandler.h>
#include "MitkOverlaysExports.h"
#include <vtkSmartPointer.h>

class vtkStringArray;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkActor2D;
class vtkProperty2D;
class vtkPointSetToLabelHierarchy;
class vtkLabelPlacementMapper;
class vtkIntArray;

namespace mitk {

class PointSet;


/** \brief Can display a high amount of 3D labels to a PointSet */
class MitkOverlays_EXPORT LabelOverlay3D : public mitk::VtkOverlay3D {
public:

  /** \brief Internal class holding the vtkActor, etc. for each of the render windows */
  class LocalStorage : public mitk::Overlay::BaseLocalStorage
  {
  public:
    vtkSmartPointer<vtkPolyData> m_Points;
    vtkSmartPointer<vtkActor2D> m_LabelsActor;
    vtkSmartPointer<vtkIntArray> m_Sizes;
    vtkSmartPointer<vtkStringArray> m_Labels;
    vtkSmartPointer<vtkLabelPlacementMapper> m_LabelMapper;
    vtkSmartPointer<vtkPointSetToLabelHierarchy> m_PointSetToLabelHierarchyFilter;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();

  };

  mitkClassMacro(LabelOverlay3D, mitk::VtkOverlay3D);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** \brief Set the vector of labels that are shown to each corresponding point3D. The size has to be equal to the provided LabelCoordinates. */
  void SetLabelVector(const std::vector<std::string>& LabelVector);

  /** \brief Optional: Provide a vector of priorities. The labels with higher priorities will be visible in lower LOD */
  void SetPriorityVector(const std::vector<int>& PriorityVector);

  /** \brief Coordinates of the labels */
  void SetLabelCoordinates(itk::SmartPointer<PointSet> LabelCoordinates);

  void PointSetModified(const itk::Object *, const itk::EventObject &);
protected:

  /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
  mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

  virtual vtkProp *GetVtkProp(BaseRenderer *renderer) const;
  void UpdateVtkOverlay(mitk::BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit LabelOverlay3D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~LabelOverlay3D();

private:

  /** \brief The char arrays in this vector are displayed at the corresponding coordinates.*/
  std::vector<std::string> m_LabelVector;

  /** \brief values in this array set a priority to each label. Higher priority labels are not covert by labels with lower priority.*/
  std::vector<int> m_PriorityVector;

  /** \brief The coordinates of the labels. Indices must match the labelVector and the priorityVector.*/
  itk::SmartPointer<PointSet> m_LabelCoordinates;

  unsigned long m_PointSetModifiedObserverTag;

  /** \brief copy constructor */
  LabelOverlay3D( const LabelOverlay3D &);

  /** \brief assignment operator */
  LabelOverlay3D &operator=(const LabelOverlay3D &);

};

} // namespace mitk
#endif // LabelOverlay3D_H


