#include "mitkParticle.h"
#include "auxilary_classes.cpp"

using namespace std;

#define PI 3.1415926536

float squareNorm( vnl_vector_fixed<float, 3> v )
{
  return v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
}

mitk::Particle* createMitkParticle(Particle* p)
{
  mitk::Particle* particle = new mitk::Particle();
  pVector R = p->R;
  vnl_vector_fixed<float,3> pos;
  pos[0] = R[0]-0.5;
  pos[1] = R[1]-0.5;
  pos[2] = R[2]-0.5;
  particle->SetPosition(pos);
  //MITK_INFO << "mitk: " << particle->GetPosition();
  pVector N = p->N;
  vnl_vector_fixed<float,3> dir;
  dir[0] = N[0];
  dir[1] = N[1];
  dir[2] = N[2];
  particle->SetDirection(dir);
  return particle;
}

Particle* createParticle(Particle* particle){
  Particle* p = new Particle();
  p->R = pVector(particle->R);
  p->N = pVector(particle->N);
  return p;
}

vector<Particle*>* ResampleFibers(vector<Particle*>* particleContainer, float len)
{
  //vector< mitk::Particle::Pointer >* outContainer = new::vector< mitk::Particle::Pointer >();
  vector< Particle* >* container =
      new vector< Particle* >();

  int numPoints = particleContainer->size();
  if(numPoints<2)
    return container;

  Particle* source = createParticle(particleContainer->at(0));
  Particle* sink = createParticle(particleContainer->at(numPoints-1));

  float Leng = 0;

  container->push_back(source);
  //  source->R.storeXYZ();
  //  MITK_INFO << source->R.store[0] << ", " << source->R.store[1] << ", " << source->R.store[2];

  float dtau = 0;
  int cur_p = 1;
  int cur_i = 1;
  pVector dR;
  float normdR;

  for (;;)
  {
    while (dtau <= len && cur_p < numPoints)
    {
      dR  = particleContainer->at(cur_p)->R - particleContainer->at(cur_p-1)->R;
      normdR = sqrt(dR.norm_square());
      dtau += normdR;
      Leng += normdR;
      cur_p++;
    }

    if (dtau >= len)  // if particles reach next voxel
    {
      // proposal = current particle position - (current p - last p)*(current fibre length - len???)/(norm current p-last p)
      Particle* p = new Particle();
      p->R = particleContainer->at(cur_p-1)->R - dR*( (dtau-len)/normdR );
      p->N = dR;
      //      p->R.storeXYZ();
      //      MITK_INFO << p->R.store[0] << ", " << p->R.store[1] << ", " << p->R.store[2];
      container->push_back(p);
    }
    else
    {
      container->push_back(sink);
      break;
    }

    dtau = dtau-len;

    cur_i++;
  }
  return container;
}

