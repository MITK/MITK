/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $ $
Version:   $ $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkSkeletonizationFilter_txx
#define _itkSkeletonizationFilter_txx

#include "itkSkeletonizationFilter.h"
#include "mitkProgressBar.h"
#include <limits>
#include <time.h> 

namespace itk
{

  template< class TInputImage, class TOutputImage >
  SkeletonizationFilter<TInputImage, TOutputImage>::SkeletonizationFilter()
  {
    m_DirectionImage = VectorImageType::New();
  }

  template< class TInputImage, class TOutputImage >
  SkeletonizationFilter<TInputImage, TOutputImage>::~SkeletonizationFilter()
  {    

  }

 

 
  template< class TInputImage, class TOutputImage >
  void SkeletonizationFilter<TInputImage, TOutputImage>::GenerateData()
  {
    //----------------------------------------------------------------------//
    //  Progress bar                                                        //
    //----------------------------------------------------------------------//
    mitk::ProgressBar::GetInstance()->AddStepsToDo( 3 );    

    std::cout << "Skeletonize" << std::endl;
    CalculatePerpendicularDirections();
  }



  template< class TInputImage, class TOutputImage >
  void SkeletonizationFilter<TInputImage, TOutputImage>::CalculatePerpendicularDirections()
  {      

    const InputImageType* faImage = this->GetInput();
    typename InputImageType::SizeType size = faImage->GetRequestedRegion().GetSize();

    //typename RealImageType::SizeType size = m_FaImage->GetRequestedRegion().GetSize();

    m_DirectionImage->SetRegions(faImage->GetRequestedRegion());
    m_DirectionImage->SetDirection(faImage->GetDirection());
    m_DirectionImage->SetSpacing(faImage->GetSpacing());    
    m_DirectionImage->SetOrigin(faImage->GetOrigin());
    m_DirectionImage->Allocate();
    m_DirectionImage->FillBuffer(0.0);
    

    for(int z=1; z<size[2]-1; z++) for(int y=1; y<size[1]-1; y++) for(int x=1; x<size[0]-1; x++)
    { 
      typename InputImageType::IndexType ix;
      ix[0]=x; ix[1]=y; ix[2]=z;
      float theval = faImage->GetPixel(ix);

      if(theval != 0)
      {      

        /* Calculate point of gravity. We will consider each 3x3x3 neighbourhood as a unit cube. The center
         * point of each voxel will be a multiplicative of 1/6. The center of the unit cube is 3/6 = 1/2/
         */
        
        float cogX = 0.0; float cogY = 0.0; float cogZ = 0.0; float sum = 0.0; float l;
        int vecX = 0; int vecY = 0; int vecZ = 0;
        
        for(int dz=-1; dz<=1; dz++) for(int dy=-1; dy<=1; dy++) for(int dx=-1; dx<=1;dx++)
        {
          typename InputImageType::IndexType p;
          p[0] = x+dx; p[1] = y+dy; p[2] = z+dz;
          float mass = faImage->GetPixel(p);

          sum += mass;
          cogX += (float)dx*mass; cogY += (float)dy*mass; cogZ += (float)dz*mass;           
        }

        cogX /= sum; cogY /= sum; cogZ /= sum;
        l = sqrt(cogX*cogX + cogY*cogY + cogZ*cogZ);

        if (l > 0.1) /* is CofG far enough away from centre voxel? */
        {
          vecX = std::max(std::min(round(cogX/l),1),-1);
          vecY = std::max(std::min(round(cogY/l),1),-1);
          vecZ = std::max(std::min(round(cogZ/l),1),-1);          
        }
        else
          // Find direction of max curvature
        { 
          
          float maxcost=0, centreval=2*theval;
          for(int zz=0; zz<=1; zz++) // note - starts at zero as we're only searching half the voxels 
          {
            for(int yy=-1; yy<=1; yy++)
            {
              for(int xx=-1; xx<=1; xx++)
              {
                if ( (zz==1) || (yy==1) || ((yy==0)&&(xx==1)) )
                {
                  float weighting = pow( (float)(xx*xx+yy*yy+zz*zz) , (float)-0.7 ); // power is arbitrary: maybe test other functions here 
                  

                  typename InputImageType::IndexType i,j;
                  i[0] = x+xx; i[1] = y+yy; i[2] = z+zz;
                  j[0] = x-xx; j[1] = y-yy; j[2] = z-zz;
                  float cost = weighting * ( centreval 
                    - (float)faImage->GetPixel(i)
                    - (float)faImage->GetPixel(j));

                  if (cost>maxcost)
                  {
                    maxcost=cost;
                    vecX=xx;
                    vecY=yy;
                    vecZ=zz;
                  }
                }
              }
            }
          }         
        }

        VectorType vec;
        vec[0] = vecX; vec[1] = vecY; vec[2]=vecZ;
        m_DirectionImage->SetPixel(ix, vec);        
      
      }     
    }

    mitk::ProgressBar::GetInstance()->Progress();


    // Smooth m_DirectionImage and store in directionSmoothed by finding the
    // mode in a 3*3 neighbourhoud
    VectorImageType::Pointer directionSmoothed = VectorImageType::New();
    directionSmoothed->SetRegions(faImage->GetRequestedRegion());
    directionSmoothed->SetDirection(faImage->GetDirection());
    directionSmoothed->SetSpacing(faImage->GetSpacing());    
    directionSmoothed->SetOrigin(faImage->GetOrigin());
    directionSmoothed->Allocate();

    VectorImageType::PixelType p;
    p[0]=0; p[1]=0; p[2]=0; 
    directionSmoothed->FillBuffer(p);

  

    for(int z=1; z<size[2]-1; z++) for(int y=1; y<size[1]-1; y++) for(int x=1; x<size[0]-1; x++)
    {   
      VectorImageType::IndexType ix;
      ix[0]=x; ix[1]=y; ix[2]=z;  

      // Find the vector that occured most
      int* localsum = new int[27];
      int localmax=0, xxx, yyy, zzz;

      for(int zz=0; zz<27; zz++) localsum[zz]=0;

      for(int zz=-1; zz<=1; zz++) for(int yy=-1; yy<=1; yy++) for(int xx=-1; xx<=1; xx++)
      {
        VectorImageType::IndexType i;
        i[0] = x+xx; i[1] = y+yy; i[2] = z+zz;
        VectorType v = m_DirectionImage->GetPixel(i);
        xxx = v[0];
        yyy = v[1];
        zzz = v[2];
       
        localsum[(1+zzz)*9+(1+yyy)*3+1+xxx]++;
        localsum[(1-zzz)*9+(1-yyy)*3+1-xxx]++;
      }

      for(int zz=-1; zz<=1; zz++) for(int yy=-1; yy<=1; yy++) for(int xx=-1; xx<=1; xx++)
      {
        if (localsum[(1+zz)*9+(1+yy)*3+1+xx]>localmax)
        {
          localmax=localsum[(1+zz)*9+(1+yy)*3+1+xx];
          VectorType v;
          v[0] = xx; v[1] = yy; v[2] = zz;
          directionSmoothed->SetPixel(ix, v);
        }
      }

      delete localsum;
      
    }

    m_DirectionImage = directionSmoothed;

    mitk::ProgressBar::GetInstance()->Progress();

    // Do non-max-suppression in the direction of perp and set as output of the filter
    typename OutputImageType::Pointer outputImg = OutputImageType::New();
    outputImg->SetRegions(faImage->GetRequestedRegion());
    outputImg->SetDirection(faImage->GetDirection());
    outputImg->SetSpacing(faImage->GetSpacing());    
    outputImg->SetOrigin(faImage->GetOrigin());
    outputImg->Allocate();
    outputImg->FillBuffer(0.0);  

    for(int z=1; z<size[2]-1; z++) for(int y=1; y<size[1]-1; y++) for(int x=1; x<size[0]-1; x++)
    {

      typename InputImageType::IndexType ix;
      ix[0]=x; ix[1]=y; ix[2]=z;
      
      float theval = faImage->GetPixel(ix);
      VectorType v = directionSmoothed->GetPixel(ix);
      
      typename VectorImageType::IndexType i;

      i[0] = x-v[0]; i[1] = y-v[1]; i[2] = z-v[2];
      float min = faImage->GetPixel(i);

      i[0] = x+v[0]; i[1] = y+v[1]; i[2] = z+v[2];
      float plus = faImage->GetPixel(i);

      i[0] = x-2*v[0]; i[1] = y-2*v[1]; i[2] = z-2*v[2];
      float minmin = faImage->GetPixel(i);

      i[0] = x+2*v[0]; i[1] = y+2*v[1]; i[2] = z+2*v[2];
      float plusplus = faImage->GetPixel(i);

      if( ((v[0]!=0) || (v[1]!=0) || (v[2]!=0)) &&
           theval >= plus && theval >  min && theval >= plusplus && theval >  minmin  )
      {
        outputImg->SetPixel(ix, theval);
      }
     
    }
   
    

    Superclass::SetNthOutput( 0, outputImg );
    mitk::ProgressBar::GetInstance()->Progress();
   

  }

  
  template< class TInputImage, class TOutputImage >
  itk::VectorImage<int, 3>::Pointer SkeletonizationFilter<TInputImage, TOutputImage>::GetGradientImage()
  {
    GradientImageType::Pointer gradImg = GradientImageType::New();

    if(m_DirectionImage.IsNotNull())
    {
      gradImg->SetSpacing(m_DirectionImage->GetSpacing());
      gradImg->SetOrigin(m_DirectionImage->GetOrigin());
      gradImg->SetDirection(m_DirectionImage->GetDirection());
      gradImg->SetRegions(m_DirectionImage->GetLargestPossibleRegion().GetSize());
      gradImg->SetVectorLength(3);
      gradImg->Allocate();


      VectorImageType::SizeType size = m_DirectionImage->GetLargestPossibleRegion().GetSize();

      for(int i=0; i<size[0]; i++)
      {
        for(int j=0; j<size[1]; j++)
        {
          for(int k=0; k<size[2]; k++)
          {
            itk::Index<3> ix;
            ix[0] = i;
            ix[1] = j;
            ix[2] = k;

            VectorType vec = m_DirectionImage->GetPixel(ix);

            itk::VariableLengthVector<int> pixel;
            pixel.SetSize(3);
            pixel.SetElement(0, vec.GetElement(0));
            pixel.SetElement(1, vec.GetElement(1));
            pixel.SetElement(2, vec.GetElement(2));

            gradImg->SetPixel(ix, pixel);

          }
        }
      }

    }




    return gradImg;
  }
  
}
#endif // _itkSkeletonizationFilter_txx
