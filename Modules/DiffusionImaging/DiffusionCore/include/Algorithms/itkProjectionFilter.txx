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
#ifndef _itkProjectionFilter_txx
#define _itkProjectionFilter_txx

#include "itkProjectionFilter.h"

#include "mitkProgressBar.h"
//#include <itkSignedMaurerDistanceMapImageFilter.h>

#define SEARCHSIGMA 10 /* length in linear voxel dimensions */
#define MAXSEARCHLENGTH (3*SEARCHSIGMA)


namespace itk
{


  ProjectionFilter::ProjectionFilter()
  {

  }

  ProjectionFilter::~ProjectionFilter()
  {


  }

  void ProjectionFilter::Project()
  {
    // Contains only code for the projection of FA data. The original FSL code contains some extra lines
    // For projection of other measurements than FA

    mitk::ProgressBar::GetInstance()->AddStepsToDo( 3 );

    Float4DImageType::Pointer data_4d_projected = Float4DImageType::New();
    data_4d_projected->SetRegions(m_AllFA->GetRequestedRegion());
    data_4d_projected->SetDirection(m_AllFA->GetDirection());
    data_4d_projected->SetSpacing(m_AllFA->GetSpacing());
    data_4d_projected->SetOrigin(m_AllFA->GetOrigin());
    data_4d_projected->Allocate();
    data_4d_projected->FillBuffer(0.0);

    Float4DImageType::SizeType size = m_AllFA->GetRequestedRegion().GetSize();
    long s0 = size[0], s1 = size[1], s2 = size[2], s3 = size[3];

    for(int t=0; t<s3; t++)
    {
      for(int z=1; z<s2-1; z++)
      {
        for(int y=1; y<s1-1; y++)
        {

          for(int x=1; x<s0-1; x++)
          {

            VectorImageType::IndexType ix;
            ix[0] = x; ix[1] = y; ix[2] = z;

            if(m_Skeleton->GetPixel(ix) != 0)
            {
              VectorImageType::PixelType dir = m_Directions->GetPixel(ix);

              Float4DImageType::IndexType ix4d;
              ix4d[0]=x; ix4d[1]=y; ix4d[2]=z; ix4d[3]=t;
              float maxval = m_AllFA->GetPixel(ix4d);
              float maxval_weighted = maxval;
              float exponentfactor = -0.5 * (dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]) / (float)(SEARCHSIGMA*SEARCHSIGMA);


              // No tubular structure here
              if(m_Tube->GetPixel(ix) == 0)
              {
                for(int iters=0;iters<2;iters++)
                {
                  float distance=0;

                  for(int d=1;d<MAXSEARCHLENGTH;d++)
                  {
                    int D=d;
                    if (iters==1) D=-d;

                    FloatImageType::IndexType ix3d;
                    int dx = x+dir[0]*D, dy = y+dir[1]*D, dz = z+dir[2]*D;
                    ix3d[0] = dx; ix3d[1] = dy; ix3d[2] = dz;

                    if(dx<0 || dy<0 || dz<0
                      || (dx>=s0 && dy<=s1 && dz<=s2))
                    {
                      d=MAXSEARCHLENGTH;
                    }
                    else if(m_DistanceMap->GetPixel(ix3d)>=distance)
                    {
                      float distanceweight = exp(d * d * exponentfactor);
                      distance = m_DistanceMap->GetPixel(ix3d);

                      ix4d[0]=x+dir[0]*D; ix4d[1]=y+dir[1]*D; ix4d[2]=z+dir[2]*D; ix4d[3]=t;

                      if(distanceweight*m_AllFA->GetPixel(ix4d)>maxval_weighted)
                      {
                        maxval = m_AllFA->GetPixel(ix4d);
                        maxval_weighted =  maxval*distanceweight;
                      }
                    }
                    else{
                      d=MAXSEARCHLENGTH;
                    }



                  }

                }
              }

              // Tubular structure
              else
              {
                for(int dy=-MAXSEARCHLENGTH; dy<=MAXSEARCHLENGTH;dy++) {
                  for(int dx=-MAXSEARCHLENGTH; dx<=MAXSEARCHLENGTH; dx++) {

                    float distanceweight = exp(-0.5 * (dx*dx+dy*dy) / (float)(SEARCHSIGMA*SEARCHSIGMA) );
                    float r=sqrt((float)(dx*dx+dy*dy));

                    if (r>0)
                    {
                      int allok=1;

                      for(float rr=1; rr<=r+0.1; rr++) /* search outwards from centre to current voxel - test that distancemap always increasing */
                      {

                        int dx1=round(rr*dx/r);
                        int dy1=round(rr*dy/r);
                        int dx2=round((rr+1)*dx/r);
                        int dy2=round((rr+1)*dy/r);

                        RealImageType::IndexType ix1, ix2;
                        ix1[0]=x+dx1; ix1[1]=y+dy1; ix1[2]=z;
                        ix2[0]=x+dx2; ix2[1]=y+dy2; ix2[2]=z;
                        if(m_DistanceMap->GetPixel(ix1) > m_DistanceMap->GetPixel(ix2) )
                        {
                          allok=0;
                        }

                      }

                      ix4d[0]=x+dx; ix4d[1]=y+dy, ix4d[2]=z; ix4d[3]=t;
                      if( allok && (distanceweight * m_AllFA->GetPixel(ix4d) > maxval_weighted) )
                      {
                        maxval = m_AllFA->GetPixel(ix4d);
                        maxval_weighted = maxval * distanceweight;
                      }


                    }

                  }
                }
              }

              ix4d[0]=x; ix4d[1]=y; ix4d[2]=z; ix4d[3]=t;

              data_4d_projected->SetPixel(ix4d, maxval);

            }
          }
        }
      }
    }

    m_Projections = data_4d_projected;



  }





}
#endif // _itkProjectionFilter_txx
