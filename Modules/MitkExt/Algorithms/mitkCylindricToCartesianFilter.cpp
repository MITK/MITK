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


#include "mitkCylindricToCartesianFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"

#include <ipPic/ipPicTypeMultiplex.h>

template <class T>
void _transform(ipPicDescriptor *pic, ipPicDescriptor *dest, float _outsideValue, float *fr, float *fphi, float *fz, short *rt, unsigned int *phit, unsigned int *zt, ipPicDescriptor *coneCutOff_pic)  //...t=truncated
{
  T outsideValue = static_cast<T>(_outsideValue);
  register float f, ft, f0, f1, f2, f3;

  ipInt2_t ox_size;
  ipInt2_t nx_size, ny_size, nz_size;
  int oxy_size, nxy_size;

  T* orig, *dp, *dest_start;

  ipInt2_t* coneCutOff=(ipInt2_t*)coneCutOff_pic->data;

  orig=(T*)pic->data;
  ox_size=pic->n[0];
  oxy_size=ox_size*pic->n[1];

  nx_size=dest->n[0];
  ny_size=dest->n[1];
  nxy_size=nx_size*ny_size;
  nz_size=dest->n[2];

  /*nx_size=360;
  ny_size=360;
  nxy_size=nx_size*ny_size;
  nz_size=256;*/

  dest_start=dp=((T*)dest->data)+nxy_size*(nz_size-1);

  ipInt2_t y;
  //  int size=_ipPicElements(pic);

  register ipInt2_t x,z;
  for(y=0;y<ny_size;++y)
  {
    ipInt2_t x_start;
    register ipInt2_t x_end;
    int r0plusphi0=*rt;
    if(r0plusphi0>=0)
    {
      x_start=0; x_end=nx_size;
    }
    else
    {
      x_start=-r0plusphi0; x_end=nx_size+r0plusphi0;

      for(z=0;z<nz_size;++z,dp+=-nxy_size-x_start)
        for(x=0;x<x_start;++x,++dp)
          *dp=outsideValue;    
      dp+=nxy_size*nz_size; dp+=x_end;
      for(z=0;z<nz_size;++z,dp+=-nxy_size-x_start)
        for(x=x_end;x<nx_size;++x,++dp)
          *dp=outsideValue;        
      dp+=nxy_size*nz_size; dp-=x_end;

      fr+=x_start;
      fphi+=x_start;
      rt+=x_start;
      phit+=x_start;
      dp+=x_start;
      coneCutOff+=x_start;
    }

    for(x=x_start;x<x_end;++x, ++fr, ++fphi, ++rt, ++phit, ++dp, ++coneCutOff)
    {
      f=*fr;
      f3=*fphi;
      f0=1-f;
      f1=1-f3;
      f2=f0*f3;
      f0*=f1;
      f1*=f;
      f3*=f;

      r0plusphi0=*rt+*phit;

      ipInt2_t z_start;

      z_start=*coneCutOff;

      T *op;
      register unsigned int step;
      unsigned int *ztp;

      op=orig+*rt+*phit;

      float *fzp;

      for(z=0;z<z_start;++z, dp-=nxy_size)
        *dp=outsideValue;

      ztp=zt+z_start;
      fzp=fz+z_start;

      for(z=z_start;z<nz_size;++z, dp-=nxy_size)
      {
        step=*(ztp++);

        register T *opt=op;
        opt+=step;
        f =*opt*f0;  ++opt;
        f+=*opt*f1;  opt+=oxy_size; --opt;
        f+=*opt*f2;  ++opt;
        f+=*opt*f3;  opt-=oxy_size; --opt;

        opt+=ox_size;
        ft =*opt*f0;  ++opt;
        ft+=*opt*f1;  opt+=oxy_size; --opt;
        ft+=*opt*f2;  ++opt;
        ft+=*opt*f3;

        *dp=(T)((1-*fzp)*f+*fzp*ft+0.5);
      }

      dp+=nxy_size*nz_size;
    }
    fr+=nx_size-x_end;
    fphi+=nx_size-x_end;
    rt+=nx_size-x_end;
    phit+=nx_size-x_end;
    dp+=nx_size-x_end;
    coneCutOff+=nx_size-x_end;
  }
}

void mitk::CylindricToCartesianFilter::buildTransformShortCuts(int orig_xsize, int orig_ysize, int orig_zsize, int new_xsize, ipPicDescriptor * &rt_pic, ipPicDescriptor * &phit_pic, ipPicDescriptor * &fr_pic, ipPicDescriptor * &fphi_pic, unsigned int * &zt, float * &fz)
{
  --orig_zsize;

  rt_pic=ipPicNew();
  rt_pic->type=ipPicInt;
  rt_pic->bpe=16;
  rt_pic->dim=2;
  rt_pic->n[0]=rt_pic->n[1]=new_xsize;
  rt_pic->data=malloc(_ipPicSize(rt_pic));

  phit_pic=ipPicNew();
  phit_pic->type=ipPicUInt;
  phit_pic->bpe=32;
  phit_pic->dim=2;
  phit_pic->n[0]=phit_pic->n[1]=new_xsize;
  phit_pic->data=malloc(_ipPicSize(phit_pic));

  fr_pic=ipPicNew();
  fr_pic->type=ipPicFloat;
  fr_pic->bpe=32;
  fr_pic->dim=2;
  fr_pic->n[0]=fr_pic->n[1]=new_xsize;
  fr_pic->data=malloc(_ipPicSize(fr_pic));

  fphi_pic=ipPicNew();
  fphi_pic->type=ipPicFloat;
  fphi_pic->bpe=32;
  fphi_pic->dim=2;
  fphi_pic->n[0]=fphi_pic->n[1]=new_xsize;
  fphi_pic->data=malloc(_ipPicSize(fphi_pic));

  ipInt2_t  *rtp=(ipInt2_t*)rt_pic->data, *rt_xzero, rt, phit;
  ipUInt4_t  *phitp=(ipUInt4_t*)phit_pic->data;
  ipFloat4_t *fr=(ipFloat4_t *)fr_pic->data;
  ipFloat4_t *fphi=(ipFloat4_t *)fphi_pic->data;

  ipFloat4_t r, phi, scale=(double)orig_xsize/(double)new_xsize;

  int x,y,xy0,xy0_orig, oxy_size, new_zsize;
  oxy_size=orig_xsize*orig_ysize;
  xy0=(int)(((double)new_xsize)/2+0.5); 
  xy0_orig=(int)(((double)orig_xsize)/2+0.5); 

  new_zsize=(int)(orig_ysize/scale);
  // \bug y compared to x 
  for(y=0;y<new_xsize;++y)
  {
    rt_xzero=rtp; *rtp=0;
    for(x=0;x<new_xsize;++x,++fr,++fphi,++rtp, ++phitp)
    {
      int xq=x-xy0, yq=y-xy0;

      r=sqrt( (double) (xq*xq+yq*yq));

      //      float rtest=-(xy0-sqrt(xy0*xy0-yq*yq))-0.5;
      rt=(ipInt2_t)(-(xy0-sqrt((double) (xy0*xy0-yq*yq)))-0.5);/*in rt steht der Index des Endes der zu �berspringenden Punkte=>anfangen bei -rt+1!*/
      //      if((x>=-rt) && (x<new_xsize+rt))
      {
        if(y!=xy0)
          r=r*(y>xy0?1.0:-1.0)*scale+xy0_orig;
        else
          r=r*(x>xy0?-1.0:1.0)*scale+xy0_orig;
        rt=(ipInt2_t)r;
        int xtmp=x;
        if(x>xy0)
          xtmp=new_xsize-x;
        if(rt<0)
        {
          r=rt=0;
          if(xtmp>-*rt_xzero)
            *rt_xzero=-xtmp;
          *fr=0;
        }
        else
          if(rt>orig_xsize-1)
          {
            r=rt=orig_xsize-1;
            if(xtmp>-*rt_xzero)
              *rt_xzero=-xtmp;
            *fr=0;
          }
          else
            *fr=r-rt;
        if(*fr<0)
          *fr=0;
      }
      //      else
      //        *fr=0;

      phi=orig_zsize-(yq==0?1:-atan((float)xq/yq)/M_PI+0.5)*orig_zsize;
      phit=(ipUInt4_t)phi;
      *fphi=phi-phit;

      *rtp=rt;
      *phitp=phit*oxy_size;
    }
  }

  zt=(unsigned int *)malloc(sizeof(unsigned int)*new_zsize);
  fz=(float *)malloc(sizeof(float)*new_zsize);

  float *fzp=fz;
  unsigned int *ztp=zt;

  int z;
  float z_step=orig_ysize/(orig_ysize*((float)new_xsize)/orig_xsize);
  for(z=0;z<new_zsize;++z,++fzp,++ztp)
  {    
    *fzp=z*z_step;
    *ztp=(unsigned int)*fzp;
    *fzp-=*ztp;
    *ztp*=orig_xsize;
  }
}

void mitk::CylindricToCartesianFilter::buildConeCutOffShortCut(int orig_xsize, int orig_ysize, ipPicDescriptor *rt_pic, ipPicDescriptor *fr_pic, float a, float b, ipPicDescriptor * &coneCutOff_pic)
{
  coneCutOff_pic=ipPicNew();
  coneCutOff_pic->type=ipPicInt;
  coneCutOff_pic->bpe=16;
  coneCutOff_pic->dim=2;
  coneCutOff_pic->n[0]=coneCutOff_pic->n[1]=rt_pic->n[0];
  coneCutOff_pic->data=malloc(_ipPicSize(coneCutOff_pic));

  int i, size=_ipPicElements(rt_pic);
  ipInt2_t *rt, *ccop, ohx_size, nz_size;
  ipFloat4_t *fr;

  a*=(float)rt_pic->n[0]/orig_xsize;
  b*=(float)rt_pic->n[0]/orig_xsize;

  ohx_size=orig_xsize/2;
  nz_size=orig_ysize*rt_pic->n[0]/orig_xsize;

  rt=(ipInt2_t *)rt_pic->data; fr=(ipFloat4_t*)fr_pic->data; ccop=(ipInt2_t *)coneCutOff_pic->data;

  for(i=0; i<size; ++i, ++rt, ++ccop)
  {
    register ipInt2_t cco;
    if(*rt<=ohx_size)
      cco=(ipInt2_t)(a*(*rt+*fr)+b);
    else
      cco=(ipInt2_t)(a*(orig_xsize-(*rt+*fr))+b);
    if(cco<0)
      cco=0;
    if(cco>=nz_size) 
      cco=nz_size;
    *ccop=cco;
  }
}

void mitk::CylindricToCartesianFilter::GenerateOutputInformation()
{
  mitk::Image::Pointer output = this->GetOutput();
  if ((output->IsInitialized()) && (output->GetPipelineMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  mitk::Image::ConstPointer input = this->GetInput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  unsigned int i, *tmpDimensions=new unsigned int[std::max(3u,input->GetDimension())];

  tmpDimensions[0]=m_TargetXSize;
  if(tmpDimensions[0]==0)
    tmpDimensions[0] = input->GetDimension(0);

  float scale=((float)tmpDimensions[0])/input->GetDimension(0);

  tmpDimensions[1] = tmpDimensions[0];
  tmpDimensions[2] = (unsigned int)(scale*input->GetDimension(1));

  for(i=3;i<input->GetDimension();++i)
    tmpDimensions[i]=input->GetDimension(i);

  output->Initialize(input->GetPixelType(), 
    input->GetDimension(),
    tmpDimensions,
    input->GetNumberOfChannels());

  // initialize the spacing of the output
  Vector3D spacing = input->GetSlicedGeometry()->GetSpacing();
  if(input->GetDimension()>=2)
    spacing[2]=spacing[1];
  else
    spacing[2] = 1.0;
  spacing[1] = spacing[0];
  spacing *= 1.0/scale;
  output->GetSlicedGeometry()->SetSpacing(spacing);

  mitk::Point3iProperty::Pointer pointProp;
  pointProp = dynamic_cast<mitk::Point3iProperty*>(input->GetProperty("ORIGIN").GetPointer());
  if (pointProp.IsNotNull() )
  {
    itk::Point<int, 3> tp = pointProp->GetValue();
    tp[2] = (int)(tmpDimensions[2]-tp[1] * scale-1);
    tp[0] = tmpDimensions[0]/2;
    tp[1] = tmpDimensions[0]/2;
    mitk::Point3iProperty::Pointer pointProp = mitk::Point3iProperty::New(tp);
    output->SetProperty("ORIGIN", pointProp);
  }
  delete [] tmpDimensions;

  //output->GetSlicedGeometry()->SetGeometry2D(mitk::Image::BuildStandardPlaneGeometry2D(output->GetSlicedGeometry(), tmpDimensions).GetPointer(), 0);
  //set the timebounds - after SetGeometry2D, so that the already created PlaneGeometry will also receive this timebounds.
  //@fixme!!! will not work for not evenly timed data!
  output->GetSlicedGeometry()->SetTimeBounds(input->GetSlicedGeometry()->GetTimeBounds());
  output->GetTimeSlicedGeometry()->InitializeEvenlyTimed(output->GetSlicedGeometry(), output->GetTimeSlicedGeometry()->GetTimeSteps());

  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::CylindricToCartesianFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
  timeSelector->SetInput(input);

  ipPicDescriptor* pic_transformed=NULL;
  pic_transformed = ipPicNew();
  pic_transformed->dim=3;
  pic_transformed->bpe  = output->GetPixelType().GetBpe();
  pic_transformed->type = output->GetPixelType().GetType();
  pic_transformed->n[0] = output->GetDimension(0);
  pic_transformed->n[1] = output->GetDimension(1);
  pic_transformed->n[2] = output->GetDimension(2);
  pic_transformed->data=malloc(_ipPicSize(pic_transformed));

  int nstart, nmax;
  int tstart, tmax;

  tstart=output->GetRequestedRegion().GetIndex(3);
  nstart=output->GetRequestedRegion().GetIndex(4);

  tmax=tstart+output->GetRequestedRegion().GetSize(3);
  nmax=nstart+output->GetRequestedRegion().GetSize(4);

  if(zt==NULL)
  {
    timeSelector->SetChannelNr(nstart);
    timeSelector->SetTimeNr(tstart);

    buildTransformShortCuts(input->GetDimension(0),input->GetDimension(1), input->GetDimension(2), output->GetDimension(0), rt_pic, phit_pic, fr_pic, fphi_pic, zt, fz);

    // query the line limiting the sector
    a=b=0;
    mitk::FloatProperty::Pointer prop;

    prop = dynamic_cast<mitk::FloatProperty*>(input->GetProperty("SECTOR LIMITING LINE SLOPE").GetPointer());
    if (prop.IsNotNull() )
      a = prop->GetValue();
    prop = dynamic_cast<mitk::FloatProperty*>(input->GetProperty("SECTOR LIMITING LINE OFFSET").GetPointer());
    if (prop.IsNotNull() )
      b = prop->GetValue();

    buildConeCutOffShortCut(input->GetDimension(0),input->GetDimension(1), rt_pic, fr_pic, a, b, coneCutOff_pic);
    //    ipPicPut("C:\\temp\\rt_90.pic",rt_pic);
    //ipPicPut("C:\\temp\\coneCutOff.pic", coneCutOff_pic);
  }

  int n,t;
  for(n=nstart;n<nmax;++n)//output->GetNumberOfChannels();++n)
  {
    timeSelector->SetChannelNr(n);

    for(t=tstart;t<tmax;++t)
    {
      timeSelector->SetTimeNr(t);

      timeSelector->Update();

      _ipPicFreeTags(pic_transformed->info->tags_head);
      pic_transformed->info->tags_head = _ipPicCloneTags(timeSelector->GetOutput()->GetPic()->info->tags_head);
      if(input->GetDimension(2)>1)
      {

        mitkIpPicTypeMultiplex9(_transform, timeSelector->GetOutput()->GetPic(), pic_transformed, m_OutsideValue, (float*)fr_pic->data, (float*)fphi_pic->data, fz, (short *)rt_pic->data, (unsigned int *)phit_pic->data, zt, coneCutOff_pic);
        //  ipPicPut("1trf.pic",pic_transformed);  
      }
      else
      {
        ipPicDescriptor *doubleSlice=ipPicCopyHeader(timeSelector->GetOutput()->GetPic(), NULL);
        doubleSlice->dim=3;
        doubleSlice->n[2]=2;
        doubleSlice->data=malloc(_ipPicSize(doubleSlice));
        memcpy(doubleSlice->data, timeSelector->GetOutput()->GetPic()->data, _ipPicSize(doubleSlice)/2);
        mitkIpPicTypeMultiplex9(_transform, doubleSlice, pic_transformed, m_OutsideValue, (float*)fr_pic->data, (float*)fphi_pic->data, fz, (short *)rt_pic->data, (unsigned int *)phit_pic->data, zt, coneCutOff_pic);
        ipPicFree(doubleSlice);
      }
      output->SetPicVolume(pic_transformed, t, n);
    }
  }
  //ipPicPut("outzzzzzzzz.pic",pic_transformed);  
  ipPicFree(pic_transformed);
  m_TimeOfHeaderInitialization.Modified();
}

mitk::CylindricToCartesianFilter::CylindricToCartesianFilter()
: m_OutsideValue(0.0), m_TargetXSize(0)
{
  rt_pic = NULL; phit_pic = NULL; fr_pic = NULL; fphi_pic = NULL; coneCutOff_pic = NULL;
  zt = NULL; fz = NULL;
  a=b=0.0;
}

mitk::CylindricToCartesianFilter::~CylindricToCartesianFilter()
{
  if(rt_pic!=NULL)  ipPicFree(rt_pic);
  if(phit_pic!=NULL)  ipPicFree(phit_pic);
  if(fr_pic!=NULL)  ipPicFree(fr_pic);
  if(fphi_pic!=NULL)  ipPicFree(fphi_pic);
  if(coneCutOff_pic!=NULL)  ipPicFree(coneCutOff_pic);
  if(zt != NULL)  free(zt);
  if(fz != NULL)  free (fz);
}

void mitk::CylindricToCartesianFilter::GenerateInputRequestedRegion()
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
  requestedRegion.SetSize(0, input->GetDimension(0));
  requestedRegion.SetSize(1, input->GetDimension(1));
  requestedRegion.SetSize(2, input->GetDimension(2));

  input->SetRequestedRegion( & requestedRegion );
}
