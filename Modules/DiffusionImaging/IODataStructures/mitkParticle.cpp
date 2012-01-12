/*=========================================================================
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "mitkParticle.h"

namespace mitk
{
  Particle::Particle()
  {

  }

  Particle::~Particle()
  {

  }

  void Particle::SetDirection(VectorType dir)
  {
    this->m_Direction = dir;
  }

  void Particle::SetPosition(VectorType pos)
  {
    this->m_Position = pos;
  }

   Particle::VectorType Particle::GetDirection()
  {
    return this->m_Direction;
  }

   Particle::VectorType Particle::GetPosition()
  {
    return this->m_Position;
  }
}
