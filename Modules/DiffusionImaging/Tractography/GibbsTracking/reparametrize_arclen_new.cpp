#include <mitkParticle.h>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

#define PI M_PI

float squareNorm( vnl_vector_fixed<float, 3> v )
{
  return v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
}

itk::VectorContainer<unsigned int, mitk::Particle::Pointer>::Pointer mexFunction(vector< mitk::Particle::Pointer >* particleContainer, float len)
{
  //vector< mitk::Particle::Pointer >* outContainer = new::vector< mitk::Particle::Pointer >();
  itk::VectorContainer<unsigned int, mitk::Particle::Pointer>::Pointer container =
      itk::VectorContainer<unsigned int, mitk::Particle::Pointer>::New();

  int numPoints = particleContainer->size();
  if(numPoints<2)
    return container;

  mitk::Particle::Pointer source = particleContainer->at(0);
  mitk::Particle::Pointer sink = particleContainer->at(numPoints-1);

  float Leng = 0;

  container->InsertElement(container->Size(), source);

  float dtau = 0;
  int cur_p = 1;
  int cur_i = 1;
  vnl_vector_fixed<float, 3> dR;
  float normdR;

  for (;;)
  {
    while (dtau <= len && cur_p < numPoints)
    {
      dR  = particleContainer->at(cur_p)->GetPosition() - particleContainer->at(cur_p-1)->GetPosition(); // vector between two particle centers
      normdR = squareNorm(dR);  // square length of above vector
      dtau += normdR;
      Leng += normdR;
      cur_p++;
    }

    if (dtau >= len)  // if particles reach next voxel
    {
      // proposal = current particle position - (current p - last p)*(current fibre length - len???)/(norm current p-last p)
      //particles_proposal[cur_i].R = particles[cur_p-1].R - dR*( (dtau-len)/normdR );
      mitk::Particle::Pointer newSegment = mitk::Particle::New();
      newSegment->SetPosition(particleContainer->at(cur_p-1)->GetPosition() - dR*( (dtau-len)/normdR ));
      dR.normalize();
      newSegment->SetDirection(dR);
      newSegment->SetWidth(particleContainer->at(cur_p-1)->GetWidth());
      container->InsertElement(container->Size(), newSegment);
    }
    else
    {
      container->InsertElement(container->Size(), sink);
      break;
    }

    dtau = dtau-len;

    cur_i++;
  }
  return container;
}

