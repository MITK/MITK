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

#ifndef OVERLAY_H
#define OVERLAY_H

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkBaseRenderer.h"


namespace mitk {

class MITK_CORE_EXPORT Overlay : public itk::Object {
public:

  mitkClassMacro(Overlay, itk::Object);
  itkNewMacro(Overlay);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit Overlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~Overlay();

private:

  /** \brief copy constructor */
  Overlay( const Overlay &);

  /** \brief assignment operator */
  Overlay &operator=(const Overlay &);

public:

  /** \brief Base class for mapper specific rendering ressources.
   */
  class MITK_CORE_EXPORT BaseLocalStorage
  {
  public:


    bool IsGenerateDataRequired(mitk::BaseRenderer *renderer,mitk::Overlay *overlay);

    inline void UpdateGenerateDataTime()
    {
      m_LastGenerateDataTime.Modified();
    }

    inline itk::TimeStamp & GetLastGenerateDataTime() { return m_LastGenerateDataTime; }

  protected:

    /** \brief timestamp of last update of stored data */
    itk::TimeStamp m_LastGenerateDataTime;

  };


};

} // namespace mitk
#endif // OVERLAY_H


