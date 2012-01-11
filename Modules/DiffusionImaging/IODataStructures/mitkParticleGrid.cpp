/*=========================================================================
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "mitkParticleGrid.h"

namespace mitk
{
  ParticleGrid::ParticleGrid():
      m_ParticleWeight(0.009)
  {
    m_ParticleGrid = ParticleGridType::New();
    m_ParticleList = ParticleContainerType::New();
  }

  ParticleGrid::~ParticleGrid()
  {
    for (int i=0; i<m_ParticleList->Size(); i++)
    {
      delete(m_ParticleList->GetElement(i));
    }
  }

  int ParticleGrid::GetNumParticles()
  {
    return this->m_ParticleList->Size();
  }

  void ParticleGrid::SetSize(int size[3])
  {
    m_Size[0] = size[0];
    m_Size[1] = size[1];
    m_Size[2] = size[2];
  }

  void ParticleGrid::AddParticle(mitk::Particle* particle)
  {
    vnl_vector_fixed<float, 3> pos = particle->GetPosition();
    int x = pos[0]+0.5;
    int y = pos[1]+0.5;
    int z = pos[2]+0.5;

    if (!(x>=0 && x<m_Size[0] && y>=0 && y<m_Size[1] && z>=0 && z<m_Size[2]))
      return;

    int index = x + m_Size[0]*(y+m_Size[1]*z);

    ParticleContainerType::Pointer container;
    if (m_ParticleGrid->IndexExists(index))
    {
      container = m_ParticleGrid->GetElement(index);
      if(container.IsNull())
      {
        container = ParticleContainerType::New();
        container->InsertElement(container->Size(), particle);
        this->m_ParticleGrid->InsertElement(index, container);
      }
      else
      {
        container->InsertElement(container->Size(), particle);
      }
    }
    else
    {
      container = ParticleContainerType::New();
      container->InsertElement(container->Size(), particle);
      this->m_ParticleGrid->InsertElement(index, container);
    }

    this->m_ParticleList->InsertElement(m_ParticleList->Size(),particle);
  }

  Particle* ParticleGrid::GetParticle(int x, int y, int z, int pos)
  {
    int index = x + m_Size[0]*(y+m_Size[1]*z);

    if (!m_ParticleGrid->IndexExists(index))
      return NULL;

    ParticleContainerType::Pointer container = m_ParticleGrid->GetElement(index);

    if (!container->IndexExists(pos))
      return NULL;

    return container->GetElement(pos);
  }

  ParticleGrid::ParticleContainerType::Pointer  ParticleGrid::GetParticleContainer(int x, int y, int z)
  {
    int index = x + m_Size[0]*(y+m_Size[1]*z);

    return GetParticleContainer(index);
  }

  ParticleGrid::ParticleContainerType::Pointer  ParticleGrid::GetParticleContainer(int index)
  {
    if (!m_ParticleGrid->IndexExists(index))
      return NULL;

    return m_ParticleGrid->GetElement(index);
  }

  /* NECESSARY IMPLEMENTATION OF SUPERCLASS METHODS */
  void ParticleGrid::UpdateOutputInformation()
  {

  }
  void ParticleGrid::SetRequestedRegionToLargestPossibleRegion()
  {

  }
  bool ParticleGrid::RequestedRegionIsOutsideOfTheBufferedRegion()
  {
    return false;
  }
  bool ParticleGrid::VerifyRequestedRegion()
  {
    return false;
  }
  void ParticleGrid::SetRequestedRegion( itk::DataObject *data )
  {

  }
}
