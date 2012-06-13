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
#include "mitkParticle.h"
#include "MitkDiffusionImagingExports.h"

// ITK
#include <itkImage.h>

namespace mitk
{

class MitkDiffusionImaging_EXPORT ParticleGrid
{

public:

    typedef itk::Image< float, 3 >  ItkFloatImageType;


    int m_NumParticles;         // number of particles
    int m_NumConnections;       // number of connections
    int m_NumCellOverflows;     // number of cell overflows

    ParticleGrid(ItkFloatImageType* image, float cellSize);
    ~ParticleGrid();

    Particle* GetParticle(int ID);

    Particle* newParticle(vnl_vector_fixed<float, 3> R);
    bool TryUpdateGrid(int k);
    void RemoveParticle(int k);

    void ComputeNeighbors(vnl_vector_fixed<float, 3> &R);
    Particle* GetNextNeighbor();

    void CreateConnection(Particle *P1,int ep1, Particle *P2, int ep2);
    void DestroyConnection(Particle *P1,int ep1, Particle *P2, int ep2);
    void DestroyConnection(Particle *P1,int ep1);

protected:

    int ReallocateGrid();

    int capacity; // maximal number of particles
    int increase_step;

    std::vector< Particle* > grid;   // the grid
    std::vector< Particle >  m_Particles;    // particles in linear array
    std::vector< int > occnt;     // occupation count of grid cells

    // grid size
    int nx;
    int ny;
    int nz;

    // scaling factor for grid
    float mulx;
    float muly;
    float mulz;

    int csize;      // particle capacity of single cell in grid
    int gridsize;   // total number of cells

    struct NeighborTracker  // to run over the neighbors
    {
        int cellidx[8];
        int cellidx_c[8];
        int cellcnt;
        int pcnt;
    } m_NeighbourTracker;

};

class MitkDiffusionImaging_EXPORT Track
{
public:
    EndPoint track[1000];
    float energy;
    float proposal_probability;
    int length;

    void clear()
    {
        length = 0;
        energy = 0;
        proposal_probability = 1;
    }

    bool isequal(Track &t)
    {
        for (int i = 0; i < length;i++)
        {
            if (track[i].p != t.track[i].p || track[i].ep != t.track[i].ep)
                return false;
        }
        return true;
    }
};

}

#endif
