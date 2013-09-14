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


#include "mitkDopplerToStrainRateFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"
#include "mitkPlaneGeometry.h"
#include "mitkLegacyAdaptors.h"

#include <iostream>
#include <string>
#include <ipFunc/mitkIpFunc.h>


void mitk::DopplerToStrainRateFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<<"GenerateOutputInformation()");

  unsigned int i;
  unsigned int *tmpDimensions = new unsigned int[input->GetDimension()];

  for(i=0;i<input->GetDimension();++i)
    tmpDimensions[i]=input->GetDimension(i);

  //@todo maybe we should shift the following somehow in ImageToImageFilter
  output->Initialize(input->GetPixelType(),
    input->GetDimension(),
    tmpDimensions,
    input->GetNumberOfChannels());

  output->GetSlicedGeometry()->SetSpacing(input->GetSlicedGeometry()->GetSpacing());

  //output->GetSlicedGeometry()->SetGeometry2D(mitk::Image::BuildStandardPlaneGeometry2D(output->GetSlicedGeometry(), tmpDimensions).GetPointer(), 0);
  //output->GetSlicedGeometry()->SetEvenlySpaced();
  //set the timebounds - after SetGeometry2D, so that the already created PlaneGeometry will also receive this timebounds.
  output->GetSlicedGeometry()->SetTimeBounds(input->GetSlicedGeometry()->GetTimeBounds());

  output->SetPropertyList(input->GetPropertyList()->Clone());


  delete [] tmpDimensions;

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::DopplerToStrainRateFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();


  mitk::Point3iProperty::Pointer pointProp;
  pointProp = dynamic_cast<mitk::Point3iProperty*>(input->GetProperty("ORIGIN").GetPointer());
  if (pointProp.IsNotNull() )
  {
    m_Origin = pointProp->GetValue();
  }

  MITK_INFO << "compute Strain Rate Image .... " << std::endl
           << "  origin[0]=" << m_Origin[0] << " origin[1]=" << m_Origin[1] << " origin[2]=" << m_Origin[2] << std::endl
           << "  distance=" << m_Distance << std::endl
           << "  NoStrainIntervall=" << m_NoStrainInterval << std::endl;

  const Vector3D & spacing = input->GetSlicedGeometry()->GetSpacing();
  //  MITK_INFO << "   in: xres=" << spacing[0] << " yres=" << spacing[1] << " zres=" << spacing[2] << std::endl;


  mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
  timeSelector->SetInput(input);

  mitkIpPicDescriptor* picStrainRate;
  picStrainRate = mitkIpPicNew();
  picStrainRate->dim=3;
  picStrainRate->bpe  = output->GetPixelType().GetBpe();
  //picStrainRate->type = output->GetPixelType().GetType();
  picStrainRate->n[0] = output->GetDimension(0);
  picStrainRate->n[1] = output->GetDimension(1);
  picStrainRate->n[2] = output->GetDimension(2);
  picStrainRate->data=malloc(_mitkIpPicSize(picStrainRate));


  int xDim = picStrainRate->n[0];
  int yDim = picStrainRate->n[1];
  int zDim = picStrainRate->n[2];
  long slice_size = xDim*yDim;
  long vol_size = slice_size*zDim;


  mitkIpPicDescriptor *picDoppler;

  int x,y,z;//,time;        // loop-counter
  int strainRate;            // the computed Strain Rate
  int v1,v2;                // velocity and Point p1 and p2

  float alpha;                // the beam-angle, angle betwen current point and beam-point
  float dx=0, dy=0;                // projection of this->distance to x- and y-axis
  int x1;          // a square, where the velocity v1 lies in
  int y1;          // the points are used for interpolation


  int minStrainRate=128, maxStrainRate=128;

  int n, nmax;
  int t, tmax;

  t = output->GetRequestedRegion().GetIndex(3);
  n = output->GetRequestedRegion().GetIndex(4);
  MITK_INFO << "t = " <<t << " n = " << n << std::endl;

  tmax = t + output->GetRequestedRegion().GetSize(3);
  nmax = n + output->GetRequestedRegion().GetSize(4);
  MITK_INFO << "tmax = "<< tmax << " nmax = " << nmax << std::endl;

  if (m_Distance<1) m_Distance=1;

  for(;n<nmax;n++)//output->GetNumberOfChannels();++n)
  {
    timeSelector->SetChannelNr(n);
    MITK_INFO << "computing chanel n = " << n << std::endl;

    for(t=0;t<tmax;t++)
    {
      MITK_INFO << "computing time slot t = " << t << std::endl;
      timeSelector->SetTimeNr(t);
      timeSelector->Update();

      // Cast to pic descriptor for the timeSelector image
      mitkIpPicDescriptor* timeSelectorPic = mitkIpPicNew();
      ImageWriteAccessor imageAccess(timeSelector->GetOutput());
      CastToIpPicDescriptor( timeSelector->GetOutput(), &imageAccess, timeSelectorPic );

      _mitkIpPicFreeTags(picStrainRate->info->tags_head);
      picStrainRate->info->tags_head = _mitkIpPicCloneTags(timeSelectorPic->info->tags_head);

//#define       WRITE_ANGLE_PIC
#ifdef WRITE_ANGLE_PIC
      mitkIpPicDescriptor *anglePic;
      mitkIpBool_t isAnglePicWritten = mitkIpFalse;

      anglePic = mitkIpPicNew();
      anglePic->type = mitkIpPicInt;
      anglePic->bpe = 8;
      anglePic->dim = 2;
      anglePic->n[0] = xDim;
      anglePic->n[1] = yDim;
      anglePic->data = (mitkIpInt1_t *)calloc(xDim*yDim,sizeof(mitkIpInt1_t));
#endif

      picDoppler = timeSelectorPic;
      picDoppler = mitkIpFuncGausF( picDoppler, 5,2 , mitkIpFuncBorderOld )  ;



      for (z=0 ; z<zDim ; z++) {


        for (y=1; y<yDim; y++) {

          if (y<m_Origin[1]) continue;  // cannot compute StrainRate above Transducer-Position

          for (x=0; x<xDim; x++) {

            if ((m_Origin[0] - x)==0) {  // winkelhalbierende

              int yDistanceInUnits = (int)( m_Distance/spacing[1]);
              int yTmp = ( (y-yDistanceInUnits)<0 )  ? 0 : (y-yDistanceInUnits);
              v1 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + yTmp*xDim + x] ;
              v2 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y*xDim + x] ;

            } else {

              // compute angle to transducer position
              // m_Origin is given in units, not in mm
              alpha = atan( (float) (m_Origin[0] - x)  / (float) (m_Origin[1] - y) );

              // m_Distance is given in mm
              dx = -sin(alpha) * m_Distance;
              dy = -cos(alpha) * m_Distance;


              // convert to units
              dx = dx/spacing[0];
              dy = dy/spacing[1];

              //#define WEIGTHED
#ifdef WEIGTHED
              weightX = dx - floor(dx);
              weightY = dy - floor(dy);

              dxFloor = (int) floor(dx);
              dyFloor = (int) floor(dy);

              x1 = x + dxFloor;          // lower left
              y1 = y + dyFloor;

              x2 = x + (dxFloor+1);      // lower right
              y2 = y + dyFloor;

              x3 = x + (dxFloor+1);      // upper right
              y3 = y + (dyFloor+1);

              x4 = x + dxFloor;          // upper left
              y4 = y + (dyFloor+1);

              vTmp1 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y1*xDim + x1];
              vTmp2 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y2*xDim + x2];
              vTmp3 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y3*xDim + x3];
              vTmp4 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y4*xDim + x4];

              v1 = (int) ((1-weightX)*(1-weightY)*vTmp1 + weightX*(1-weightY)*vTmp2 + weightX*weightY*vTmp3 + (1-weightX)*weightY*vTmp4);
              v2 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y*xDim + x] ;
#else
              x1 = (int)(x + dx);
              y1 = (int)(y + dy);
              if (y1<0) y1=0;

#endif
              v1 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y1*xDim + x1];
              v2 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y*xDim + x] ;
            }

            if (v1==128) {  // schaue in Gegenrichtung, falls keine SRI hier berechenbar
              x1 = (int)(x - dx);
              y1 = (int)(y - dy);
              if (y1>yDim) y1=yDim;
              //swap v1 and v2, otherwise StrainRate is calculate in false direction
              v1 = v2;
              v2 = ((mitkIpUInt1_t *)picDoppler->data)[z*slice_size + y1*xDim + x1];
            }

            if (   (v1==0 ) || (v2==0)  ||  // wenn keine Geschwindigkeit vorhanden
              // oder wenn nur ganz kleine Geschwindigkeit vorhanden
              (v1>=(128-m_NoStrainInterval) && v1<=(128+m_NoStrainInterval)) ||
              (v2>=(128-m_NoStrainInterval) && v2<=(128+m_NoStrainInterval))) {

                strainRate = 128;  // this means no deformation
                // this is neccessay due to the Cyl2Cart filter

              } else {
                strainRate = (int)( (v1 - v2)/2 + 128 );
                if (strainRate==128) strainRate=129;
              }

              if (strainRate < minStrainRate && strainRate > 0 )
                minStrainRate = strainRate;

              if (strainRate > maxStrainRate)
                maxStrainRate = strainRate;

              if (strainRate<0 ) {
                //strainRate = -strainRate;
                MITK_INFO << " error: neg. strainRate ... exit() " << std::endl
                         << " x=" << x << " y=" << y << " z=" << z << std::endl
                         << " v1=" << v1 << " v2=" << v2 << " dist=" << m_Distance << std::endl
                         << " sr=" << strainRate  << std::endl;
                exit(0);
              }


              ((mitkIpUInt1_t *)picStrainRate->data)[t*vol_size + z*slice_size + y*xDim + x]=strainRate;

              // cout << "z: " << z << " y: " << y << " x: " << x << " strainrate: " << strainRate << endl;

#ifdef WRITE_ANGLE_PIC
              //            if (!isAnglePicWritten)
              //          ((mitkIpInt1_t *)anglePic->data)[y*xDim + x] = (int) ( (alpha/1.6)*128);
              if (!isAnglePicWritten)
                ((mitkIpInt1_t *)anglePic->data)[y*xDim + x] = (int) ( dx);

#endif

          }  // x

        } // y

        //isAnglePicWritten = mitkIpTrue;

      } // z

      //isStrainComputed = mitkIpTrue;
      std::string filenameD;
      filenameD ="doppler.pic";
      mitkIpPicPut(const_cast<char *>(filenameD.c_str()),picDoppler);

#define WRITE_STRAIN_PIC
#ifdef WRITE_STRAIN_PIC
      char tmpfilename[100];
      sprintf(tmpfilename,"strain%d.pic",t);;
      mitkIpPicPut(tmpfilename,picStrainRate);
#endif


#ifdef WRITE_ANGLE_PIC
      std::string filename2;
      filename2="angle.pic";
      mitkIpPicPut(const_cast<char *>(filename2.c_str()),anglePic);
#endif
      ((mitkIpUInt1_t *)picStrainRate->data)[0]=0;
      ((mitkIpUInt1_t *)picStrainRate->data)[1]=255;

      output->SetVolume(picStrainRate->data, t, n);
    }
  }

  mitkIpPicFree(picStrainRate);

#define WRITE_STRAIN_PIC
#ifdef WRITE_STRAIN_PIC
      // Get the StrainRate ipPic descriptor
      picStrainRate = mitkIpPicNew();
      ImageWriteAccessor imageAccess(output);
      CastToIpPicDescriptor( output, &imageAccess, picStrainRate );

      std::string filename;
      filename ="strain.pic";
      mitkIpPicPut(const_cast<char *>(filename.c_str()),picStrainRate);
#endif

  MITK_INFO << "Strain Rate Image computed.... " << std::endl
           << "  minStrainRate: " << minStrainRate << std::endl
           << "  maxStrainRate: " << maxStrainRate << std::endl;
}

mitk::DopplerToStrainRateFilter::DopplerToStrainRateFilter()
: m_Distance(10), m_NoStrainInterval(2)
{

  m_Origin[0] = 0;
  m_Origin[1] = 0;
  m_Origin[2] = 0;

}

float mitk::DopplerToStrainRateFilter::GetLimit()
{  return (128/m_Distance);
}


mitk::DopplerToStrainRateFilter::~DopplerToStrainRateFilter()
{

}

void mitk::DopplerToStrainRateFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::ImageToImageFilter::InputImagePointer input =
    const_cast< mitk::ImageToImageFilter::InputImageType * > ( this->GetInput() );
  mitk::Image::Pointer output = this->GetOutput();

  Image::RegionType requestedRegion;
  requestedRegion = output->GetRequestedRegion();
  requestedRegion.SetIndex(0, 0);
  requestedRegion.SetIndex(1, 0);
  requestedRegion.SetIndex(2, 0);
  //requestedRegion.SetIndex(3, 0);
  //requestedRegion.SetIndex(4, 0);
  requestedRegion.SetSize(0, input->GetDimension(0));
  requestedRegion.SetSize(1, input->GetDimension(1));
  requestedRegion.SetSize(2, input->GetDimension(2));
  //requestedRegion.SetSize(3, output->GetDimension(3));
  //requestedRegion.SetSize(4, output->GetNumberOfChannels());

  input->SetRequestedRegion( & requestedRegion );
}
