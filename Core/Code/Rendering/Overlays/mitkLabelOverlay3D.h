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

#include "mitkVtkOverlay3D.h"
#include <mitkLocalStorageHandler.h>
#include <vtkStringArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <mitkPointSet.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkIntArray.h>


namespace mitk {

class MITK_CORE_EXPORT LabelOverlay3D : public mitk::VtkOverlay3D {
public:

  /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
  /**
     * To render transveral, coronal, and sagittal, the mapper is called three times.
     * For performance reasons, the corresponding data for each view is saved in the
     * internal helper class LocalStorage. This allows rendering n views with just
     * 1 mitkMapper using n vtkMapper.
     * */
  class MITK_CORE_EXPORT LocalStorage : public mitk::Overlay::BaseLocalStorage
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

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

  mitkClassMacro(LabelOverlay3D, mitk::VtkOverlay3D);
  itkNewMacro(LabelOverlay3D);

  void SetLabelVector(std::vector<const char *> LabelVector);
  void SetPriorityVector(std::vector<int> PriorityVector);
  void SetLabelCoordinates(mitk::PointSet::Pointer LabelCoordinates);

protected:

  virtual vtkSmartPointer<vtkProp> GetVtkProp(BaseRenderer *renderer);
  void UpdateVtkOverlay(mitk::BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit LabelOverlay3D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~LabelOverlay3D();

private:

  std::vector<const char*> m_LabelVector;
  std::vector<int> m_PriorityVector;
  mitk::PointSet::Pointer m_LabelCoordinates;

  /** \brief copy constructor */
  LabelOverlay3D( const LabelOverlay3D &);

  /** \brief assignment operator */
  LabelOverlay3D &operator=(const LabelOverlay3D &);

};

} // namespace mitk
#endif // LabelOverlay3D_H


