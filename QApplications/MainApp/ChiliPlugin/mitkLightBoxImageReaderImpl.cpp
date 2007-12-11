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

#include "mitkLightBoxImageReaderImpl.h"
#include "QmitkLightBoxReaderDialog.h"

#include <mitkProperties.h>
#include "mitkPlaneGeometry.h"
#include "mitkFrameOfReferenceUIDManager.h"

#include <itkImageFileReader.h>

#include <chili/qclightbox.h>
#include <chili/isg.h>
#include <ipDicom/ipDicom.h>
#include <ipPic/ipPicTags.h>
#include <ipPic/ipPic.h>

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

// helper class for property import from pic/dicom-headers (see mitk::LightBoxImageReaderImpl::GetPropertyList())
class HeaderTagInfo
{
  public:

    typedef enum { MissingInDicom, String, Int, UnsignedInt } DataType;

    HeaderTagInfo( const char* ppicTagKey, ipUInt2_t pdicomGroup, ipUInt2_t pdicomElement, DataType ptype )
    : picTagKey( ppicTagKey ), dicomGroup( pdicomGroup ), dicomElement( pdicomElement ), type( ptype )
    {
    }

  std::string picTagKey;
  ipUInt2_t dicomGroup;
  ipUInt2_t dicomElement;
  DataType type;
};

//helper class for sort a vectorlist
class compare_Vector
{
  public:
  bool operator() (const mitk::Vector3D &first, const mitk::Vector3D &second)
  {
    if( first[2] < second[2] )
      return true;
    return false;
  }
};

mitk::LightBoxImageReaderImpl::LightBoxImageReaderImpl()
: m_LightBox(NULL)
{
}

mitk::LightBoxImageReaderImpl::~LightBoxImageReaderImpl()
{
}

void mitk::LightBoxImageReaderImpl::SetLightBox( QcLightbox* lightbox )
{
  if( lightbox != m_LightBox )
  {
    m_LightBox = lightbox;
    Modified();
  }
}

QcLightbox* mitk::LightBoxImageReaderImpl::GetLightBox() const
{
  return m_LightBox;
}

void mitk::LightBoxImageReaderImpl::GenerateOutputInformation()
{
  if( m_LightBox == NULL )
  {
    itkWarningMacro( <<"Lightbox not set, using current lightbox" );
    m_LightBox = mitk::PACSPlugin::GetInstance()->GetCurrentLightbox();
  }
  if ( m_LightBox == NULL )
  {
    itkWarningMacro( << "No lightbox found." );
    return;
  }
  if( m_LightBox->getFrames() == 0 )
  {
    itkWarningMacro( << "Lightbox empty." );
    return;
  }

  int RealPosition;
  SliceInfoArray sliceInfos;

  itkDebugMacro( <<"GenerateOutputInformation" );

  Image::Pointer output = this->GetOutput();

  //did we already do it since the last change in the lightbox?
  if ( (output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()) )
    return;

  itkDebugMacro( <<"Reading lightbox for GenerateOutputInformation()" );

  int numberOfImages = 0, numberOfTimePoints = 0; // numberOfSlicesInCurrentTimePoint = 0;
  unsigned int position;
  //ipPicDescriptor*  pic2 = NULL;
  interSliceGeometry_t *isg;
  Point3D originCurrentSlice;
  Point3D originLastSlice;
  Point3D originFirstSlice;
  sliceInfo sliceInfo;

  //virtually sort the lightbox
  m_ImageNumbers.clear();
  m_ImageNumbers.reserve( m_LightBox->getFrames() );
  SortImage( m_ImageNumbers );

  //read information of first slice
  RealPosition = GetRealPosition( 0, m_ImageNumbers );
  if( m_LightBox->fetchHeader( RealPosition ) != NULL )
  {
    isg = m_LightBox->fetchDicomGeometry( RealPosition );
    if( isg != NULL )
    {
      vtk2itk( isg->o, originFirstSlice );
      originLastSlice = originFirstSlice;
    }
  }

  //count number of time points (numberOfTimePoints):
  // iterate through the virutally sorted lightbox:
  //  position= sorted position
  //  realPosition= original position in the (real) lightbox
  for( position = 0; position < m_LightBox->getFrames(); ++position )
  {
    //get original position of the image slice in the (real) lightbox
    RealPosition = GetRealPosition( position,m_ImageNumbers );

    //is it an image?
    if( m_LightBox->fetchHeader( RealPosition ) != NULL )
    {
      //get the isg
      isg = m_LightBox->fetchDicomGeometry( RealPosition );
      if( isg != NULL )
      {
        itkDebugMacro( <<numberOfImages );
        vtk2itk( isg->o,originCurrentSlice );
        //timeconv1=ConvertTime(pic2);

        //check if we are still on the same slice
        if( originCurrentSlice == originLastSlice )
        {
          /*if (numberOfTimePoints==1 && number==1)
          {
          tsv=ipPicQueryTag(pic2,"SOURCE HEADER");
          dicomFindElement((unsigned char*) tsv->value, 0x0008, 0x0033, &data, &len);
          sscanf( (char *) data, "%f", &imagetime );
          }*/
          //yes? so this is another time frame
          ++numberOfTimePoints;
        }
        else
        {
          sliceInfo.numberOfTimePoints = numberOfTimePoints;
          sliceInfo.startPosition = originLastSlice;
          sliceInfos.push_back( sliceInfo );
          numberOfTimePoints = 1;
        }
      }

      originLastSlice = originCurrentSlice;
      //timeconv2=timeconv1;
      ++numberOfImages;
    }

  }

  if( numberOfImages == 0 )
  {
    itk::ImageFileReaderException e(__FILE__, __LINE__);
    itk::OStringStream msg;
    msg << "lightbox is empty";
    e.SetDescription( msg.str().c_str() );
    throw e;
    return;
  }

  //add information for last slice
  sliceInfo.numberOfTimePoints = numberOfTimePoints;
  sliceInfo.startPosition = originCurrentSlice;
  sliceInfos.push_back( sliceInfo );

  //check whether all slices have the same number of time points
  bool differentNumberOfTimePoints = false;
  SliceInfoArray::iterator slit, slend = sliceInfos.end();
  unsigned int sliceNo;
  for( slit = sliceInfos.begin(), sliceNo = 0; slit != slend; ++slit, ++sliceNo )
  {
    if( slit->numberOfTimePoints != numberOfTimePoints )
    {
      if( slit->numberOfTimePoints < numberOfTimePoints )
        numberOfTimePoints = slit->numberOfTimePoints;
      itkWarningMacro(<<"problem: different number of time points in slices (slice: " << sliceNo << " has " << slit->numberOfTimePoints << ") - will take minimum (currently: " << numberOfTimePoints);
      differentNumberOfTimePoints = true;
    }
  }
  int numberOfSlices = sliceInfos.size();
  if( differentNumberOfTimePoints )
  {
    itkWarningMacro( <<"minimum of numberOfTimePoints is " << numberOfTimePoints );
  }
  else
  {
    itkDebugMacro( <<"numberOfTimePoints" << numberOfTimePoints );
    if( numberOfSlices != numberOfImages/numberOfTimePoints )
    {
      itkWarningMacro( <<"numberOfSlices!=numberOfImages/numberOfTimePoints: " << numberOfSlices << " != " << numberOfImages/numberOfTimePoints );
    }
  }

  //build pic-header to initialize output information
  itkDebugMacro( <<"copy header" );
  RealPosition = GetRealPosition( 0, m_ImageNumbers );
  ipPicDescriptor *originalHeader = m_LightBox->fetchHeader( RealPosition );
  ipPicDescriptor *header = ipPicCopyHeader( originalHeader, NULL );

  //@FIXME: was ist, wenn die Bilder nicht alle gleich gross sind?
  if( numberOfImages>1 )
  {
    itkDebugMacro( <<"numberofimages > 1 :" << numberOfImages );
    if( numberOfTimePoints > 1 )
    {
      header->dim = 4;
      header->n[2] = numberOfSlices;
      header->n[3] = numberOfTimePoints;
    }
    else
    {
      header->dim = 3;
      header->n[2] = numberOfSlices;
      itkDebugMacro( <<"dim=3:" );
    }
  }
  itkDebugMacro( <<"initialize output" );

  //build geometry
  interSliceGeometry_t *interSliceGeometry;
  interSliceGeometry = m_LightBox->fetchDicomGeometry( RealPosition );

  if( interSliceGeometry != NULL )
  {
    Vector3D rightVector;
    Vector3D downVector;
    Vector3D spacing;

    vtk2itk( interSliceGeometry->u, rightVector );
    vtk2itk( interSliceGeometry->v, downVector );

    //its possible that a 2D-picture have no right- or down-vector, but you can not initialize [0,0,0]
    if( rightVector[0] == 0 && rightVector[1] == 0 && rightVector[2] == 0 )
      rightVector[0] = 1;
    if( downVector[0] == 0 && downVector[1] == 0 && downVector[2] == 0 )
      downVector[2] = -1;

    vtk2itk( interSliceGeometry->ps, spacing );
    itkDebugMacro( <<"spacing: "<<spacing );

    PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();

    abortLoadingTheLightbox = false;
    spacing = GetSpacingFromLB( m_ImageNumbers );
    if( abortLoadingTheLightbox ) return;

    output->Initialize( header );

    itkDebugMacro( <<"get spacing: "<<spacing );
    planegeometry->InitializeStandardPlane( output->GetDimension(0), output->GetDimension(1), rightVector, downVector, &spacing );
    //planegeometry->InitializeStandardPlane( rightVector,downVector,&GetSpacingFromLB());
    Point3D origin;
    vtk2itk( interSliceGeometry->o, origin );
    itkDebugMacro( <<"origin: "<<origin );
    planegeometry->SetOrigin( origin );
    planegeometry->SetFrameOfReferenceID( FrameOfReferenceUIDManager::AddFrameOfReferenceUID(interSliceGeometry->forUID) );

    ipPicTSV_t *tsv = ipPicQueryTag( originalHeader, "SOURCE HEADER" );

    void* data; 
    ipUInt4_t len;
    //double wert;
    //int s = 0, t = 0; 
    ScalarType timearray[2];
    int repetitionTime = 0;

    if( tsv )
    {
      //0x0018, 0x0080 : RepetitionTime
      char * tmp;
      if( dicomFindElement( (unsigned char*) tsv->value, 0x0018, 0x0080, &data, &len ) )
      {
        tmp = new char[len+1];
        strncpy( tmp, (char*)data, len );
        tmp[len]=0;
        repetitionTime = (int) atof( (char*) tmp );
      }

      //0x0018, 0x1060 : Trigger Time
      if( dicomFindElement((unsigned char*) tsv->value, 0x0018, 0x1060, &data, &len) )
      {
        tmp = new char[len+1];
        strncpy(tmp, (char*)data, len);
        tmp[len]=0;
        timearray[0] = atof( (char*) tmp );
        timearray[1] = timearray[0]+repetitionTime;
        delete [] tmp;
      }
    }

    TimeBounds timebounds( timearray );

    //workaround FIXME
    if( numberOfTimePoints>1 )
    {
      ScalarType timeBounds[] = {0.0, 1.0};
      planegeometry->SetTimeBounds( timeBounds );
    }
//    planegeometry->SetTimeBounds(timebounds);

    SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
    itkDebugMacro(<<"output->GetDimension(2): "<<output->GetDimension(2));
    slicedGeometry->InitializeEvenlySpaced(planegeometry, output->GetDimension(2));
    TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();
    itkDebugMacro( <<"output->GetDimension(3): "<<output->GetDimension(3) );
    timeSliceGeometry->InitializeEvenlyTimed( slicedGeometry, output->GetDimension(3) );
    //Variante B: timeSliceGeometry->SetEvenlyTimed(false);
    //unten: output->GetTimeGeometry()->GetGeometry3D(time)->SetTimeBounds(timeboundsOfTimeStepTime);

    timeSliceGeometry->TransferItkToVtkTransform();

    output->SetGeometry( timeSliceGeometry );

    itkDebugMacro( "origin of top slice: "<<0<<" lb pos: "<<position<< "origin:"<<output->GetGeometry()->GetCornerPoint(0) );
  }
  else
  {
    output->Initialize( header );
    itkWarningMacro( <<"interSliceGeometry is NULL" );
    itkDebugMacro( <<"spacing from pic: "<<output->GetSlicedGeometry()->GetSpacing() );
  }

  itkDebugMacro( <<" modified " );
  m_ReadHeaderTime.Modified();
}

void mitk::LightBoxImageReaderImpl::GenerateData()
{
  if( abortLoadingTheLightbox ) return;  //userabort
  if( m_LightBox == NULL )
  {
    itkWarningMacro( <<"Lightbox not set, using current lightbox" );
    m_LightBox = mitk::PACSPlugin::GetInstance()->GetCurrentLightbox();
  }
  if ( m_LightBox == NULL )
  {
    itkWarningMacro( << "No lightbox found." );
    return;
  }
  if( m_LightBox->getFrames() == 0 )
  {
    itkWarningMacro( << "Lightbox empty." );
    return;
  }

  itkDebugMacro(<<"GenerateData ");

  itkDebugMacro(<<"request output ");

  Image::Pointer output = this->GetOutput();

  int numberOfImages = 0, time = 0, time1 = 0, time2 = 0;
  unsigned int position;
  ipPicDescriptor*  pic0 = NULL;
  ipPicDescriptor*  pic = NULL;
  interSliceGeometry_t* isg0;
  interSliceGeometry_t* isg;
  Point3D origin1;
  Point3D origin0;
  Point3D origin;

  //sort image
  int RealPosition;

  int zDim = ( output->GetDimension()>2?output->GetDimensions()[2]:1 );
  itkDebugMacro( <<" zdim is "<<zDim );
  RealPosition = GetRealPosition( 0, m_ImageNumbers );
  pic0 = m_LightBox->fetchPic( RealPosition );// pFetchImage (m_LightBox, position);
  isg0 = m_LightBox->fetchDicomGeometry( RealPosition );
  if( isg0!=NULL )
  {
    vtk2itk( isg0->o,origin0 );
    origin=origin0;
    itkDebugMacro( <<"origin    "<<origin );
  }
  else
  {
    itkWarningMacro( <<"interSliceGeometry is NULL" );
  }

  output->SetPicSlice( pic0, numberOfImages,time );
  for( position = 1; position < m_LightBox->getFrames(); ++position ) 
  {
    //GetRealPosition of image
    RealPosition = GetRealPosition( position, m_ImageNumbers );

    if( m_LightBox->fetchHeader(RealPosition) != NULL )
    {
      if( numberOfImages>zDim )
      {
        itk::ImageFileReaderException e(__FILE__, __LINE__);
        itk::OStringStream msg;
        msg << "lightbox contains more images than calculated in the last GenerateOutputInformation call (>"<<zDim<<")";
        e.SetDescription( msg.str().c_str() );
        itkDebugMacro( <<"zu viele images" );
        throw e;
        return;
      }

      pic = m_LightBox->fetchPic( RealPosition );
      isg = m_LightBox->fetchDicomGeometry( RealPosition );
      if( isg != NULL )
      {
        vtk2itk( isg->o,origin1 );
        if( origin1 != origin0 && origin1!=origin )
        {
          //itkDebugMacro("origin1: "<<origin1<<" origin0: "<<origin0);
          ++numberOfImages;
          time = time1;
          origin0 = origin1;
        }
        else
        {
          ++time;
          if( origin1 == origin && origin0 != origin )
          {
            ++time2;
            time1 = time2;
            time = time2;
            numberOfImages = 0;
          }

        }
      }
      else
      {
        ++numberOfImages;
      }

      if( !pic )
      {
        itkWarningMacro( <<"Image number " << numberOfImages << "has pic-header but no pic-data." );
        continue;
      }

      output->SetPicSlice( pic, numberOfImages, time );
      itkDebugMacro( "setting slice: "<<numberOfImages<<" lb pos: "<<RealPosition<< "origin:"<<origin1 );
      //itkDebugMacro(<<"add slice  "<< numberOfImages <<" x:" <<pic->n[0]<<"y:"<<pic->n[1]);
      //output->SetPicSlice(pic, zDim-1-numberOfImages,time);
      //itkDebugMacro(<<" add slice   successful "<< numberOfImages<<"  "<< pic->n[0]<<"  "<<pic->n[1]);
      //++numberOfImages;
    }
  }
  itkDebugMacro( <<"fertig " );
}

mitk::Vector3D mitk::LightBoxImageReaderImpl::GetSpacingFromLB( LocalImageInfoArray& imageNumbers )
{
  Vector3D spacing;
  spacing.Fill(1.0);
  interSliceGeometry_t* isg_t  = m_LightBox->fetchDicomGeometry(0);
  if( isg_t != NULL )
  {
    vtk2itk( isg_t->ps, spacing );
  }

  //if there is only one image, there is no spacing
  if( imageNumbers.size() == 1 ) return spacing;

  LocalImageInfoArray::iterator it = imageNumbers.begin(), infoEnd = imageNumbers.end();
  Vector3D& origin0 = it->origin;
  Vector3D toNext;
  std::list<Vector3D> spacingList;

  //count all spacings
  while( it != infoEnd)
  {
    if( Equal( it->origin, origin0 ) == false)
    {
      toNext = it->origin-origin0;
      spacing[2] = toNext.GetNorm();
/*
      // round after 2. decimal digit
      spacing[2] *= 100.0;
      spacing[2] = ROUND(spacing[2]);
      spacing[2] /= 100.0;
*/
      spacingList.push_back( spacing );
      origin0 = it->origin;
    }
    ++it;
  }

  //sort all spacings
  compare_Vector cV;
  spacingList.sort( cV );

  //copy the spacinglist and delete twice entries
  std::list<Vector3D> tempSpacingList = spacingList;
  tempSpacingList.unique();

  //no spacing found -> return defaultspacing
  if( tempSpacingList.size() == 0 ) return spacing;
  //one spacing found -> return it as result
  if( tempSpacingList.size() == 1 ) return tempSpacingList.front();
  //found different spacings -> count them and let the user decide
  //create a diolog for the user
  QmitkLightBoxReaderDialog myDialog( 0 );
  if( tempSpacingList.size() > 1 )
  {
    int currentCount = 0;
    Vector3D currentSpacing = spacingList.front();
    while( !spacingList.empty() )
    {
      if( currentSpacing == spacingList.front() )
      {
        currentCount++;
        spacingList.pop_front();
      }
      else
      {
        //add the found spacing to the dialog
        myDialog.addSpacings( currentSpacing, currentCount );
        currentCount = 1;
        currentSpacing = spacingList.front();
        spacingList.pop_front();
      }
    }
    //add the last counted spacing
    myDialog.addSpacings( currentSpacing, currentCount );
  }
  int dialogReturnValue = myDialog.exec();

  if ( dialogReturnValue == QDialog::Rejected )
  {
    //we have to abort the whole loadprocess
    abortLoadingTheLightbox = true;
    return spacing;
  }
  return myDialog.GetSpacing();
}

bool mitk::LightBoxImageReaderImpl::ImageOriginLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 )
{
  if( Equal( elem1.origin, elem2.origin ) )
  {
    if( elem1.imageNumber == elem2.imageNumber )
      return elem1.pos < elem2.pos;
    return elem1.imageNumber < elem2.imageNumber;
  }
  return elem1.origin**elem1.direction < elem2.origin**elem2.direction; // projection of origin on inter-slice-direction
};

bool mitk::LightBoxImageReaderImpl::ImageNumberLesser ( const LocalImageInfo& elem1, const LocalImageInfo& elem2 )
{
  return elem1.imageNumber < elem2.imageNumber;
};

void mitk::LightBoxImageReaderImpl::SortImage( LocalImageInfoArray& imageNumbers )
{
  ipPicDescriptor*  pic = NULL;
  ipPicTSV_t *tsv;
  void* data;
  ipUInt4_t len;
  int imageNumber;
  LocalImageInfo info;
  Vector3D direction;
  info.direction = &direction;
  bool directionInitialized = false;

  for (unsigned int position = 0; position < m_LightBox->getFrames(); ++position)
  {
    info.pos = position;

    pic = m_LightBox->fetchHeader(position);
    if( !pic )
    {
      std::cout << "****** LightBoxImageReaderImpl::SortImage(): pic is NULL" << std::endl;
      continue;
    }
    tsv=ipPicQueryTag(pic,"SOURCE HEADER");
    if( tsv )
    {
      bool ok = dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len );
      if( ok )
      {
        sscanf( (char *) data, "%d", &imageNumber );
        info.imageNumber = imageNumber;
      }
      else
      {
        ipPicTSV_t* imagenumberTag = ipPicQueryTag( pic, tagIMAGE_NUMBER );
        if( imagenumberTag && imagenumberTag->type == ipPicInt )
        {
          info.imageNumber = *( (int*)(imagenumberTag->value) );
        }
        else
        {
          info.imageNumber = position;
          itkWarningMacro(<<"No information about the imagenumber found in pic/dicom-header. At lighbox-position: "<<position);
        }
      }
      //itkDebugMacro(<<"number image: "<<imageNumber);
    }
    else
    {
      info.imageNumber = position;
      itkWarningMacro(<<"Pic-tag SOURCE HEADER is NULL at lighbox-position: "<<position);
    }
    interSliceGeometry_t *isg;
    isg = m_LightBox->fetchDicomGeometry( position );
    if( isg != NULL )
    {
      vtk2itk( isg->o, info.origin );
      if( directionInitialized == false )
      {
        VnlVector u(3), v(3);
        vtk2vnl( isg->u, u );
        vtk2vnl( isg->v, v );
        VnlVector normal = vnl_cross_3d(u, v);
        normal.normalize();
        direction.Set_vnl_vector( normal );
        itkDebugMacro(<<"DIRECTION: "<<direction);
        directionInitialized = true;
      }
    }
    else
    {
      FillVector3D( info.origin, 0, 0, -info.imageNumber );
      itkWarningMacro(<<"interSliceGeometry not available at lighbox-position "<<position);
      continue;
    }
    imageNumbers.push_back( info );
  }

  ////do not understand the necessity of the following anymore
  //LocalImageInfoArray::iterator it = imageNumbers.begin(), infoEnd=imageNumbers.end();

  //int maxNumber = std::max_element(it, infoEnd, ImageNumberLesser)->imageNumber;
  //int minNumber = std::min_element(it, infoEnd, ImageNumberLesser)->imageNumber;
  //int layers=imageNumbers.size()/maxNumber;//number of the layer

  //int afterFirstLayer=imageNumbers.size()-(maxNumber-minNumber+1);
  //if(afterFirstLayer>0)
  //{
  //  it+=afterFirstLayer;
  //  imageNumbers.erase(it, infoEnd);
  //}

  std::sort( imageNumbers.begin(), imageNumbers.end(), ImageOriginLesser );
}

int mitk::LightBoxImageReaderImpl::GetRealPosition( int position, LocalImageInfoArray& list )
{
  return list[position].pos;
}

const mitk::PropertyList::Pointer mitk::LightBoxImageReaderImpl::GetImageTagsAsPropertyList()
{
  if( m_LightBox == NULL )
  {
    itkWarningMacro( <<"Lightbox not set, using current lightbox" );
    m_LightBox = mitk::PACSPlugin::GetInstance()->GetCurrentLightbox();
  }
  if ( m_LightBox == NULL )
  {
    itkWarningMacro( << "No lightbox found." );
    return NULL;
  }
  if( m_LightBox->getFrames() == 0 )
  {
    itkWarningMacro( << "Lightbox empty." );
    return NULL;
  }

  PropertyList::Pointer resultPropertyList = PropertyList::New();

  ipPicDescriptor* imageToExtractTagsFrom = m_LightBox->fetchHeader(0); // TODO set this to the right descriptor
  if (!imageToExtractTagsFrom || !imageToExtractTagsFrom->info || !imageToExtractTagsFrom->info->tags_head) return NULL;

  // Extract ALL tags from the PIC header
  _ipPicTagsElement_t* currentTagNode = imageToExtractTagsFrom->info->tags_head;

  while( currentTagNode )
  {
    ipPicTSV_t* currentTag = currentTagNode->tsv;

    std::string propertyName = "CHILI: ";
    propertyName += currentTag->tag;

    switch( currentTag->type )
    {
      case ipPicASCII:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), new mitk::StringProperty( static_cast<char*>( currentTag->value ) ) );
        break;
      }
      case ipPicInt:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), new mitk::IntProperty( *static_cast<int*>( currentTag->value ) ) );
        break;
      }
      case ipPicUInt:
      {
        resultPropertyList->SetProperty( propertyName.c_str(), new mitk::IntProperty( (int)*( (char*)( currentTag->value ) ) ) );
        break;
      }
      default:  //ipPicUnknown, ipPicBool, ipPicFloat, ipPicNonUniform, ipPicTSV, _ipPicTypeMax
      {
        std::cout << "WARNING: Type of PIC-Tag '" << currentTag->type << "' not handled in LightBoxImageReader." << std::endl;
        break;
      }
    }
    // proceed to the next tag
    currentTagNode = currentTagNode->next;
  }

  BaseProperty::Pointer name = resultPropertyList->GetProperty( "CHILI: SERIES DESCRIPTION" );
  if( name )
  {
    resultPropertyList->SetProperty( "name", new mitk::StringProperty( name->GetValueAsString() ) );
  }
  else
  {
    if (m_LightBox->currentSeries())
    {
      if( m_LightBox->currentSeries()->description )
      {
        resultPropertyList->SetProperty( "name", new StringProperty( std::string( m_LightBox->currentSeries()->description ) ) );
      }
      else
        resultPropertyList->SetProperty( "name", new mitk::StringProperty( "unnamed" ) );
    }
  }
  return resultPropertyList;
}

bool mitk::LightBoxImageReaderImpl::userAbort()
{
  return abortLoadingTheLightbox;
}
