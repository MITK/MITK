
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


#ifndef _MITK_ParticleGrid_H
#define _MITK_ParticleGrid_H

#include "mitkBaseData.h"
#include "mitkParticle.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk {

  /**
   * \brief 3D grid conatiner for fiber Particles;   */
  class  MitkDiffusionImaging_EXPORT ParticleGrid : public BaseData
  {
  public:

    mitkClassMacro( ParticleGrid, BaseData );
    itkNewMacro( Self );

    // grid types
    typedef itk::VectorContainer< int, mitk::Particle* > ParticleContainerType;
    typedef itk::VectorContainer< int, ParticleContainerType::Pointer > ParticleGridType;

    void AddParticle(mitk::Particle* particle);
    mitk::Particle* GetParticle(int x, int y, int z, int pos);
    ParticleContainerType::Pointer GetParticleContainer(int x, int y, int z);
    ParticleContainerType::Pointer GetParticleContainer(int index);

    itkSetMacro(ParticleWeight, float);
    itkGetMacro(ParticleWeight, float);

    itkGetMacro(ParticleList, ParticleContainerType::Pointer);

    void SetSize(int size[3]);
    int GetNumParticles();

    // virtual methods that need to be implemented
    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion( itk::DataObject *data );

  protected:
    ParticleGrid();
    virtual ~ParticleGrid();

    ParticleGridType::Pointer m_ParticleGrid;
    ParticleContainerType::Pointer m_ParticleList;
    int m_Size[3];

    float m_ParticleWeight;
  };

} // namespace mitk

#endif /*  _MITK_ParticleGrid_H */
