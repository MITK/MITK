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

#include "mitkParticle.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk
{

class MitkDiffusionImaging_EXPORT ParticleGrid
{

    //////////////// Container
public:
    Particle*   m_Particles;    // particles in linear array
    Particle**  m_AddressContainer;

    int m_NumParticles;         // number of particles
    int m_NumConnections;       // number of connections
    int m_NumCellOverflows;     // number of cell overflows

    ParticleGrid();
    ~ParticleGrid();

    float GetMemoryUsage();
    int allocate(int _capacity, int _nx, int _ny, int _nz, float cellsize, int cellcapacity);
    int reallocate();

    int ID_2_index(int ID);

    Particle* newParticle(vnl_vector_fixed<float, 3> R);
    bool TryUpdateGrid(int k);
    void RemoveParticle(int k);

    void ComputeNeighbors(vnl_vector_fixed<float, 3> &R);
    Particle* GetNextNeighbor();

    void CreateConnection(Particle *P1,int ep1, Particle *P2, int ep2);
    void DestroyConnection(Particle *P1,int ep1, Particle *P2, int ep2);
    void DestroyConnection(Particle *P1,int ep1);

private:

    int capacity; // maximal number of particles
    int increase_step;

    Particle **grid;   // the grid

    // grid size
    int nx;
    int ny;
    int nz;

    // scaling factor for grid
    float mulx;
    float muly;
    float mulz;

    int csize;      // particle capacity of single cell in grid
    int *occnt;     // occupation count of grid cells
    int gridsize;   // total number of cells
    float m_Memory;

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
