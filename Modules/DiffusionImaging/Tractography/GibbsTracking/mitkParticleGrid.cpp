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

ParticleGrid::ParticleGrid(ItkFloatImageType* image, float cellSize)
{
    //// involving the container
    m_NumParticles = 0;
    m_NumConnections = 0;
    m_NumCellOverflows = 0;

    increase_step = 100000;

    int width = image->GetLargestPossibleRegion().GetSize()[0]*image->GetSpacing()[0];
    int height = image->GetLargestPossibleRegion().GetSize()[1]*image->GetSpacing()[1];
    int depth = image->GetLargestPossibleRegion().GetSize()[2]*image->GetSpacing()[2];

    float   cellcnt_x = (int)((float)width/cellSize) +1;
    float   cellcnt_y = (int)((float)height/cellSize) +1;
    float   cellcnt_z = (int)((float)depth/cellSize) +1;
    int     cell_capacity = 512;

    //// involving the container
    capacity = 100000;
    m_Particles.resize(capacity);

    int i;
    for (i = 0;i < capacity;i++)
        m_Particles[i].ID = i;                          // initialize unique IDs

    // involving the grid
    nx = cellcnt_x; ny = cellcnt_y; nz = cellcnt_z; csize = cell_capacity;
    gridsize = nx*ny*nz;

    grid.resize(gridsize*csize);
    occnt.resize(gridsize);

    for (i = 0;i < gridsize;i++)
        occnt[i] = 0;

    mulx = 1/cellSize;
    muly = 1/cellSize;
    mulz = 1/cellSize;
}

int ParticleGrid::ReallocateGrid()
{
    int new_capacity = capacity + increase_step;
    m_Particles.resize(new_capacity);
    fprintf(stderr,"   now %i particles are allocated \n",new_capacity);

    for (int i = 0; i<capacity; i++)    // update addresses
       grid[m_Particles[i].gridindex] = &m_Particles[i];

    for (int i = capacity; i < new_capacity; i++)
        m_Particles[i].ID = i;         // initialize unique IDs

    capacity = new_capacity;

    return 1;
}



ParticleGrid::~ParticleGrid()
{
}

Particle* ParticleGrid::GetParticle(int ID)
{
    return &m_Particles[ID];
}


Particle* ParticleGrid::newParticle(vnl_vector_fixed<float, 3> R)
{
    /////// get free place in container;
    if (m_NumParticles >= capacity)
    {
        fprintf(stderr,"capacity overflow , reallocating ...\n");
        if (ReallocateGrid() == -1)
        {
            fprintf(stderr,"out of Memory!!\n");
            return 0;
        }
    }

    int xint = int(R[0]*mulx);
    if (xint < 0)
        return 0;
    if (xint >= nx)
        return 0;
    int yint = int(R[1]*muly);
    if (yint < 0)
        return 0;
    if (yint >= ny)
        return 0;
    int zint = int(R[2]*mulz);
    if (zint < 0)
        return 0;
    if (zint >= nz)
        return 0;

    int idx = xint + nx*(yint + ny*zint);
    if (occnt[idx] < csize)
    {
        Particle *p = &(m_Particles[m_NumParticles]);
        p->R = R;
        p->mID = -1;
        p->pID = -1;
        m_NumParticles++;
        p->gridindex = csize*idx + occnt[idx];
        grid[p->gridindex] = p;
        occnt[idx]++;
        return p;
    }
    else
    {
        m_NumCellOverflows++;
        return 0;
    }
}

bool ParticleGrid::TryUpdateGrid(int k)
{
    Particle* p = &(m_Particles[k]);

    /////// find new grid cell
    int xint = int(p->R[0]*mulx);
    if (xint < 0) {  return false; }
    if (xint >= nx) { return false; }
    int yint = int(p->R[1]*muly);
    if (yint < 0) { return false; }
    if (yint >= ny) { return false; }
    int zint = int(p->R[2]*mulz);
    if (zint < 0) { return false; }
    if (zint >= nz) { return false; }


    int idx = xint + nx*(yint+ zint*ny);
    int cellidx = p->gridindex/csize;
    if (idx != cellidx) // cell has changed
    {

        if (occnt[idx] < csize)
        {
            // remove from old position in grid;
            int grdindex = p->gridindex;
            grid[grdindex] = grid[cellidx*csize + occnt[cellidx]-1];
            grid[grdindex]->gridindex = grdindex;
            occnt[cellidx]--;

            // insert at new position in grid
            p->gridindex = idx*csize + occnt[idx];
            grid[p->gridindex] = p;
            occnt[idx]++;
            return true;
        }
        else
            return false;

    }
    return true;
}



void ParticleGrid::RemoveParticle(int k)
{
    Particle* p = &(m_Particles[k]);
    int grdindex = p->gridindex;
    int cellidx = grdindex/csize;

    // remove pending connections
    if (p->mID != -1)
        DestroyConnection(p,-1);
    if (p->pID != -1)
        DestroyConnection(p,+1);

    // remove from grid (no need to actually change grid entries because the adresses stay the same)
    occnt[cellidx]--;

    // remove from container
    if (k<m_NumParticles-1)
    {
        Particle* toMove = &m_Particles[m_NumParticles-1];  // last particle

        // update connections of last particle because its index is changig
        if (toMove->mID!=-1)
        {
            if ( m_Particles[toMove->mID].mID == m_NumParticles-1 )
                m_Particles[toMove->mID].mID = k;
            else if ( m_Particles[toMove->mID].pID == m_NumParticles-1 )
                m_Particles[toMove->mID].pID = k;
        }
        if (toMove->pID!=-1)
        {
            if ( m_Particles[toMove->pID].mID == m_NumParticles-1 )
                m_Particles[toMove->pID].mID = k;
            else if ( m_Particles[toMove->pID].pID == m_NumParticles-1 )
                m_Particles[toMove->pID].pID = k;
        }

        m_Particles[k] = m_Particles[m_NumParticles-1];         // move very last particle to empty slot
        m_Particles[m_NumParticles-1].ID = m_NumParticles-1;    // update ID of deleted element to match the index
        m_Particles[k].ID = k;                                  // update ID of moved element
        m_Particles[k].gridindex = grdindex;                    // update grid index of moved element
    }
    m_NumParticles--;
}

void ParticleGrid::ComputeNeighbors(vnl_vector_fixed<float, 3> &R)
{
    float xfrac = R[0]*mulx;
    float yfrac = R[1]*muly;
    float zfrac = R[2]*mulz;
    int xint = int(xfrac);
    int yint = int(yfrac);
    int zint = int(zfrac);

    int dx = -1;
    if (xfrac-xint > 0.5) dx = 1;
    if (xint <= 0) { xint = 0; dx = 1; }
    if (xint >= nx-1) { xint = nx-1; dx = -1; }

    int dy = -1;
    if (yfrac-yint > 0.5) dy = 1;
    if (yint <= 0) {yint = 0; dy = 1; }
    if (yint >= ny-1) {yint = ny-1; dy = -1;}

    int dz = -1;
    if (zfrac-zint > 0.5) dz = 1;
    if (zint <= 0) {zint = 0; dz = 1; }
    if (zint >= nz-1) {zint = nz-1; dz = -1;}


    m_NeighbourTracker.cellidx[0] = xint + nx*(yint+zint*ny);
    m_NeighbourTracker.cellidx[1] = m_NeighbourTracker.cellidx[0] + dx;
    m_NeighbourTracker.cellidx[2] = m_NeighbourTracker.cellidx[1] + dy*nx;
    m_NeighbourTracker.cellidx[3] = m_NeighbourTracker.cellidx[2] - dx;
    m_NeighbourTracker.cellidx[4] = m_NeighbourTracker.cellidx[0] + dz*nx*ny;
    m_NeighbourTracker.cellidx[5] = m_NeighbourTracker.cellidx[4] + dx;
    m_NeighbourTracker.cellidx[6] = m_NeighbourTracker.cellidx[5] + dy*nx;
    m_NeighbourTracker.cellidx[7] = m_NeighbourTracker.cellidx[6] - dx;


    m_NeighbourTracker.cellidx_c[0] = csize*m_NeighbourTracker.cellidx[0];
    m_NeighbourTracker.cellidx_c[1] = csize*m_NeighbourTracker.cellidx[1];
    m_NeighbourTracker.cellidx_c[2] = csize*m_NeighbourTracker.cellidx[2];
    m_NeighbourTracker.cellidx_c[3] = csize*m_NeighbourTracker.cellidx[3];
    m_NeighbourTracker.cellidx_c[4] = csize*m_NeighbourTracker.cellidx[4];
    m_NeighbourTracker.cellidx_c[5] = csize*m_NeighbourTracker.cellidx[5];
    m_NeighbourTracker.cellidx_c[6] = csize*m_NeighbourTracker.cellidx[6];
    m_NeighbourTracker.cellidx_c[7] = csize*m_NeighbourTracker.cellidx[7];

    m_NeighbourTracker.cellcnt = 0;
    m_NeighbourTracker.pcnt = 0;
}



Particle* ParticleGrid::GetNextNeighbor()
{

    if (m_NeighbourTracker.pcnt < occnt[m_NeighbourTracker.cellidx[m_NeighbourTracker.cellcnt]])
    {

        return grid[m_NeighbourTracker.cellidx_c[m_NeighbourTracker.cellcnt] + (m_NeighbourTracker.pcnt++)];

    }
    else
    {

        for(;;)
        {
            m_NeighbourTracker.cellcnt++;
            if (m_NeighbourTracker.cellcnt >= 8)
                return 0;
            if (occnt[m_NeighbourTracker.cellidx[m_NeighbourTracker.cellcnt]] > 0)
                break;
        }

        m_NeighbourTracker.pcnt = 1;
        return grid[m_NeighbourTracker.cellidx_c[m_NeighbourTracker.cellcnt]];
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
    {
        P2->mID = -1;
    }
    else
    {
        P2->pID = -1;
    }
    m_NumConnections--;
}
