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

#ifndef SCALELEGENDOVERLAY_H
#define SCALELEGENDOVERLAY_H

#include <mitkVtkOverlay.h>
#include <mitkLocalStorageHandler.h>
#include <vtkSmartPointer.h>
#include "MitkOverlaysExports.h"

class vtkLegendScaleActor;

namespace mitk {

/** \brief Displays configurable scales on the renderwindow. The scale is determined by the image spacing. */
class MitkOverlays_EXPORT ScaleLegendOverlay : public mitk::VtkOverlay {
public:

  class LocalStorage : public mitk::Overlay::BaseLocalStorage
  {
  public:
    /** \brief Actor of a 2D render window. */
    vtkSmartPointer<vtkLegendScaleActor> m_legendScaleActor;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };

  mitkClassMacro(ScaleLegendOverlay, mitk::VtkOverlay);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void SetRightAxisVisibility(bool visibility);
  bool GetRightAxisVisibility() const;

  void SetLeftAxisVisibility(bool visibility);
  bool GetLeftAxisVisibility() const;

  void SetTopAxisVisibility(bool visibility);
  bool GetTopAxisVisibility() const;

  void SetBottomAxisVisibility(bool visibility);
  bool GetBottomAxisVisibility() const;

  void SetLegendVisibility(bool visibility);
  bool GetLegendVisibility() const;

  void SetRightBorderOffset(int offset);
  int GetRightBorderOffset() const;

  void SetCornerOffsetFactor(float offsetFactor);
  float GetCornerOffsetFactor() const;

protected:

  /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
  mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

  virtual vtkProp* GetVtkProp(BaseRenderer *renderer) const;
  virtual void UpdateVtkOverlay(BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit ScaleLegendOverlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~ScaleLegendOverlay();

private:

  /** \brief copy constructor */
  ScaleLegendOverlay( const ScaleLegendOverlay &);

  /** \brief assignment operator */
  ScaleLegendOverlay &operator=(const ScaleLegendOverlay &);

};

} // namespace mitk
#endif // SCALELEGENDOVERLAY_H


