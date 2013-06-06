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

#ifndef TextOverlay3D_H
#define TextOverlay3D_H

#include "mitkVtkOverlay.h"
#include <mitkLocalStorageHandler.h>
#include <vtkFollower.h>
#include <vtkVectorText.h>
#include <vtkTextActor3D.h>


namespace mitk {

class MITK_CORE_EXPORT TextOverlay3D : public mitk::VtkOverlay {
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
    /** \brief Actor of a 2D render window. */
    vtkSmartPointer<vtkFollower> m_follower;

    vtkSmartPointer<vtkVectorText> m_textSource;


    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();

  };

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

  mitkClassMacro(TextOverlay3D, mitk::VtkOverlay);
  itkNewMacro(TextOverlay3D);

  void setPosition3D(Point3D position3D);

  void setText(std::string text);

  void setPosition3D(Point3D position3D, mitk::BaseRenderer* renderer);

  Point3D getPosition3D();

  std::string getText();

  Point3D getPosition3D(mitk::BaseRenderer* renderer);

protected:

  virtual vtkSmartPointer<vtkActor> getVtkActor(BaseRenderer *renderer);

  /** \brief explicit constructor which disallows implicit conversions */
  explicit TextOverlay3D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~TextOverlay3D();

private:

  /** \brief copy constructor */
  TextOverlay3D( const TextOverlay3D &);

  /** \brief assignment operator */
  TextOverlay3D &operator=(const TextOverlay3D &);

};

} // namespace mitk
#endif // TextOverlay3D_H


