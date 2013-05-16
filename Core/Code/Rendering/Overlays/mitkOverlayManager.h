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

namespace mitk {

class MITK_CORE_EXPORT OverlayManager : public itk::Object {
public:

  mitkClassMacro(OverlayManager, itk::Object);
  itkNewMacro(OverlayManager);

  void AddOverlay(Overlay::Pointer overlay);

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


