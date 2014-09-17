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

#ifndef LOGOOVERLAY_H
#define LOGOOVERLAY_H

#include <mitkVtkOverlay.h>
#include <mitkLocalStorageHandler.h>
#include <vtkSmartPointer.h>
#include "MitkOverlaysExports.h"

class vtkLogoRepresentation;
class vtkImageData;
class vtkImageReader2Factory;

namespace mitk {

/** \brief Displays text on the renderwindow */
class MitkOverlays_EXPORT LogoOverlay : public mitk::VtkOverlay {
public:

  class LocalStorage : public mitk::Overlay::BaseLocalStorage
  {
  public:
    /** \brief Actor of a 2D render window. */
    vtkSmartPointer<vtkImageData> m_LogoImage;
    vtkSmartPointer<vtkLogoRepresentation> m_LogoRep;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };

  mitkClassMacro(LogoOverlay, mitk::VtkOverlay);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual Overlay::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const;
  virtual void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds& bounds);
  vtkSmartPointer<vtkImageReader2Factory> m_readerFactory;

  void SetLogoImagePath(std::string text);
  std::string GetLogoImagePath() const;

protected:

  /** \brief The LocalStorageHandler holds all LocalStorages for the render windows. */
  mutable mitk::LocalStorageHandler<LocalStorage> m_LSH;

  virtual vtkProp *GetVtkProp(BaseRenderer *renderer) const;
  void UpdateVtkOverlay(mitk::BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit LogoOverlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~LogoOverlay();

  vtkImageData* CreateMbiLogo();

private:

  /** \brief copy constructor */
  LogoOverlay( const LogoOverlay &);

  /** \brief assignment operator */
  LogoOverlay &operator=(const LogoOverlay &);

};

} // namespace mitk
#endif // LOGOOVERLAY_H


