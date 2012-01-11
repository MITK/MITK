/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkAngleCorrectByPointFilter.h"
#include "mitkIpPic.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"

mitk::AngleCorrectByPointFilter::AngleCorrectByPointFilter() : m_PreferTransducerPositionFromProperty(true)
{
  m_Center.Fill(0);
  m_TransducerPosition.Fill(0);
}

mitk::AngleCorrectByPointFilter::~AngleCorrectByPointFilter()
{

}

void mitk::AngleCorrectByPointFilter::GenerateOutputInformation()
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
  mitk::PixelType scalarPType = MakeScalarPixelType<ScalarType>();
  output->Initialize(scalarPType,
    input->GetDimension(),
    tmpDimensions,
    input->GetNumberOfChannels());

  output->GetSlicedGeometry()->SetSpacing(input->GetSlicedGeometry()->GetSpacing());

  //output->GetSlicedGeometry()->SetGeometry2D(mitk::Image::BuildStandardPlaneGeometry2D(output->GetSlicedGeometry(), tmpDimensions).GetPointer(), 0);
  //output->GetSlicedGeometry()->SetEvenlySpaced();
  //set the timebounds - after SetGeometry2D, so that the already created PlaneGeometry will also receive this timebounds.
  //@fixme!!! will not work for not evenly timed data!
  output->GetSlicedGeometry()->SetTimeBounds(input->GetSlicedGeometry()->GetTimeBounds());

  output->GetTimeSlicedGeometry()->InitializeEvenlyTimed(output->GetSlicedGeometry(), output->GetTimeSlicedGeometry()->GetTimeSteps());

  output->SetPropertyList(input->GetPropertyList()->Clone());    


  delete [] tmpDimensions;

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::AngleCorrectByPointFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();


  if(m_PreferTransducerPositionFromProperty)
  {
    mitk::Point3iProperty::Pointer pointProp;
    pointProp = dynamic_cast<mitk::Point3iProperty*>(input->GetProperty("ORIGIN").GetPointer());
    if (pointProp.IsNotNull() )
    {
      const itk::Point<int, 3> & p = pointProp->GetValue();
      m_TransducerPosition[0] = p[0];
      m_TransducerPosition[1] = p[1];
      m_TransducerPosition[2] = p[2];
    }
  }

  itkDebugMacro( << "compute angle corrected image .... " );
  itkDebugMacro( << "  Center[0]=" << m_Center[0] << " Center[1]=" << m_Center[1] << " Center[2]=" << m_Center[2] );
  itkDebugMacro( << "  TransducerPosition[0]=" << m_TransducerPosition[0] << " TransducerPosition[1]=" << m_TransducerPosition[1] << " TransducerPosition[2]=" << m_TransducerPosition[2] );

  const Vector3D & spacing = input->GetSlicedGeometry()->GetSpacing();
  //  MITK_INFO << "   in: xres=" << spacing[0] << " yres=" << spacing[1] << " zres=" << spacing[2] << std::endl;
  
  if((spacing[0]!=spacing[1]) || (spacing[0]!=spacing[2]))
  {
    itkExceptionMacro("filter does not work for uninsotropic data: spacing: ("<< spacing[0] << "," << spacing[1] << "," << spacing[2] << ")");
  }

  Vector3D p;
  Vector3D tx_direction;
  Vector3D tx_position = m_TransducerPosition.GetVectorFromOrigin();
  Vector3D center = m_Center.GetVectorFromOrigin();
  Vector3D assumed_direction;
  ScalarType &x=p[0];
  ScalarType &y=p[1];
  ScalarType &z=p[2];
  Vector3D down;
  FillVector3D(down,0.0,0.0,-1.0);

  int xDim = input->GetDimension(0);
  int yDim = input->GetDimension(1);
  int zDim = input->GetDimension(2);

  mitkIpPicDescriptor* pic_out;
  pic_out = mitkIpPicNew();
  pic_out->dim = 3;
  pic_out->bpe  = output->GetPixelType().GetBpe();
  //pic_out->type = output->GetPixelType().GetType();
  pic_out->n[0] = xDim;
  pic_out->n[1] = yDim;
  pic_out->n[2] = zDim;
  pic_out->data = malloc(_mitkIpPicSize(pic_out));

  //go!
  mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
  timeSelector->SetInput(input);

  int nstart, nmax;
  int tstart, tmax;

  tstart=output->GetRequestedRegion().GetIndex(3);
  nstart=output->GetRequestedRegion().GetIndex(4);

  tmax=tstart+output->GetRequestedRegion().GetSize(3);
  nmax=nstart+output->GetRequestedRegion().GetSize(4);

  int n,t;
  for(n=nstart;n<nmax;++n)//output->GetNumberOfChannels();++n)
  {
    timeSelector->SetChannelNr(n);

    for(t=tstart;t<tmax;++t)
    {
      timeSelector->SetTimeNr(t);

      timeSelector->Update();

      typedef unsigned char InputImagePixelType;
      typedef ScalarType OutputImagePixelType;

      if(input->GetPixelType().GetTypeId()!=typeid(InputImagePixelType))
      {
        itkExceptionMacro("only implemented for " << typeid(PixelType).name() );
      }

      InputImagePixelType *in;
      OutputImagePixelType *out;

      in  = (InputImagePixelType *)timeSelector->GetOutput()->GetData();
      out = (OutputImagePixelType*)pic_out->data;

      for (z=0 ; z<zDim ; ++z) 
      {
        for (y=0; y<yDim; ++y) 
        {
          for (x=0; x<xDim; ++x, ++in, ++out) 
          {
            tx_direction = tx_position-p;
            tx_direction.Normalize();

            //are we within the acquisition cone?
//            if(-tx_direction*down>vnl_math::pi_over_4)
            {
              assumed_direction = center-p;
              assumed_direction.Normalize();
              ScalarType cos_factor = tx_direction*assumed_direction;

              if(fabs(cos_factor)>eps)
                *out=((ScalarType)(*in)-128.0)/cos_factor;
              else
                *out=((ScalarType)(*in)-128.0)/eps;
            }
            //else
            //  *out=0;
          }
        }
      }
      //output->SetPicVolume(pic_out, t, n);
    }
  }
}

void mitk::AngleCorrectByPointFilter::GenerateInputRequestedRegion()
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
