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

#ifndef __mitkAxisOverlay_h
#define __mitkAxisOverlay_h

#include <mitkVtkOverlay.h>
#include <mitkLocalStorageHandler.h>

#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>

namespace mitk {
  class  AxisOverlay : public mitk::VtkOverlay {
  public:
    class LocalStorage : public mitk::VtkOverlay::BaseLocalStorage
    {
    public:
      /** \brief Actor of a 2D render window. */
      bool m_initialized = false;
      vtkSmartPointer<vtkAxesActor> m_axesActor;
      vtkSmartPointer<vtkOrientationMarkerWidget> m_widget;

      vtkSmartPointer<vtkActor> m_dummyActor;
      vtkSmartPointer<vtkPolyDataMapper> m_dummyMapper;
      vtkSmartPointer<vtkSphereSource> m_dummySource;

      /** \brief Timestamp of last update of stored data. */
      itk::TimeStamp m_LastUpdateTime;

      /** \brief Default constructor of the local storage. */
      LocalStorage();
      /** \brief Default deconstructor of the local storage. */
      ~LocalStorage();

    };

    mitkClassMacro(AxisOverlay, mitk::VtkOverlay);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected:

    /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
    mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

    virtual vtkProp* GetVtkProp(BaseRenderer *renderer) const;
    void UpdateVtkOverlay(mitk::BaseRenderer *renderer);

    /** \brief explicit constructor which disallows implicit conversions */
    explicit AxisOverlay();

    /** \brief virtual destructor in order to derive from this class */
    virtual ~AxisOverlay();

  private:

    /** \brief copy constructor */
    AxisOverlay(const AxisOverlay &);

    /** \brief assignment operator */
    AxisOverlay &operator=(const AxisOverlay &);

  };

}

#endif
