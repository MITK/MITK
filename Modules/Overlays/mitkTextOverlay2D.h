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
class vtkImageMapper;
class vtkImageData;

namespace mitk {

/** \brief Displays text on the renderwindow */
class MitkOverlays_EXPORT TextOverlay2D : public mitk::VtkOverlay2D {
public:

  class LocalStorage : public mitk::Overlay::BaseLocalStorage
  {
  public:
    /** \brief Actor of a 2D render window. */
    vtkSmartPointer<vtkActor2D> m_textActor;
    vtkSmartPointer<vtkImageData> m_textImage;
    vtkSmartPointer<vtkImageMapper> m_imageMapper;

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

  virtual Overlay::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const;
  virtual void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds& bounds);

protected:

  /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
  mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

  virtual vtkActor2D* GetVtkActor2D(BaseRenderer *renderer) const;
  void UpdateVtkOverlay2D(mitk::BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit TextOverlay2D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~TextOverlay2D();

private:

  /** \brief copy constructor */
  TextOverlay2D( const TextOverlay2D &);

  /** \brief assignment operator */
  TextOverlay2D &operator=(const TextOverlay2D &);

};

} // namespace mitk
#endif // TEXTOVERLAY2D_H


