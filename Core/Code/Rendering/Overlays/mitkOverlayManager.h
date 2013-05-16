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

#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "MitkExports.h"
#include <itkObject.h>
#include <mitkLocalStorageHandler.h>
#include <vtkSmartPointer.h>
#include <mitkBaseRenderer.h>

namespace mitk {

class OverlayManager : public itk::Object {
public:

//  /** \brief Internal class holding the mapper, actor, etc. for each of the 3 2D render windows */
//  /**
//       * To render transveral, coronal, and sagittal, the mapper is called three times.
//       * For performance reasons, the corresponding data for each view is saved in the
//       * internal helper class LocalStorage. This allows rendering n views with just
//       * 1 mitkMapper using n vtkMapper.
//       * */
//  class LocalStorage
//  {

//    bool IsGenerateDataRequired(mitk::BaseRenderer *renderer,mitk::Mapper *mapper,mitk::DataNode *dataNode);

//    inline void UpdateGenerateDataTime()
//    {
//      m_LastGenerateDataTime.Modified();
//    }

//    inline itk::TimeStamp & GetLastGenerateDataTime() { return m_LastGenerateDataTime; }

//  public:
//    /** \brief Actor of a 2D render window. */
//    vtkSmartPointer<vtkActor> m_Actor;

//    /** \brief Timestamp of last update of stored data. */
//    itk::TimeStamp m_LastUpdateTime;

//    /** \brief Default constructor of the local storage. */
//    LocalStorage();
//    /** \brief Default deconstructor of the local storage. */
//    ~LocalStorage();

//  protected:

//    /** \brief timestamp of last update of stored data */
//    itk::TimeStamp m_LastGenerateDataTime;
//  };

  mitkClassMacro(OverlayManager, itk::Object);
  itkNewMacro(OverlayManager);

//  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
//  mitk::LocalStorageHandler<LocalStorage> m_LSH;

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayManager();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~OverlayManager();

private:

  /** \brief copy constructor */
  OverlayManager( const OverlayManager &);

  /** \brief assignment operator */
  OverlayManager &operator=(const OverlayManager &);

};

} // namespace mitk
#endif // OVERLAYMANAGER_H


