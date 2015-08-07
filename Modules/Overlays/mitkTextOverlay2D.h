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

#ifndef TEXTOVERLAY2D_H
#define TEXTOVERLAY2D_H

#include <mitkVtkOverlay2D.h>
#include <mitkLocalStorageHandler.h>
#include <vtkSmartPointer.h>
#include "MitkOverlaysExports.h"

class vtkTextActor;
class vtkPropAssembly;

namespace mitk {

  enum TextOrientation
  {
    TextRigth = 0,
    TextCenterBottom,
    TextCenterTop,
    TextLeft
  };

/** \brief Displays text on the renderwindow */
class MITKOVERLAYS_EXPORT TextOverlay2D : public mitk::VtkOverlay2D {
public:

  class LocalStorage : public mitk::Overlay::BaseLocalStorage
  {
  public:
    /** \brief Actor of a 2D render window. */
    vtkSmartPointer<vtkTextActor> m_TextActor;

    vtkSmartPointer<vtkTextProperty> m_TextProp;

    vtkSmartPointer<vtkTextActor> m_STextActor;

    vtkSmartPointer<vtkTextProperty> m_STextProp;

    vtkSmartPointer<vtkPropAssembly> m_Assembly;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };

  mitkClassMacro(TextOverlay2D, mitk::VtkOverlay2D);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual Overlay::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const override;
  virtual void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds& bounds) override;

  void SetOrientation(const TextOrientation& orientation);

protected:

  /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
  mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

  vtkProp* GetVtkProp(BaseRenderer *renderer) const override;
  virtual vtkActor2D* GetVtkActor2D(BaseRenderer *renderer) const override;
  void UpdateVtkOverlay2D(mitk::BaseRenderer *renderer) override;

  /** \brief explicit constructor which disallows implicit conversions */
  explicit TextOverlay2D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~TextOverlay2D();

private:

  TextOrientation m_Orientation;

  /** \brief copy constructor */
  TextOverlay2D( const TextOverlay2D &);

  /** \brief assignment operator */
  TextOverlay2D &operator=(const TextOverlay2D &);

  void calculateTextPosWithOffset(float& x, float& y);
  void applyTextOrientation(vtkSmartPointer<vtkTextProperty>& textProperty, mitk::TextOrientation& orientation);
};

} // namespace mitk
#endif // TEXTOVERLAY2D_H


