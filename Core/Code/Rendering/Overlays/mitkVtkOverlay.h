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

#ifndef VTKOVERLAY_H
#define VTKOVERLAY_H

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkOverlay.h"
#include "mitkBaseRenderer.h"


namespace mitk {

class MITK_CORE_EXPORT VtkOverlay : public Overlay {
public:

  virtual void MitkRender(BaseRenderer *renderer) = 0;

  //##Documentation
  //## @brief Get the PropertyList of the @a renderer. If @a renderer is @a
  //## NULL, the BaseRenderer-independent PropertyList of this DataNode
  //## is returned.
  //## @sa GetProperty
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  mitk::PropertyList* GetPropertyList();

  //##Documentation
  //## @brief Get the PropertyList of the @a renderer. If @a renderer is @a
  //## NULL, the BaseRenderer-independent PropertyList of this DataNode
  //## is returned.
  //## @sa GetProperty
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  virtual mitk::PropertyList* GetBRPropertyList(mitk::BaseRenderer* renderer) = 0 ;

  mitkClassMacro(VtkOverlay, Overlay);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit VtkOverlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~VtkOverlay();

private:

  /** \brief copy constructor */
  VtkOverlay( const VtkOverlay &);

  /** \brief assignment operator */
  VtkOverlay &operator=(const VtkOverlay &);

  //##Documentation
  //## @brief BaseRenderer-independent PropertyList
  //##
  //## Properties herein can be overwritten specifically for each BaseRenderer
  //## by the BaseRenderer-specific properties defined in m_MapOfPropertyLists.
  PropertyList::Pointer m_PropertyList;

};

} // namespace mitk
#endif // OVERLAY_H


