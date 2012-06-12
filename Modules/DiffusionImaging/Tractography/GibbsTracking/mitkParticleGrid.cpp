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

ParticleGrid::ParticleGrid()
{
    //// involving the container
    capacity = 0;
    m_Particles = 0;
    m_AddressContainer = 0;
    m_NumParticles = 0;
    m_NumConnections = 0;
    m_NumCellOverflows = 0;

    ////// involvin the grid
    nx = 0; ny = 0; nz = 0; csize = 0;
    gridsize = 0;
    grid = (Particle**) 0;
    occnt = (int*) 0;

    increase_step = 100000;
    m_Memory = 0;
}

float ParticleGrid::GetMemoryUsage()
{
    return m_Memory;
}

int ParticleGrid::allocate(int _capacity, int _nx, int _ny, int _nz, float cellsize, int cellcapacity)
{
    //// involving the container
    capacity = _capacity;
    m_Particles = (Particle*) malloc(sizeof(Particle)*capacity);
    m_AddressContainer = (Particle**) malloc(sizeof(Particle*)*capacity);

    if (m_Particles == 0 || m_AddressContainer == 0)
    {
        fprintf(stderr,"error: Out of Memory\n");
        capacity = 0;
        return -1;
    }
    else
    {
        fprintf(stderr,"Allocated Memory for %i particles \n",capacity);
    }

    m_NumParticles = 0;
    int i;
    for (i = 0;i < capacity;i++)
    {
        m_AddressContainer[i] = &(m_Particles[i]);   // initialize pointer in LUT
        m_Particles[i].ID = i;         // initialize unique IDs
    }

    ////// involvin the grid
    nx = _nx; ny = _ny; nz = _nz; csize = cellcapacity;
    gridsize = nx*ny*nz;

    m_Memory = (float)(sizeof(Particle*)*gridsize*csize)/1000000;

    grid = (Particle**) malloc(sizeof(Particle*)*gridsize*csize);
    occnt = (int*) malloc(sizeof(int)*gridsize);

    if (grid == 0 || occnt == 0)
    {
        fprintf(stderr,"error: Out of Memory\n");
        capacity = 0;
        return -1;
    }

    for (i = 0;i < gridsize;i++)
        occnt[i] = 0;

    mulx = 1/cellsize;
    muly = 1/cellsize;
    mulz = 1/cellsize;

    return 1;
}



int ParticleGrid::reallocate()
{
    int new_capacity = capacity + increase_step;
    Particle* new_particles = (Particle*) realloc(m_Particles,sizeof(Particle)*new_capacity);
    Particle** new_ID_2_address = (Particle**) realloc(m_AddressContainer,sizeof(Particle*)*new_capacity);
    if (new_particles == 0 || new_ID_2_address == 0)
    {
        fprintf(stderr,"ParticleGird:reallocate: out of memory!\n");
        return -1;
    }
    fprintf(stderr,"   now %i particles are allocated \n",new_capacity);
    m_Memory = (float)(sizeof(Particle*)*new_capacity)/1000000;

    int i;
    for (i = 0; i < capacity; i++)
    {
        new_ID_2_address[i] = new_ID_2_address[i] - m_Particles + new_particles;   // shift address
    }
    for (i = capacity; i < new_capacity; i++)
    {
        new_particles[i].ID = i;         // initialize unique IDs
        new_ID_2_address[i] = &(new_particles[i]) ;   // initliaze pointer in LUT
    }
    for (i = 0; i < nx*ny*nz*csize; i++)
    {
        grid[i] = grid[i] - m_Particles + new_particles;
    }
    m_Particles = new_particles;
    m_AddressContainer = new_ID_2_address;
    capacity = new_capacity;

    return 1;
}



ParticleGrid::~ParticleGrid()
{
    if (m_Particles != 0)
        free(m_Particles);
    if (grid != 0)
        free(grid);
    if (occnt != 0)
        free(occnt);
    if (m_AddressContainer != 0)
        free(m_AddressContainer);
}



int ParticleGrid::ID_2_index(int ID)
{
    if (ID == -1)
        return -1;
    else
        return (m_AddressContainer[ID] - m_Particles);

}


Particle* ParticleGrid::newParticle(vnl_vector_fixed<float, 3> R)
{
    /////// get free place in container;
    if (m_NumParticles >= capacity)
    {
        fprintf(stderr,"capacity overflow , reallocating ...\n");
        if (reallocate() == -1)
        {
            fprintf(stderr,"out of Memory!!\n");
            return 0;
        }
    }

    int xint = int(R[0]*mulx);
    if (xint < 0) { //fprintf(stderr,"error: out of grid\n");
        return 0;}
    if (xint >= nx)  { // fprintf(stderr,"error: out of grid\n");
        return 0;}
    int yint = int(R[1]*muly);
    if (yint < 0)  { //fprintf(stderr,"error: out of grid\n");
        return 0;}
    if (yint >= ny)  {// fprintf(stderr,"error: out of grid\n");
        return 0;}
    int zint = int(R[2]*mulz);
    if (zint < 0) {// fprintf(stderr,"error: out of grid\n");
        return 0;}
    if (zint >= nz)  { //fprintf(stderr,"error: out of grid\n");
        return 0;}

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
        //fprintf(stderr,"error: cell overflow \n");
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
    int idx = grdindex%csize;

    // remove pending connections
    if (p->mID != -1)
        DestroyConnection(p,-1);
    if (p->pID != -1)
        DestroyConnection(p,+1);

    // remove from grid
    if (idx < occnt[cellidx]-1)
    {
        grid[grdindex] = grid[cellidx*csize + occnt[cellidx]-1];
        grid[grdindex]->gridindex = grdindex;
    }
    occnt[cellidx]--;



    // remove from container
    if (k<m_NumParticles-1)
    {
        int todel_ID = p->ID;
        int move_ID = m_Particles[m_NumParticles-1].ID;

        *p = m_Particles[m_NumParticles-1];          // move very last particle to empty slot
        m_Particles[m_NumParticles-1].ID = todel_ID; // keep IDs unique
        grid[p->gridindex] = p;          // keep gridindex consistent

        // permute address table
        m_AddressContainer[todel_ID] = &(m_Particles[m_NumParticles-1]);
        m_AddressContainer[move_ID]  = p;

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
        P2 = m_AddressContainer[P1->pID];
        P1->pID = -1;
    }
    else
    {
        P2 = m_AddressContainer[P1->mID];
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
