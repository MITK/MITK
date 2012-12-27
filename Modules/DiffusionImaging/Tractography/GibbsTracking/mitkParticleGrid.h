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

#ifndef _PARTICLEGRID
#define _PARTICLEGRID

// MITK
#include "MitkDiffusionImagingExports.h"
#include <mitkParticle.h>

// ITK
#include <itkImage.h>

namespace mitk
{

/**
* \brief Contains and manages particles.   */

class MitkDiffusionImaging_EXPORT ParticleGrid
{

public:

    typedef itk::Image< float, 3 >  ItkFloatImageType;

    int m_NumParticles;         // number of particles
    int m_NumConnections;       // number of connections
    int m_NumCellOverflows;     // number of cell overflows
    float m_ParticleLength;

    ParticleGrid(ItkFloatImageType* image, float particleLength, int cellCapacity);
    ~ParticleGrid();

    Particle* GetParticle(int ID);

    Particle* NewParticle(vnl_vector_fixed<float, 3> R);
    bool TryUpdateGrid(int k);
    void RemoveParticle(int k);

    void ComputeNeighbors(vnl_vector_fixed<float, 3> &R);
    Particle* GetNextNeighbor();

    void CreateConnection(Particle *P1,int ep1, Particle *P2, int ep2);
    void DestroyConnection(Particle *P1,int ep1, Particle *P2, int ep2);
    void DestroyConnection(Particle *P1,int ep1);

    bool CheckConsistency();
    void ResetGrid();

protected:

    bool ReallocateGrid();

    std::vector< Particle* >    m_Grid;             // the grid
    std::vector< Particle >     m_Particles;        // particle container
    std::vector< int >          m_OccupationCount;  // number of particles per grid cell

    int m_ContainerCapacity;     // maximal number of particles

    vnl_vector_fixed< int, 3 >      m_GridSize;     // grid dimensions
    vnl_vector_fixed< float, 3 >    m_GridScale;    // scaling factor for grid

    int m_CellCapacity;      // particle capacity of single cell in grid

    struct NeighborTracker  // to run over the neighbors
    {
        std::vector< int > cellidx;
        std::vector< int > cellidx_c;
        int cellcnt;
        int pcnt;
    } m_NeighbourTracker;

};

class MitkDiffusionImaging_EXPORT Track
{
public:
    std::vector< EndPoint > track;
    float   m_Energy;
    float   m_Probability;
    int     m_Length;

    Track()
    {
        track.resize(1000);
    }

    ~Track(){}

    void clear()
    {
        m_Length = 0;
        m_Energy = 0;
        m_Probability = 1;
    }

    bool isequal(Track& t)
    {
        for (int i = 0; i < m_Length;i++)
        {
            if (track[i].p != t.track[i].p || track[i].ep != t.track[i].ep)
                return false;
        }
        return true;
    }
};

}

#endif
