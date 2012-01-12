
/*=========================================================================
 Program:   Medical Imaging & Interaction Toolkit
 Module:    $RCSfile$
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 11989 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef _MITK_Particle_H
#define _MITK_Particle_H

#include "mitkBaseData.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk {

  /**
   * \brief Base Class for Fiber Segments;   */
  class  MitkDiffusionImaging_EXPORT Particle
  {
  public:
    Particle();
    virtual ~Particle();

    typedef vnl_vector_fixed<float, 3> VectorType;

    void SetDirection(VectorType dir);
    void SetPosition(VectorType pos);

    VectorType GetPosition();
    VectorType GetDirection();

  protected:
    VectorType m_Position;    // position
    VectorType m_Direction;   // direction
  };

} // namespace mitk

#endif /*  _MITK_Particle_H */
