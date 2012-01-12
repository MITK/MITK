


#ifndef _PARTICLEGRID
#define _PARTICLEGRID


#include "auxilary_classes.cpp"


template <class T>
    class ParticleGrid
{

  //////////////// Container
public:
  T *particles; // particles in linear array
  int pcnt; // actual number of particles
  int concnt; // number of connections
  int celloverflows;

  T **ID_2_address;

private:

  int capacity; // maximal number of particles
  int increase_step;

  /////////////////// Grid

  T **grid;   // the grid

  // grid size
  int nx;
  int ny;
  int nz;

  // scaling factor for grid
  float mulx;
  float muly;
  float mulz;

  int csize; // particle capacity of single cell in grid
  int *occnt; // occupation count of grid cells
  int gridsize; // total number of cells
  float m_Memory;

  struct NeighborTracker  // to run over the neighbors
  {
    int cellidx[8];
    int cellidx_c[8];

    int cellcnt;
    int pcnt;

  } nbtrack;



public:


  ParticleGrid()
  {

    //// involving the container
    capacity = 0;
    particles = 0;
    ID_2_address = 0;
    pcnt = 0;
    concnt = 0;
    celloverflows = 0;

    ////// involvin the grid
    nx = 0; ny = 0; nz = 0; csize = 0;
    gridsize = 0;
    grid = (T**) 0;
    occnt = (int*) 0;

    increase_step = 100000;
    m_Memory = 0;
  }

  float GetMemoryUsage()
  {
    return m_Memory;
  }

  int allocate(int _capacity,
               int _nx, int _ny, int _nz, float cellsize, int cellcapacity)
  {
    //// involving the container
    capacity = _capacity;
    particles = (T*) malloc(sizeof(T)*capacity);
    ID_2_address = (T**) malloc(sizeof(T*)*capacity);

    if (particles == 0 || ID_2_address == 0)
    {
      fprintf(stderr,"error: Out of Memory\n");
      capacity = 0;
      return -1;
    }
    else
    {
      fprintf(stderr,"Allocated Memory for %i particles \n",capacity);
    }

    pcnt = 0;
    int i;
    for (i = 0;i < capacity;i++)
    {
      ID_2_address[i] = &(particles[i]);   // initialize pointer in LUT
      particles[i].ID = i;         // initialize unique IDs
    }

    ////// involvin the grid
    nx = _nx; ny = _ny; nz = _nz; csize = cellcapacity;
    gridsize = nx*ny*nz;

    m_Memory = (float)(sizeof(T*)*gridsize*csize)/1000000;

    grid = (T**) malloc(sizeof(T*)*gridsize*csize);
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



  int reallocate()
  {
    int new_capacity = capacity + increase_step;
    T* new_particles = (T*) realloc(particles,sizeof(T)*new_capacity);
    T** new_ID_2_address = (T**) realloc(ID_2_address,sizeof(T*)*new_capacity);
    if (new_particles == 0 || new_ID_2_address == 0)
    {
      fprintf(stderr,"ParticleGird:reallocate: out of memory!\n");
      return -1;
    }
    fprintf(stderr,"   now %i particles are allocated \n",new_capacity);
    m_Memory = (float)(sizeof(T*)*new_capacity)/1000000;

    int i;
    for (i = 0; i < capacity; i++)
    {
      new_ID_2_address[i] = new_ID_2_address[i] - particles + new_particles;   // shift address
    }
    for (i = capacity; i < new_capacity; i++)
    {
      new_particles[i].ID = i;         // initialize unique IDs
      new_ID_2_address[i] = &(new_particles[i]) ;   // initliaze pointer in LUT
    }
    for (i = 0; i < nx*ny*nz*csize; i++)
    {
      grid[i] = grid[i] - particles + new_particles;
    }
    particles = new_particles;
    ID_2_address = new_ID_2_address;
    capacity = new_capacity;

    return 1;
  }



  ~ParticleGrid()
  {
    if (particles != 0)
      free(particles);
    if (grid != 0)
      free(grid);
    if (occnt != 0)
      free(occnt);
    if (ID_2_address != 0)
      free(ID_2_address);
  }



  int ID_2_index(int ID)
  {
    if (ID == -1)
      return -1;
    else
      return (ID_2_address[ID] - particles);

  }


  T* newParticle(pVector R)
  {
    /////// get free place in container;
    if (pcnt >= capacity)
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
      T *p = &(particles[pcnt]);
      p->R = R;
      p->mID = -1;
      p->pID = -1;
      pcnt++;
      p->gridindex = csize*idx + occnt[idx];
      grid[p->gridindex] = p;
      occnt[idx]++;
      return p;
    }
    else
    {
      celloverflows++;
      //fprintf(stderr,"error: cell overflow \n");
      return 0;
    }
  }


  inline void updateGrid(int k)
  {
    T* p = &(particles[k]);

    /////// find new grid cell
    int xint = int(p->R[0]*mulx);
    if (xint < 0) { remove(k); return; }
    if (xint >= nx) { remove(k); return; }
    int yint = int(p->R[1]*muly);
    if (yint < 0) { remove(k); return; }
    if (yint >= ny) { remove(k); return; }
    int zint = int(p->R[2]*mulz);
    if (zint < 0) { remove(k); return; }
    if (zint >= nz) { remove(k); return; }


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
      }
      else
        remove(k);

    }
  }


  inline bool tryUpdateGrid(int k)
  {
    T* p = &(particles[k]);

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



  inline void remove(int k)
  {
    T* p = &(particles[k]);
    int grdindex = p->gridindex;
    int cellidx = grdindex/csize;
    int idx = grdindex%csize;

    // remove pending connections
    if (p->mID != -1)
      destroyConnection(p,-1);
    if (p->pID != -1)
      destroyConnection(p,+1);

    // remove from grid
    if (idx < occnt[cellidx]-1)
    {
      grid[grdindex] = grid[cellidx*csize + occnt[cellidx]-1];
      grid[grdindex]->gridindex = grdindex;
    }
    occnt[cellidx]--;



    // remove from container
    if (k<pcnt-1)
    {
      int todel_ID = p->ID;
      int move_ID = particles[pcnt-1].ID;

      *p = particles[pcnt-1];          // move very last particle to empty slot
      particles[pcnt-1].ID = todel_ID; // keep IDs unique
      grid[p->gridindex] = p;          // keep gridindex consistent

      // permute address table
      ID_2_address[todel_ID] = &(particles[pcnt-1]);
      ID_2_address[move_ID]  = p;

    }
    pcnt--;



  }

  inline void computeNeighbors(pVector &R)
  {
    float xfrac = R.GetX()*mulx;
    float yfrac = R.GetY()*muly;
    float zfrac = R.GetZ()*mulz;
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


    nbtrack.cellidx[0] = xint + nx*(yint+zint*ny);
    nbtrack.cellidx[1] = nbtrack.cellidx[0] + dx;
    nbtrack.cellidx[2] = nbtrack.cellidx[1] + dy*nx;
    nbtrack.cellidx[3] = nbtrack.cellidx[2] - dx;
    nbtrack.cellidx[4] = nbtrack.cellidx[0] + dz*nx*ny;
    nbtrack.cellidx[5] = nbtrack.cellidx[4] + dx;
    nbtrack.cellidx[6] = nbtrack.cellidx[5] + dy*nx;
    nbtrack.cellidx[7] = nbtrack.cellidx[6] - dx;


    nbtrack.cellidx_c[0] = csize*nbtrack.cellidx[0];
    nbtrack.cellidx_c[1] = csize*nbtrack.cellidx[1];
    nbtrack.cellidx_c[2] = csize*nbtrack.cellidx[2];
    nbtrack.cellidx_c[3] = csize*nbtrack.cellidx[3];
    nbtrack.cellidx_c[4] = csize*nbtrack.cellidx[4];
    nbtrack.cellidx_c[5] = csize*nbtrack.cellidx[5];
    nbtrack.cellidx_c[6] = csize*nbtrack.cellidx[6];
    nbtrack.cellidx_c[7] = csize*nbtrack.cellidx[7];



    nbtrack.cellcnt = 0;
    nbtrack.pcnt = 0;

  }



  inline T *getNextNeighbor()
  {

    if (nbtrack.pcnt < occnt[nbtrack.cellidx[nbtrack.cellcnt]])
    {

      return grid[nbtrack.cellidx_c[nbtrack.cellcnt] + (nbtrack.pcnt++)];

    }
    else
    {

      for(;;)
      {
        nbtrack.cellcnt++;
        if (nbtrack.cellcnt >= 8)
          return 0;
        if (occnt[nbtrack.cellidx[nbtrack.cellcnt]] > 0)
          break;
      }

      nbtrack.pcnt = 1;
      return grid[nbtrack.cellidx_c[nbtrack.cellcnt]];
    }
  }


  inline void createConnection(T *P1,int ep1, T *P2, int ep2)
  {
    if (ep1 == -1)
      P1->mID = P2->ID;
    else
      P1->pID = P2->ID;

    if (ep2 == -1)
      P2->mID = P1->ID;
    else
      P2->pID = P1->ID;

    concnt++;
  }

  inline void destroyConnection(T *P1,int ep1, T *P2, int ep2)
  {
    if (ep1 == -1)
      P1->mID = -1;
    else
      P1->pID = -1;

    if (ep2 == -1)
      P2->mID = -1;
    else
      P2->pID = -1;
    concnt--;
  }

  inline void destroyConnection(T *P1,int ep1)
  {

    T *P2 = 0;
    if (ep1 == 1)
    {
      P2 = ID_2_address[P1->pID];
      P1->pID = -1;
    }
    else
    {
      P2 = ID_2_address[P1->mID];
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
    concnt--;

  }
};




/*


struct Connection
{
  int lID;
  int rID;
};


class ConnectionContainer
{

  //////////////// Container
public:
  Connection *cons; // cons in linear array
  int ccnt; // actual number of cons

private:

  int capacity; // maximal number of particles



public:


  ConnectionContainer()
  {

    //// involving the container
    capacity = 0;
    cons = 0;
    ccnt = 0;

  }



  void allocate(int _capacity)
  {

    //// involving the container
    capacity = _capacity;
    cons = (Connection*) malloc(sizeof(Connection)*capacity);
    ccnt = 0;

  }

  ~ConnectionContainer()
  {
    if (cons != 0)
      free(cons);
  }


  Connection* newConnection(int lid,int rid)
  {
    /////// get free place in container;
    if (ccnt < capacity)
    {
      Connection *c = &(cons[ccnt]);
      c->lID = lid;
      c->rID = rid;
      ccnt++;
      return c;
    }
          return 0;
  }

  inline void remove(int k)
  {
    Connection* c = &(cons[k]);

    // remove from container
    if (k<ccnt-1)
    {
      *c = cons[ccnt-1];
    }
    ccnt--;

  }
};


*/

#endif
