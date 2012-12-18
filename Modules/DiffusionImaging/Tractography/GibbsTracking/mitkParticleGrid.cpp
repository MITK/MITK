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


#include "mitkParticleGrid.h"
#include <stdlib.h>
#include <stdio.h>

using namespace mitk;

ParticleGrid::ParticleGrid(ItkFloatImageType* image, float particleLength, int cellCapacity)
{
    // initialize counters
    m_NumParticles = 0;
    m_NumConnections = 0;
    m_NumCellOverflows = 0;
    m_ParticleLength = particleLength;

    // define isotropic grid from voxel spacing and particle length
    float cellSize = 2*m_ParticleLength;
    m_GridSize[0] = image->GetLargestPossibleRegion().GetSize()[0]*image->GetSpacing()[0]/cellSize +1;
    m_GridSize[1] = image->GetLargestPossibleRegion().GetSize()[1]*image->GetSpacing()[1]/cellSize +1;
    m_GridSize[2] = image->GetLargestPossibleRegion().GetSize()[2]*image->GetSpacing()[2]/cellSize +1;
    m_GridScale[0] = 1/cellSize;
    m_GridScale[1] = 1/cellSize;
    m_GridScale[2] = 1/cellSize;

    m_CellCapacity = cellCapacity;          // maximum number of particles per grid cell
    m_ContainerCapacity = 100000;           // initial particle container capacity
    unsigned long  numCells = m_GridSize[0]*m_GridSize[1]*m_GridSize[2];   // number of grid cells
    unsigned long  gridSize = numCells*m_CellCapacity;
    if ( itk::NumericTraits<int>::max()<gridSize )
        throw std::bad_alloc();

    m_Particles.resize(m_ContainerCapacity);        // allocate and initialize particles
    m_Grid.resize(gridSize, NULL);   // allocate and initialize particle grid
    m_OccupationCount.resize(numCells, 0);          // allocate and initialize occupation counter array
    m_NeighbourTracker.cellidx.resize(8, 0);        // allocate and initialize neighbour tracker
    m_NeighbourTracker.cellidx_c.resize(8, 0);

    for (int i = 0;i < m_ContainerCapacity;i++)     // initialize particle IDs
        m_Particles[i].ID = i;

    std::cout << "ParticleGrid: allocated " << (sizeof(Particle)*m_ContainerCapacity + sizeof(Particle*)*m_GridSize[0]*m_GridSize[1]*m_GridSize[2])/1048576 << "mb for " << m_ContainerCapacity/1000 << "k particles." << std::endl;
}

ParticleGrid::~ParticleGrid()
{

}

// remove all particles
void ParticleGrid::ResetGrid()
{
    // initialize counters
    m_NumParticles = 0;
    m_NumConnections = 0;
    m_NumCellOverflows = 0;
    m_Particles.clear();
    m_Grid.clear();
    m_OccupationCount.clear();
    m_NeighbourTracker.cellidx.clear();
    m_NeighbourTracker.cellidx_c.clear();

    int numCells = m_GridSize[0]*m_GridSize[1]*m_GridSize[2];   // number of grid cells

    m_Particles.resize(m_ContainerCapacity);        // allocate and initialize particles
    m_Grid.resize(numCells*m_CellCapacity, NULL);   // allocate and initialize particle grid
    m_OccupationCount.resize(numCells, 0);          // allocate and initialize occupation counter array
    m_NeighbourTracker.cellidx.resize(8, 0);        // allocate and initialize neighbour tracker
    m_NeighbourTracker.cellidx_c.resize(8, 0);

    for (int i = 0;i < m_ContainerCapacity;i++)     // initialize particle IDs
        m_Particles[i].ID = i;
}

bool ParticleGrid::ReallocateGrid()
{
    std::cout << "ParticleGrid: reallocating ..." << std::endl;
    int new_capacity = m_ContainerCapacity + 100000;    // increase container capacity by 100k particles
    try
    {
        m_Particles.resize(new_capacity);                   // reallocate particles

        for (int i = 0; i<m_ContainerCapacity; i++)         // update particle addresses (changed during reallocation)
            m_Grid[m_Particles[i].gridindex] = &m_Particles[i];

        for (int i = m_ContainerCapacity; i < new_capacity; i++)    // initialize IDs of ne particles
            m_Particles[i].ID = i;

        m_ContainerCapacity = new_capacity;     // update member variable
        std::cout << "ParticleGrid: allocated " << (sizeof(Particle)*m_ContainerCapacity + sizeof(Particle*)*m_GridSize[0]*m_GridSize[1]*m_GridSize[2])/1048576 << "mb for " << m_ContainerCapacity/1000 << "k particles." << std::endl;
    }
    catch(...)
    {
        std::cout << "ParticleGrid: allocation of " << (sizeof(Particle)*new_capacity + sizeof(Particle*)*m_GridSize[0]*m_GridSize[1]*m_GridSize[2])/1048576 << "mb for " << new_capacity/1000 << "k particles failed!" << std::endl;
        return false;
    }
    return true;
}

Particle* ParticleGrid::GetParticle(int ID)
{
    if (ID!=-1)
        return &m_Particles[ID];
    return NULL;
}


Particle* ParticleGrid::NewParticle(vnl_vector_fixed<float, 3> R)
{
    if (m_NumParticles >= m_ContainerCapacity)
    {
        if (!ReallocateGrid())
            return NULL;
    }

    int xint = int(R[0]*m_GridScale[0]);
    if (xint < 0)
        return NULL;
    if (xint >= m_GridSize[0])
        return NULL;
    int yint = int(R[1]*m_GridScale[1]);
    if (yint < 0)
        return NULL;
    if (yint >= m_GridSize[1])
        return NULL;
    int zint = int(R[2]*m_GridScale[2]);
    if (zint < 0)
        return NULL;
    if (zint >= m_GridSize[2])
        return NULL;

    int idx = xint + m_GridSize[0]*(yint + m_GridSize[1]*zint);
    if (m_OccupationCount[idx] < m_CellCapacity)
    {
        Particle *p = &(m_Particles[m_NumParticles]);
        p->pos = R;
        p->mID = -1;
        p->pID = -1;
        m_NumParticles++;
        p->gridindex = m_CellCapacity*idx + m_OccupationCount[idx];
        m_Grid[p->gridindex] = p;
        m_OccupationCount[idx]++;
        return p;
    }
    else
    {
        m_NumCellOverflows++;
        return NULL;
    }
}

bool ParticleGrid::TryUpdateGrid(int k)
{
    Particle* p = &(m_Particles[k]);

    int xint = int(p->pos[0]*m_GridScale[0]);
    if (xint < 0)
        return false;
    if (xint >= m_GridSize[0])
        return false;
    int yint = int(p->pos[1]*m_GridScale[1]);
    if (yint < 0)
        return false;
    if (yint >= m_GridSize[1])
        return false;
    int zint = int(p->pos[2]*m_GridScale[2]);
    if (zint < 0)
        return false;
    if (zint >= m_GridSize[2])
        return false;

    int idx = xint + m_GridSize[0]*(yint+ zint*m_GridSize[1]);
    int cellidx = p->gridindex/m_CellCapacity;
    if (idx != cellidx) // cell has changed
    {

        if (m_OccupationCount[idx] < m_CellCapacity)
        {
            // remove from old position in grid;
            int grdindex = p->gridindex;
            m_Grid[grdindex] = m_Grid[cellidx*m_CellCapacity + m_OccupationCount[cellidx]-1];
            m_Grid[grdindex]->gridindex = grdindex;
            m_OccupationCount[cellidx]--;

            // insert at new position in grid
            p->gridindex = idx*m_CellCapacity + m_OccupationCount[idx];
            m_Grid[p->gridindex] = p;
            m_OccupationCount[idx]++;
            return true;
        }
        else
        {
            m_NumCellOverflows++;
            return false;
        }
    }
    return true;
}

void ParticleGrid::RemoveParticle(int k)
{
    Particle* p = &(m_Particles[k]);
    int gridIndex = p->gridindex;
    int cellIdx = gridIndex/m_CellCapacity;
    int idx = gridIndex%m_CellCapacity;

    // remove pending connections
    if (p->mID != -1)
        DestroyConnection(p,-1);
    if (p->pID != -1)
        DestroyConnection(p,+1);

    // remove from grid
    if (idx < m_OccupationCount[cellIdx]-1)
    {
        m_Grid[gridIndex] = m_Grid[cellIdx*m_CellCapacity+m_OccupationCount[cellIdx]-1];
        m_Grid[cellIdx*m_CellCapacity+m_OccupationCount[cellIdx]-1] = NULL;
        m_Grid[gridIndex]->gridindex = gridIndex;
    }
    m_OccupationCount[cellIdx]--;

    // remove from container
    if (k < m_NumParticles-1)
    {
        Particle* last = &m_Particles[m_NumParticles-1];  // last particle

        // update connections of last particle because its index is changing
        if (last->mID!=-1)
        {
            if ( m_Particles[last->mID].mID == m_NumParticles-1 )
                m_Particles[last->mID].mID = k;
            else if ( m_Particles[last->mID].pID == m_NumParticles-1 )
                m_Particles[last->mID].pID = k;
        }
        if (last->pID!=-1)
        {
            if ( m_Particles[last->pID].mID == m_NumParticles-1 )
                m_Particles[last->pID].mID = k;
            else if ( m_Particles[last->pID].pID == m_NumParticles-1 )
                m_Particles[last->pID].pID = k;
        }

        m_Particles[k] = m_Particles[m_NumParticles-1];         // move very last particle to empty slot
        m_Particles[m_NumParticles-1].ID = m_NumParticles-1;    // update ID of removed particle to match the index
        m_Particles[k].ID = k;                                  // update ID of moved particle
        m_Grid[m_Particles[k].gridindex] = &m_Particles[k];     // update address of moved particle
    }
    m_NumParticles--;
}

void ParticleGrid::ComputeNeighbors(vnl_vector_fixed<float, 3> &R)
{
    float xfrac = R[0]*m_GridScale[0];
    float yfrac = R[1]*m_GridScale[1];
    float zfrac = R[2]*m_GridScale[2];
    int xint = int(xfrac);
    int yint = int(yfrac);
    int zint = int(zfrac);

    int dx = -1;
    if (xfrac-xint > 0.5) dx = 1;
    if (xint <= 0) { xint = 0; dx = 1; }
    if (xint >= m_GridSize[0]-1) { xint = m_GridSize[0]-1; dx = -1; }

    int dy = -1;
    if (yfrac-yint > 0.5) dy = 1;
    if (yint <= 0) {yint = 0; dy = 1; }
    if (yint >= m_GridSize[1]-1) {yint = m_GridSize[1]-1; dy = -1;}

    int dz = -1;
    if (zfrac-zint > 0.5) dz = 1;
    if (zint <= 0) {zint = 0; dz = 1; }
    if (zint >= m_GridSize[2]-1) {zint = m_GridSize[2]-1; dz = -1;}


    m_NeighbourTracker.cellidx[0] = xint + m_GridSize[0]*(yint+zint*m_GridSize[1]);
    m_NeighbourTracker.cellidx[1] = m_NeighbourTracker.cellidx[0] + dx;
    m_NeighbourTracker.cellidx[2] = m_NeighbourTracker.cellidx[1] + dy*m_GridSize[0];
    m_NeighbourTracker.cellidx[3] = m_NeighbourTracker.cellidx[2] - dx;
    m_NeighbourTracker.cellidx[4] = m_NeighbourTracker.cellidx[0] + dz*m_GridSize[0]*m_GridSize[1];
    m_NeighbourTracker.cellidx[5] = m_NeighbourTracker.cellidx[4] + dx;
    m_NeighbourTracker.cellidx[6] = m_NeighbourTracker.cellidx[5] + dy*m_GridSize[0];
    m_NeighbourTracker.cellidx[7] = m_NeighbourTracker.cellidx[6] - dx;


    m_NeighbourTracker.cellidx_c[0] = m_CellCapacity*m_NeighbourTracker.cellidx[0];
    m_NeighbourTracker.cellidx_c[1] = m_CellCapacity*m_NeighbourTracker.cellidx[1];
    m_NeighbourTracker.cellidx_c[2] = m_CellCapacity*m_NeighbourTracker.cellidx[2];
    m_NeighbourTracker.cellidx_c[3] = m_CellCapacity*m_NeighbourTracker.cellidx[3];
    m_NeighbourTracker.cellidx_c[4] = m_CellCapacity*m_NeighbourTracker.cellidx[4];
    m_NeighbourTracker.cellidx_c[5] = m_CellCapacity*m_NeighbourTracker.cellidx[5];
    m_NeighbourTracker.cellidx_c[6] = m_CellCapacity*m_NeighbourTracker.cellidx[6];
    m_NeighbourTracker.cellidx_c[7] = m_CellCapacity*m_NeighbourTracker.cellidx[7];

    m_NeighbourTracker.cellcnt = 0;
    m_NeighbourTracker.pcnt = 0;
}

Particle* ParticleGrid::GetNextNeighbor()
{
    if (m_NeighbourTracker.pcnt < m_OccupationCount[m_NeighbourTracker.cellidx[m_NeighbourTracker.cellcnt]])
    {
        return m_Grid[m_NeighbourTracker.cellidx_c[m_NeighbourTracker.cellcnt] + (m_NeighbourTracker.pcnt++)];
    }
    else
    {
        for(;;)
        {
            m_NeighbourTracker.cellcnt++;
            if (m_NeighbourTracker.cellcnt >= 8)
                return 0;
            if (m_OccupationCount[m_NeighbourTracker.cellidx[m_NeighbourTracker.cellcnt]] > 0)
                break;
        }
        m_NeighbourTracker.pcnt = 1;
        return m_Grid[m_NeighbourTracker.cellidx_c[m_NeighbourTracker.cellcnt]];
    }
}

void ParticleGrid::CreateConnection(Particle *P1,int ep1, Particle *P2, int ep2)
{
    if (ep1 == -1)
        P1->mID = P2->ID;
    else
        P1->pID = P2->ID;

    if (ep2 == -1)
        P2->mID = P1->ID;
    else
        P2->pID = P1->ID;

    m_NumConnections++;
}

void ParticleGrid::DestroyConnection(Particle *P1,int ep1, Particle *P2, int ep2)
{
    if (ep1 == -1)
        P1->mID = -1;
    else
        P1->pID = -1;

    if (ep2 == -1)
        P2->mID = -1;
    else
        P2->pID = -1;
    m_NumConnections--;
}

void ParticleGrid::DestroyConnection(Particle *P1,int ep1)
{

    Particle *P2 = 0;
    if (ep1 == 1)
    {
        P2 = &m_Particles[P1->pID];
        P1->pID = -1;
    }
    else
    {
        P2 = &m_Particles[P1->mID];
        P1->mID = -1;
    }

    if (P2->mID == P1->ID)
        P2->mID = -1;
    else
        P2->pID = -1;

    m_NumConnections--;
}

bool ParticleGrid::CheckConsistency()
{
    for (int i=0; i<m_NumParticles; i++)
    {
        Particle* p = &m_Particles[i];
        if (p->ID != i)
        {
            std::cout << "Particle ID error!" << std::endl;
            return false;
        }

        if (p->mID!=-1)
        {
            Particle* p2 = &m_Particles[p->mID];
            if (p2->mID!=p->ID && p2->pID!=p->ID)
            {
                std::cout << "Connection inconsistent!" << std::endl;
                return false;
            }
        }
        if (p->pID!=-1)
        {
            Particle* p2 = &m_Particles[p->pID];
            if (p2->mID!=p->ID && p2->pID!=p->ID)
            {
                std::cout << "Connection inconsistent!" << std::endl;
                return false;
            }
        }
    }
    return true;
}
