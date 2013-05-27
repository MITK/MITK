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
#include <vtkSmartPointer.h>
#include "mitkOverlay.h"
#include "mitkBaseRenderer.h"

namespace mitk {

class MITK_CORE_EXPORT OverlayManager : public itk::Object {
public:

  mitkClassMacro(OverlayManager, itk::Object);
  itkNewMacro(OverlayManager);

  void AddOverlay(Overlay::Pointer overlay);
  void PrepareOverlays(BaseRenderer *baseRenderer);

  /** \brief Base class for mapper specific rendering ressources.
   */
  class MITK_CORE_EXPORT LocalStorage
  {
  public:

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();

    bool IsGenerateDataRequired(mitk::BaseRenderer *renderer,mitk::OverlayManager *overlaymanager);

    inline void UpdateGenerateDataTime()
    {
      m_LastGenerateDataTime.Modified();
    }

    inline itk::TimeStamp & GetLastGenerateDataTime() { return m_LastGenerateDataTime; }

  protected:

    /** \brief timestamp of last update of stored data */
    itk::TimeStamp m_LastGenerateDataTime;

  };

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayManager();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~OverlayManager();

private:

  std::vector<Overlay::Pointer> m_OverlayList;

  /** \brief copy constructor */
  OverlayManager( const OverlayManager &);

  /** \brief assignment operator */
  OverlayManager &operator=(const OverlayManager &);

};

} // namespace mitk
#endif // OVERLAYMANAGER_H


