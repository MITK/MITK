/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "math.h"
#include "vtkActor.h"
#include <vtkSmartPointer.h>

struct LessDereference {
  template <class T>
      bool operator()(const T * lhs, const T * rhs) const {
    return *lhs < *rhs;
  }
};

namespace itk
{
  TractsToDWIImageFilter::TractsToDWIImageFilter()
  {
    this->SetNumberOfRequiredInputs(1);
  }

  TractsToDWIImageFilter::~TractsToDWIImageFilter()
  {
  }

  void TractsToDWIImageFilter::GenerateParticleGrid()
  {
    MITK_INFO << "Generating particle grid from fiber bundle";
    m_ParticleGrid = mitk::ParticleGrid::New();

    float* bounds = m_FiberBundle->GetBounds();
    int size[] = {(int)bounds[0],(int)bounds[1],(int)bounds[2]};
    m_ParticleGrid->SetSize(size);

    typedef itk::Point<float,3>                                                   ContainerPointType; //no need to init, is no smartpointer
    typedef itk::VectorContainer<unsigned int, ContainerPointType>                ContainerTractType;
    typedef itk::VectorContainer< unsigned int, ContainerTractType::Pointer >     ContainerType; //init via smartpointer

    ContainerType::Pointer tractContainer = m_FiberBundle->GetTractContainer();

    for (int i=0; i<tractContainer->Size(); i++)
    {
      ContainerTractType::Pointer tract = tractContainer->GetElement(i);
      for (int j=0; j<tract->Size(); j++)
      {
        vnl_vector_fixed<float,3> pos = tract->GetElement(j).GetVnlVector();
        vnl_vector_fixed<float,3> next;
        vnl_vector_fixed<float,3> dir;

        if (j<tract->Size()-1)
          next = tract->GetElement(j+1).GetVnlVector();
        else
          next = tract->GetElement(j-1).GetVnlVector();

        dir = next-pos;
        dir.normalize();
        mitk::Particle* p = new mitk::Particle();
        p->SetPosition(pos);
        p->SetDirection(dir);
        m_ParticleGrid->AddParticle(p);
      }
    }
  }

  void TractsToDWIImageFilter::GenerateData()
  {

    DWIImageType::Pointer originalDwiImage = dynamic_cast<DWIImageType*>(this->ProcessObject::GetInput(0));
    if (originalDwiImage.IsNull())
    {
      MITK_INFO << "no dwi image to extract b0 specified";
      return;
    }
    short* originalDwiImageBuffer = (short*) originalDwiImage->GetBufferPointer();

    if (this->m_FiberBundle.IsNotNull())
      this->GenerateParticleGrid();
    else
    {
      MITK_INFO << "no fiber bundle specified";
      return;
    }

    MITK_INFO << "reconstructing dwi-image from particle grid";

    float* bounds = m_FiberBundle->GetBounds();
    ImageRegion<3> region;
    region.SetSize(0, bounds[0]);
    region.SetSize(1, bounds[1]);
    region.SetSize(2, bounds[2]);

    m_Size[0] = bounds[0];
    m_Size[1] = bounds[1];
    m_Size[2] = bounds[2];

    mitk::Geometry3D::Pointer geom = this->m_FiberBundle->GetGeometry();

    int numDirections = m_GradientDirections->Size();
    int notNullDirections = 0;
    for (int i=0; i<numDirections; i++){
      GradientDirectionType dir = m_GradientDirections->GetElement(i);
      if (dir[0]!=0 || dir[1]!=0 || dir[2]!=0)
        notNullDirections++;
    }
    MITK_INFO << "Gradient directions: " << notNullDirections;
    MITK_INFO << "B0 images: " << numDirections-notNullDirections;

//    short *averageImageBuffer = new short[m_Size[0]*m_Size[1]*m_Size[2]];
//    for (int x=0; x<m_Size[0]; x++)
//      for (int y=0; y<m_Size[1]; y++)
//        for (int z=0; z<m_Size[2]; z++)
//        {
//          short val = 0;
//          for(unsigned int i=0; i<numDirections; i++)
//          {
//            int index = i + (x + m_Size[0]*(y+m_Size[1]*z))*numDirections;

//            GradientDirectionType dir = m_GradientDirections->GetElement(i);
//            if (dir[0]!=0 || dir[1]!=0 || dir[2]!=0)
//              val += originalDwiImageBuffer[index];
//          }
//          averageImageBuffer[x + m_Size[0]*(y+m_Size[1]*z)] = (short)val/notNullDirections;
//        }

    // allocate output image
    m_OutImage = static_cast< DWIImageType* >(this->ProcessObject::GetOutput(0));
    m_OutImage->SetSpacing( geom->GetSpacing() );   // Set the image spacing
    m_OutImage->SetOrigin( geom->GetOrigin() );     // Set the image origin
    itk::Matrix<double, 3, 3> matrix;
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
        matrix[j][i] = geom->GetMatrixColumn(i)[j];
    m_OutImage->SetDirection( matrix );  // Set the image direction
    m_OutImage->SetRegions( region );  // Set image region
    m_OutImage->SetVectorLength( numDirections );
    m_OutImage->Allocate();

    short* imageBuffer = (short*) m_OutImage->GetBufferPointer();

    m_BesselApproxCoeff = this->ComputeFiberCorrelation();

    int dist = 1;
    if (this->m_ParticleWidth==0)
    {
      double voxsize[3];
      voxsize[0] = geom->GetSpacing().GetElement(0);
      voxsize[1] = geom->GetSpacing().GetElement(1);
      voxsize[2] = geom->GetSpacing().GetElement(2);
      float vox_half;
      if(voxsize[0]<voxsize[1] && voxsize[0]<voxsize[2])
        vox_half = voxsize[0] / 2.0;
      else if (voxsize[1] < voxsize[2])
        vox_half = voxsize[1] / 2.0;
      else
        vox_half = voxsize[2] / 2.0;
      m_ParticleWidth = 0.5*vox_half;
    }
    float width = this->m_ParticleWidth*this->m_ParticleWidth;

    int bufferSize = m_Size[0]*m_Size[1]*m_Size[2]*numDirections;
    int counter = 0;
    short maxVal = 0;

    for (int x=0; x<m_Size[0]; x++)
      for (int y=0; y<m_Size[1]; y++)
        for (int z=0; z<m_Size[2]; z++)
        {
          vnl_vector_fixed<float, 3> pos;
          pos[0] = (float)x;
          pos[1] = (float)y;
          pos[2] = (float)z;
          for(unsigned int i=0; i<numDirections; i++)
          {
            int index = i + (x + m_Size[0]*(y+m_Size[1]*z))*numDirections;
            float val = 0;

            if (i>=notNullDirections){
              imageBuffer[index] = originalDwiImageBuffer[index];
              continue;
            }

            vnl_vector_fixed<double, 3> temp = m_GradientDirections->GetElement(i);

            // gradient direction
            vnl_vector_fixed<float, 3> n;
            n[0] = temp[0];
            n[1] = temp[1];
            n[2] = temp[2];

            for (int nx=-dist; nx<=dist; nx++)
              if((x+nx)>=0 && (x+nx)<m_Size[0])
              for (int ny=-dist; ny<=dist; ny++)
              if((y+ny)>=0 && (y+ny)<m_Size[1])
                for (int nz=-dist; nz<=dist; nz++)
                if((z+nz)>=0 && (z+nz)<m_Size[2]){
                  ParticleContainerType::Pointer container = m_ParticleGrid->GetParticleContainer(x+nx, y+ny, z+nz);
                  if(container.IsNotNull())
                  for (int j=0; j<container->Size(); j++)
                  {
                    mitk::Particle* particle = container->GetElement(j);
                    // Particle direction
                    vnl_vector_fixed<float, 3> ni = particle->GetDirection();
                    // Particle position
                    vnl_vector_fixed<float, 3> xi = particle->GetPosition();

                    vnl_vector_fixed<float, 3> diff = pos-xi;
                    float angle = fabs(dot_product(n, ni));

                    val += std::exp(-diff.squared_magnitude()/width)*(std::exp(-m_bD*angle*angle));
                  }
                }
            //MITK_INFO << val; averageImageBuffer[x + m_Size[0]*(y+m_Size[1]*z)]
            imageBuffer[index] = (short)(val*100+1);
            if (counter%((int)(bufferSize/10))==0)
              MITK_INFO << 100*counter/bufferSize << "%";
            counter++;
            if (imageBuffer[index] > maxVal)
              maxVal = imageBuffer[index];
          }
        }
    //delete(averageImageBuffer);
    for (int x=0; x<m_Size[0]; x++)
      for (int y=0; y<m_Size[1]; y++)
        for (int z=0; z<m_Size[2]; z++)
        {
          for(unsigned int i=0; i<numDirections; i++)
          {
            int index = i + (x + m_Size[0]*(y+m_Size[1]*z))*numDirections;
            if (i>=notNullDirections){
              imageBuffer[index] += maxVal;
              continue;
            }
          }
        }
  }

  float TractsToDWIImageFilter::Bessel(float x)
  {
    float y = x*x;
    float z = y*y;
    float erg = m_BesselApproxCoeff[0];
    erg += y*m_BesselApproxCoeff[1];
    erg += z*m_BesselApproxCoeff[2];
    erg += z*y*m_BesselApproxCoeff[3];
    return erg;
  }

  float* TractsToDWIImageFilter::ComputeFiberCorrelation(){

    float bD = m_bD;

    vnl_matrix_fixed<double, 3, QBALL_ODFSIZE> bDir =
        *itk::PointShell<QBALL_ODFSIZE, vnl_matrix_fixed<double, 3, QBALL_ODFSIZE> >::DistributePointShell();

    const int N = QBALL_ODFSIZE;

    vnl_matrix_fixed<double, N, N> C = bDir.transpose()*bDir;
    vnl_matrix_fixed<double, N, N> Q = C;
    for(int i=0; i<N; i++)
    {
      for(int j=0; j<N; j++)
      {
        C(i,j) = std::abs(C(i,j));
        Q(i,j) = exp(-bD * C(i,j) * C(i,j));
      }
    }

    vnl_matrix_fixed<double, N, N> P = Q*Q;

    std::vector<const double *> pointer;
    pointer.reserve(N*N);
    double * start = C.data_block();
    double * end =  start + N*N;
    for (double * iter = start; iter != end; ++iter)
    {
      pointer.push_back(iter);
    }
    std::sort(pointer.begin(), pointer.end(), LessDereference());

    vnl_vector_fixed<double,N*N> alpha;
    vnl_vector_fixed<double,N*N> beta;
    for (int i=0; i<N*N; i++) {
      alpha(i) = *pointer[i];
      beta(i)  = *(P.data_block()+(pointer[i]-start));
    }

    double nfac = sqrt(beta(N*N-1));
    beta = beta / (nfac*nfac);
    Q = Q / nfac;

    double sum = 0;
    for(int i=0; i<N; i++)
    {
      sum += Q(0,i);
    }
    // if left to default 0
    // then mean is not substracted in order to correct odf integral
    this->m_Meanval_sq = (sum*sum)/N;

    vnl_vector_fixed<double,N*N> alpha_0;
    vnl_vector_fixed<double,N*N> alpha_2;
    vnl_vector_fixed<double,N*N> alpha_4;
    vnl_vector_fixed<double,N*N> alpha_6;
    for(int i=0; i<N*N; i++)
    {
      alpha_0(i) = 1;
      alpha_2(i) = alpha(i)*alpha(i);
      alpha_4(i) = alpha_2(i)*alpha_2(i);
      alpha_6(i) = alpha_4(i)*alpha_2(i);
    }

    vnl_matrix_fixed<double, N*N, 4> T;
    T.set_column(0,alpha_0);
    T.set_column(1,alpha_2);
    T.set_column(2,alpha_4);
    T.set_column(3,alpha_6);

    vnl_vector_fixed<double,4> coeff = vnl_matrix_inverse<double>(T).pinverse()*beta;

    float* retval = new float[4];
    retval[0] = coeff(0);
    retval[1] = coeff(1);
    retval[2] = coeff(2);
    retval[3] = coeff(3);
    return retval;
  }
}
