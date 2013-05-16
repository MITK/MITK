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

#ifndef TEXTOVERLAY_H
#define TEXTOVERLAY_H

#include "mitkOverlay.h"
#include <mitkLocalStorageHandler.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>


namespace mitk {

class TextOverlay : public mitk::Overlay {
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
    vtkSmartPointer<vtkTextActor> m_textActor;

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();
  };

  mitkClassMacro(TextOverlay, mitk::Overlay);

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit TextOverlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~TextOverlay();

private:

  /** \brief copy constructor */
  TextOverlay( const TextOverlay &);

  /** \brief assignment operator */
  TextOverlay &operator=(const TextOverlay &);

};

} // namespace mitk
#endif // TEXTOVERLAY_H


