/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkSeedsImage.h>

#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkDrawOperation.h>
#include <mitkImageAccessByItk.h>
#include <mitkInteractionConst.h>

#include <mitkRenderWindow.h>//*\todo remove later, when update ok!

void mitk::SeedsImage::ExecuteOperation(mitk::Operation* operation)
{
  //mitkCheckOperationTypeMacro(SeedsOperation, operation, seedsOp);

  spacing = this->GetGeometry()->GetSpacing();
  for(int i=0; i<this->GetDimension(); i++)
    orig_size[i] = this->GetDimension(i);

  mitk::DrawOperation * seedsOp = dynamic_cast<mitk::DrawOperation*>( operation );
  if ( seedsOp != NULL )
  {
    m_DrawState = seedsOp->GetDrawState();
    m_Radius = seedsOp->GetRadius();

    switch (operation->GetOperationType())
    {
    case mitk::OpADD:
      {	
				point = seedsOp->GetPoint();
				last_point = point;
 				AccessByItk(this, AddSeedPoint);
        break;
      }
    case mitk::OpMOVE:
      {
				point = seedsOp->GetPoint();
 				AccessByItk(this, AddSeedPoint);
 				AccessByItk(this, PointInterpolation);
				last_point = point;
        break;
      }
    case mitk::OpREMOVE:
      {
				last_point = point;
        break;
      }
		}
    this->Modified();
    mitk::RenderWindow::UpdateAllInstances(); //*todo has to be done here, cause of update-pipeline not working yet
  }
}


template < typename TPixel, unsigned int VImageDimension >
void mitk::SeedsImage::AddSeedPoint(itk::Image< TPixel, VImageDimension >* itkImage)
{	
	itk::ImageRegionIterator<itk::Image < TPixel, VImageDimension > >
  iterator(itkImage, itkImage->GetRequestedRegion());
	itk::Index<VImageDimension>baseIndex;
	itk::Index<VImageDimension>setIndex;
	for (int i=0; i<3; i++) baseIndex[i] = (int)ceil(point[i]/spacing[i]);
	
	// setting a sphere around the point
	for(int z = baseIndex[2] - m_Radius; z <= baseIndex[2] + m_Radius; ++z){
		for(int y = baseIndex[1] - m_Radius; y <= baseIndex[1] + m_Radius; ++y){
			for(int x = baseIndex[0] - m_Radius; x <= baseIndex[0] + m_Radius; ++x){
				delta_x = abs(x - baseIndex[0])*spacing[0];
				delta_y = abs(y - baseIndex[1])*spacing[1];
				delta_z = abs(z - baseIndex[2])*spacing[2];
				sphere_distance = sqrt((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z));
				if (sphere_distance <= m_Radius){
					// check -> is the point inside the image?
					if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1]) && (z>=0) && (z<orig_size[2])){
						setIndex[2]=z;
						setIndex[1]=y;
						setIndex[0]=x;
						iterator.SetIndex(setIndex);
						iterator.Set(m_DrawState);
					}
				}
			}
		}
  }
}


template < typename TPixel, unsigned int VImageDimension >
void mitk::SeedsImage::PointInterpolation(itk::Image< TPixel, VImageDimension >* itkImage)
{
	itk::ImageRegionIterator<itk::Image < TPixel, VImageDimension > >
  iterator(itkImage, itkImage->GetRequestedRegion());
	itk::Index<VImageDimension>pointIndex;
	itk::Index<VImageDimension>last_pointIndex;
	itk::Index<VImageDimension>baseIndex;
	itk::Index<VImageDimension>setIndex;

	// coordinate transformation from physical coordinates to index coordinates
	for (int i=0; i<3; i++) pointIndex[i] = (int)ceil(point[i]/spacing[i]);
	for (int i=0; i<3; i++) last_pointIndex[i] = (int)ceil(last_point[i]/spacing[i]);
						
	// calculation of the distance between last_point and point
	delta_x = abs(last_point[0] - point[0]);
	delta_y = abs(last_point[1] - point[1]);
	delta_z = abs(last_point[2] - point[2]);
	double point_distance;
	point_distance = sqrt((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z));
	
	int distance_step = m_Radius;
	int distance_iterator = distance_step;
	float t;

	// check - is there a gap between the points?
	if(point_distance > distance_step){
		// fill the gap
		while (distance_iterator < point_distance){
			t = distance_iterator/point_distance;
			// interpolation between the points
			for (int i=0; i<3; i++) baseIndex[i] = (int)(((1-t)*last_pointIndex[i]) + (t*pointIndex[i]));
			
			for(int z = baseIndex[2] - m_Radius; z <= baseIndex[2] + m_Radius; ++z){
				for(int y = baseIndex[1] - m_Radius; y <= baseIndex[1] + m_Radius; ++y){
					for(int x = baseIndex[0] - m_Radius; x <= baseIndex[0] + m_Radius; ++x){
						delta_x = abs(x - baseIndex[0])*spacing[0];
						delta_y = abs(y - baseIndex[1])*spacing[1];
						delta_z = abs(z - baseIndex[2])*spacing[2];
						sphere_distance = sqrt((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z));
						if (sphere_distance <= m_Radius){
							// is the point inside the image?
							if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1]) && (z>=0) && (z<orig_size[2])){
								setIndex[2]=z;
								setIndex[1]=y;
								setIndex[0]=x;
								iterator.SetIndex(setIndex);
								iterator.Set(m_DrawState);
							}
						}
					}
				}
			}			
		distance_iterator = distance_iterator + distance_step;
		}
	}
}
