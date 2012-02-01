
#ifndef _AUXCLASS
#define _AUXCLASS

#ifndef INFINITY
#define INFINITY 1000000000
#endif

//#define INLINE __attribute__ ((always_inline))
#define INLINE inline

//#define __SSE2



#ifdef __SSE2
#include <xmmintrin.h>

class pVector
{

private:
  __m128 r;

public:

  static float store[4];

  pVector()
  {

  }

  pVector(__m128 q)
  {
    r = q;
  }


  pVector(float x,float y,float z)
  {
    r = _mm_set_ps(0,z,y,x);
  }


  INLINE void storeXYZ()
  {
    _mm_store_ps(store,r);
  }

  INLINE void setXYZ(float sx,float sy,float sz)
  {
    r = _mm_set_ps(0,sz,sy,sx);
  }




  INLINE void rand(float w,float h,float d)
  {
    float x = mtrand.frand()*w;
    float y = mtrand.frand()*h;
    float z = mtrand.frand()*d;
    r = _mm_set_ps(0,z,y,x);

  }

  INLINE void rand_sphere()
  {
    r = _mm_set_ps(0,mtrand.frandn(),mtrand.frandn(),mtrand.frandn());
    normalize();
  }

  INLINE void normalize()
  {
    __m128 q = _mm_mul_ps(r,r);
    _mm_store_ps(store,q);
    float norm = sqrt(store[0]+store[1]+store[2]) + 0.00000001;
    q = _mm_set_ps1(1/norm);
    r = _mm_mul_ps(r,q);

  }

  INLINE float norm_square()
  {
    __m128 q = _mm_mul_ps(r,r);
    _mm_store_ps(store,q);
    return store[0]+store[1]+store[2];
  }

  INLINE void distortn(float sigma)
  {
    __m128 s = _mm_set_ps(0,mtrand.frandn(),mtrand.frandn(),mtrand.frandn());
    __m128 q = _mm_set_ps1(sigma);
    r = _mm_add_ps(r,_mm_mul_ps(q,s));
  }

  INLINE pVector operator*(float s)
  {
    __m128 q = _mm_set_ps1(s);
    return pVector(_mm_mul_ps(q,r));
  }

  INLINE void operator*=(float &s)
  {
    __m128 q = _mm_set_ps1(s);
    r = _mm_mul_ps(q,r);
  }

  INLINE pVector operator+(pVector R)
  {
    return pVector(_mm_add_ps(R.r,r));
  }

  INLINE void operator+=(pVector R)
  {
    r = _mm_add_ps(r,R.r);
  }

  INLINE pVector operator-(pVector R)
  {
    return pVector(_mm_sub_ps(r,R.r));
  }

  INLINE void operator-=(pVector R)
  {
    r = _mm_sub_ps(r,R.r);
  }

  INLINE pVector operator/(float &s)
  {
    __m128 q = _mm_set_ps1(s);
    return pVector(_mm_div_ps(q,r));
  }

  INLINE void operator/=(float &s)
  {
    __m128 q = _mm_set_ps1(s);
    r = _mm_div_ps(r,q);
  }

  INLINE float operator*(pVector R)
  {
    __m128 q = _mm_mul_ps(r,R.r);
    _mm_store_ps(store,q);
    return store[0]+store[1]+store[2];
  }


};

float pVector::store[4];

#else

class pVector
{
private:
  float x;
  float y;
  float z;

public:

  pVector()
  {

  }

  pVector(float x,float y,float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  INLINE void SetXYZ(float sx,float sy, float sz)
  {
    x = sx;
    y = sy;
    z = sz;
  }

  INLINE void GetXYZ(float *xyz)
  {
    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;
  }

  INLINE float GetX()
  {
    return x;
  }

  INLINE float GetY()
  {
    return y;
  }

  INLINE float GetZ()
  {
    return z;
  }

  INLINE void rand(float w, float h, float d)
  {
    this->x = mtrand.frand()*w;
    this->y = mtrand.frand()*h;
    this->z = mtrand.frand()*d;
  }

  INLINE void rand_sphere()
  {
    this->x = mtrand.frandn();
    this->y = mtrand.frandn();
    this->z = mtrand.frandn();
    normalize();
  }

  INLINE void normalize()
  {
    float norm = sqrt(x*x+y*y+z*z)+ 0.00000001;
    *this /= norm;
  }

  INLINE float norm_square()
  {
    return x*x + y*y + z*z;
  }

  INLINE void distortn(float sigma)
  {
    x += sigma*mtrand.frandn();
    y += sigma*mtrand.frandn();
    z += sigma*mtrand.frandn();
  }

  INLINE float operator[](int index)
  {
    switch(index)
    {
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
      default:
        return 0.0f;
    }
  }

  INLINE pVector operator*(float s)
  {
    return pVector(s*x,s*y,s*z);
  }

  INLINE void operator*=(float &s)
  {
    x *= s;
    y *= s;
    z *= s;
  }

  INLINE pVector operator+(pVector R)
  {
    return pVector(x+R.x,y+R.y,z+R.z);
  }

  INLINE void operator+=(pVector R)
  {
    x += R.x;
    y += R.y;
    z += R.z;
  }

  INLINE pVector operator-(pVector R)
  {
    return pVector(x-R.x,y-R.y,z-R.z);
  }

  INLINE void operator-=(pVector R)
  {
    x -= R.x;
    y -= R.y;
    z -= R.z;
  }

  INLINE pVector operator/(float &s)
  {
    return pVector(x/s,y/s,z/s);
  }

  INLINE void operator/=(float &s)
  {
    x /= s;
    y /= s;
    z /= s;
  }


  INLINE float operator*(pVector R)
  {
    return x*R.x+y*R.y+z*R.z;
  }
};

#endif


class Particle
{
public:

  Particle()
  {
    label = 0;
    pID = -1;
    mID = -1;
    inserted = false;
  }

  ~Particle()
  {
  }

  pVector R;
  pVector N;
  float cap;
  float len;

  int gridindex; // index in the grid where it is living
  int ID;
  int pID;
  int mID;

  int label;
  int numerator;
  bool inserted;
};


class EnergyGradient
{
public:
  pVector gR;
  pVector gN;

  INLINE float norm2()
  {
    return gR.norm_square() + gN.norm_square();
  }
} ;


template <class T>
class SimpSamp
{

  float *P;
  int cnt;

public:
  T *objs;


  SimpSamp()
  {
    P = (float*) malloc(sizeof(float)*1000);
    objs = (T*) malloc(sizeof(T)*1000);
  }
  ~SimpSamp()
  {
    free(P);
    free(objs);
  }

  INLINE void clear()
  {
    cnt = 1;
    P[0] = 0;
  }

  INLINE void add(float p, T obj)
  {
    P[cnt] = P[cnt-1] + p;
    objs[cnt-1] = obj;
    cnt++;
  }

  //   INLINE int draw()
  //   {
  //     float r = mtrand.frand()*P[cnt-1];
  //     for (int i = 1; i < cnt; i++)
  //     {
  //       if (r <= P[i])
  //         return i-1;
  //     }
  //     return cnt-2;
  //   }

  INLINE int draw()
  {
    float r = mtrand.frand()*P[cnt-1];
    int j;
    int rl = 1;
    int rh = cnt-1;
    while(rh != rl)
    {
      j = rl + (rh-rl)/2;
      if (r < P[j])
      {
        rh = j;
        continue;
      }
      if (r > P[j])
      {
        rl = j+1;
        continue;
      }
      break;
    }
    return rh-1;
  }





  INLINE T drawObj()
  {
    return objs[draw()];
  }

  INLINE bool isempty()
  {
    if (cnt == 1)
      return true;
    else
      return false;
  }


  float probFor(int idx)
  {
    return (P[idx+1]-P[idx])/P[cnt-1];
  }

  float probFor(T &t)
  {
    for (int i = 1; i< cnt;i++)
    {
      if (t == objs[i-1])
        return probFor(i-1);
    }
    return 0;
  }



};


class EndPoint
{
public:
  EndPoint()
  {}

  EndPoint(Particle *p,int ep)
  {
    this->p = p;
    this->ep = ep;
  }
  Particle *p;
  int ep;

  inline bool operator==(EndPoint P)
  {
    return (P.p == p) && (P.ep == ep);
  }
};

class Track
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

float getMax(float *arr, int cnt)
{
  float max = arr[0];
  for (int i = 1; i < cnt; i++)
  {
    if (arr[i] > max)
      max = arr[i];
  }
  return max;
}



float getMin(float *arr, int cnt)
{
  float min = arr[0];
  for (int i = 1; i < cnt; i++)
  {
    if (arr[i] < min)
      min = arr[i];
  }
  return min;
}


int getArgMin(float *arr, int cnt)
{
  float min = arr[0];
  int idx = 0;
  for (int i = 1; i < cnt; i++)
  {
    if (arr[i] < min)
    {
      min = arr[i];
      idx = i;
    }
  }
  return idx;
}




inline float distLseg(pVector &R1,pVector &N1,pVector &R2,pVector &N2,float &len)
{

  pVector D = R1-R2;
  float beta  = N1*N2;
  float divisor = 1.001-beta*beta;
  float gamma1 = N1*D;
  float gamma2 = N2*D;
  float t,u;
  float EPdist[4];

  pVector Q;
  float dist = 102400000.0;

  while(true)
  {

    t = -(gamma1+beta*gamma2) / divisor;
    u =  (gamma1*beta+gamma2) / divisor;
    if (fabs(t) < len && fabs(u) < len)
    {
      Q = D +  N1*t - N2*u;
      dist = Q*Q;
      break;
    }

    beta = len*beta;

    t = beta - gamma1;
    if (fabs(t) < len)
    {
      Q = D +  N1*t - N2*len;
      float d = Q*Q;
      if (d < dist) dist = d;
    }

    t = -beta - gamma1;
    if (fabs(t) < len)
    {
      Q = D +  N1*t + N2*len;
      float d = Q*Q;
      if (d < dist) dist = d;
    }

    u = beta + gamma2;
    if (fabs(u) < len)
    {
      Q = D +  N1*len - N2*u;
      float d = Q*Q;
      if (d < dist) dist = d;
    }

    u = -beta + gamma2;
    if (fabs(u) < len)
    {
      Q = D -  N1*len - N2*u;
      float d = Q*Q;
      if (d < dist) dist = d;
    }

    if (dist != 102400000.0)
      break;


    EPdist[0] =  beta + gamma1 - gamma2;
    EPdist[1] = -beta + gamma1 + gamma2;
    EPdist[2] = -beta - gamma1 - gamma2;
    EPdist[3] =  beta - gamma1 + gamma2;
    int c = getArgMin(EPdist,4);
    if (c==0) {t = +len; u = +len; }
    if (c==1) {t = +len; u = -len; }
    if (c==2) {t = -len; u = +len; }
    if (c==3) {t = -len; u = -len; }
    Q = D +  N1*t - N2*u;
    dist = Q*Q;
    break;

  }


  return dist;

}


#endif


