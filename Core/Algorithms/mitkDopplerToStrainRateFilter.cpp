#include "mitkDopplerToStrainRateFilter.h"
#include "mitkImageTimeSelector.h"

#include <iostream.h>
#include <string>


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


		
	output->Initialize(input->GetPixelType(),
										input->GetDimension(),
										tmpDimensions,
										input->GetNumberOfChannels());

//	// initialize the spacing of the output
//    const float *spacinglist = input->GetGeometry()->GetSpacing();
//    Vector3D spacing(spacinglist[0],spacinglist[0],1.0);
//    if(input->GetDimension()>=2)
//        spacing.z=spacinglist[1];
//	spacing *= 1.0/scale;
//
//	
    output->GetSlicedGeometry()->SetSpacing(input->GetSlicedGeometry()->GetSpacing());

	// initialize the spacing of the output
	// @todo position of input image is not yet used to calculate position of the output
    mitk::Point3D origin, right, bottom;
    origin.set(0,0,0);                output->GetGeometry()->UnitsToMM(origin, origin);
    right.set(tmpDimensions[0],0,0);  output->GetGeometry()->UnitsToMM(right, right);
    bottom.set(0,tmpDimensions[1],0); output->GetGeometry()->UnitsToMM(bottom, bottom);

    PlaneView view_std(origin, right, bottom);

    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    planegeometry->SetPlaneView(view_std);
    planegeometry->SetSizeInUnits(tmpDimensions[0], tmpDimensions[1]);

    output->GetSlicedGeometry()->SetGeometry2D(planegeometry.GetPointer(), 0, 0);
    output->GetSlicedGeometry()->SetEvenlySpaced();

	// @todo convert transducer position into new coordinate system
/*	
	Tag = ipPicQueryTag(picHeader, "ORIGIN");
	if (Tag != NULL)
	{
		int z = ((int *) Tag->value)[1];
		((int *) Tag->value)[0] = picHeader->n[0]/2;
		((int *) Tag->value)[1] = picHeader->n[0]/2;
		((int *) Tag->value)[2] = z * scale;
	}
*/

	delete [] tmpDimensions;

    m_TimeOfHeaderInitialization.Modified();
}

void mitk::DopplerToStrainRateFilter::GenerateData()
{
	mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

	mitk::ImageTimeSelector::Pointer timeSelector=mitk::ImageTimeSelector::New();
	timeSelector->SetInput(input);

	ipPicDescriptor* picStrainRate;
	picStrainRate = ipPicNew();
	picStrainRate->dim=3;
	picStrainRate->bpe  = output->GetPixelType().GetBpe();
	picStrainRate->type = output->GetPixelType().GetType();
	picStrainRate->n[0] = output->GetDimension(0);
	picStrainRate->n[1] = output->GetDimension(1);
	picStrainRate->n[2] = output->GetDimension(2);
	picStrainRate->data=malloc(_ipPicSize(picStrainRate));


	int xDim = picStrainRate->n[0];
	int yDim = picStrainRate->n[1];
	int zDim = picStrainRate->n[2];
	long slice_size = xDim*yDim;
	long vol_size = slice_size*zDim;
	
	
//  ipPicDescriptor *picDoppler = timeSelector->GetOutput()->GetPic();
   ipPicDescriptor *picDoppler;
  
 	int x,y,z,time;							// loop-counter
	int strainRate;						// the computed Strain Rate
	int v1,v2;								// velocity and Point p1 and p2

	float alpha;								// the beam-angle, angle betwen current point and beam-point
	float dx, dy;								// projection of this->distance to x- and y-axis
	float weightX, weightY;			// weigth-factors for x-velocity and y-velocity
	int dxFloor, dyFloor;				// floored dx, dy
	
	int x1, x2, x3, x4;					// a square, where the velocity v1 lies in
	int y1, y2, y3, y4;					// the points are used for interpolation
	int vTmp1, vTmp2;						// velocities used for interpolation
	int vTmp3, vTmp4;	


	int minStrainRate=128, maxStrainRate=128;
  
	int n, nmax;
	int t, tmax;

	t = output->GetRequestedRegion().GetIndex(3);
	n = output->GetRequestedRegion().GetIndex(4);

	tmax = t + output->GetRequestedRegion().GetSize(3);
	nmax = n + output->GetRequestedRegion().GetSize(4);
  
	for(;n<nmax;++n)//output->GetNumberOfChannels();++n)
	{
		timeSelector->SetChannelNr(n);

		for(t=0;t<tmax;++t)
		{
 
			timeSelector->SetTimeNr(t);
			timeSelector->Update();

			_ipPicFreeTags(picStrainRate->info->tags_head);
			picStrainRate->info->tags_head = _ipPicCloneTags(timeSelector->GetOutput()->GetPic()->info->tags_head);
      
#ifdef WRITE_ANGLE_PIC
	ipPicDescriptor *anglePic;
	ipBool_t isAnglePicWritten = ipFalse;
	
	anglePic = ipPicNew();
	anglePic->type = ipPicInt;
  anglePic->bpe = 8;
	anglePic->dim = 2;
	anglePic->n[0] = xDim;
	anglePic->n[1] = yDim;
	anglePic->data = (ipInt1_t *)calloc(xDim*yDim,sizeof(ipInt1_t));
#endif

		picDoppler = timeSelector->GetOutput()->GetPic();
      

		for (z=0 ; z<zDim ; z++) {
		
		
				for (y=1; y<yDim; y++) {
				
					for (x=0; x<xDim; x++) {

						if ((m_Origin.x - x)==0) {
							
							int yTmp = ( (y-m_Distance)<0 )  ? 0 : (y-m_Distance);
							v1 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + yTmp*xDim + x] ;
							v2 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + y*xDim + x] ;
						
						} else {
						
							alpha = atan( (float) (m_Origin.x - x)  / (float) y );
							
							dx = sin(alpha) * m_Distance;
							dy = cos(alpha) * m_Distance;
						
							weightX = dx - floor(dx);
							weightY = dy - floor(dy);
						
							dxFloor = (int) floor(dx);
							dyFloor = (int) floor(dy);
						
							x1 = x + dxFloor;					// lower left
							y1 = y - dyFloor;
						
							x2 = x + (dxFloor+1);			// lower right
							y2 = y - dyFloor;
		
							x3 = x + (dxFloor+1);			// upper right
							y3 = y - (dyFloor+1);
		
							x4 = x + dxFloor;					// upper left
							y4 = y - (dyFloor+1);
						
							vTmp1 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + y1*xDim + x1];
							vTmp2 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + y2*xDim + x2];
							vTmp3 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + y3*xDim + x3];
							vTmp4 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + y4*xDim + x4];
						
							v1 = (int) ((1-weightX)*(1-weightY)*vTmp1 + weightX*(1-weightY)*vTmp2 + weightX*weightY*vTmp3 + (1-weightX)*weightY*vTmp4);
							v2 = ((ipUInt1_t *)picDoppler->data)[z*slice_size + y*xDim + x] ;
						}
					
						if ( 	(v1==0 ) ||
							(v2==0)  ||
							(v1>(128-m_NoStrainInterval) && v1<(128+m_NoStrainInterval)) ||
							(v2>(128-m_NoStrainInterval) && v2<(128+m_NoStrainInterval))) {
							
							strainRate = 0;

						} else {
						  
						  // !!!! we must divide by this->getDistance()
		
							strainRate = ( (v1 - v2)/2 +128 );  

						}
						
						if (strainRate < minStrainRate && strainRate > 0 )
						  minStrainRate = strainRate;

						if (strainRate > maxStrainRate)
						  maxStrainRate = strainRate;


						if (strainRate<0 ) {
							//strainRate = -strainRate;
							cout << " error: neg. strainRate ... exit() " << endl;
							exit(0);
							}
						
					
						((ipUInt1_t *)picStrainRate->data)[t*vol_size + z*slice_size + y*xDim + x]=strainRate;
						// cout << "z: " << z << " y: " << y << " x: " << x << " strainrate: " << strainRate << endl;
					
#ifdef WRITE_ANGLE_PIC
						if (!isAnglePicWritten)					
							((ipInt1_t *)anglePic->data)[y*xDim + x] = (int) ( (alpha/1.6)*128);
#endif
						
					}  // x
				
				} // y
				
				//isAnglePicWritten = ipTrue;
			
			} // z
			
	//isStrainComputed = ipTrue;

	std::string filename;
	filename ="strain.pic";
//	sprintf(filename,"strain.pic");
	ipPicPut(const_cast<char *>(filename.c_str()),picStrainRate);	
	
#ifdef WRITE_ANGLE_PIC
	sprintf(filename,"angle.pic");
	ipPicPut(filename,anglePic);	
#endif



			output->SetPicVolume(picStrainRate, t, n);
		}
	}
	//ipPicPut("outzzzzzzzz.pic",pic_transformed);	
//	ipPicFree(picDoppler);


  std::cout << "Strain Rate Image computed.... " << std::endl;
  std::cout << "minStrainRate: " << minStrainRate << std::endl;
	std::cout << "maxStrainRate: " << maxStrainRate << std::endl;
}

mitk::DopplerToStrainRateFilter::DopplerToStrainRateFilter()
    : m_Distance(10), m_NoStrainInterval(2)
{

    m_Origin.x = 0;
    m_Origin.y = 0;
    m_Origin.z = 0;
    
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
