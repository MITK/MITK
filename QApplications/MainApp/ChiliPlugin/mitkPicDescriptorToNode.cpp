/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-06-15 14:28:00 +0200 (Fr, 15 Jun 2007) $
Version:   $Revision: 10778 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPicDescriptorToNode.h"

// Chili-Includes
#include <chili/isg.h>
#include <chili/plugin.h>
#include <ipPic/ipPicTags.h>
#include <ipDicom/ipDicom.h>
// MITK-Includes
#include "mitkProperties.h"
#include "mitkPlaneGeometry.h"
#include "mitkFrameOfReferenceUIDManager.h"
#include "mitkDataTreeNodeFactory.h"

//#include "mitkPicHeaderProperty.h"

#include "math.h"

// helper class to sort PicDescriptors primary by there imageNumber; if the imageNumber equal, the sliceLocation get used
class compare_PicDescriptor_ImageNumber
{
  public:
  bool operator() ( ipPicDescriptor* &first, ipPicDescriptor* &second)
  {
    //try to seperate them by imageNumber
    int imageNumberISG1 = 0, imageNumberISG2 = 0;

    ipPicTSV_t* imagenumberTag1 = ipPicQueryTag( first, tagIMAGE_NUMBER );
    if( imagenumberTag1 && imagenumberTag1->type == ipPicInt )
    {
      imageNumberISG1 = *( (int*)(imagenumberTag1->value) );
    }
    else
    {
      ipPicTSV_t *tsv;
      void* data;
      ipUInt4_t len;
      tsv = ipPicQueryTag( first, "SOURCE HEADER" );
      if( tsv )
      {
        if( dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len ) )
        {
          sscanf( (char *) data, "%d", &imageNumberISG1 );
        }
      }
    }

    ipPicTSV_t* imagenumberTag2 = ipPicQueryTag( second, tagIMAGE_NUMBER );
    if( imagenumberTag2 && imagenumberTag2->type == ipPicInt )
    {
      imageNumberISG2 = *( (int*)(imagenumberTag2->value) );
    }
    else
    {
      ipPicTSV_t *tsv;
      void* data;
      ipUInt4_t len;
      tsv = ipPicQueryTag( second, "SOURCE HEADER" );
      if( tsv )
      {
        if( dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len ) )
        {
          sscanf( (char *) data, "%d", &imageNumberISG2 );
        }
      }
    }
#ifdef CHILI_PLUGIN_VERSION_CODE
    if( imageNumberISG1 == imageNumberISG2 )
    {
      bool returnValue = false;
      interSliceGeometry_t* isg1 = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      interSliceGeometry_t* isg2 = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      if( pFetchSliceGeometryFromPic( first, isg1 ) && pFetchSliceGeometryFromPic( second, isg2 ) )
      {
        if( isg1->sl < isg2->sl )
        {
          returnValue = true;
        }
      }
      delete isg1;
      delete isg2;
      return returnValue;
    }
    else
#endif
    {
      if( imageNumberISG1 < imageNumberISG2 )
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  }
};

// helper class to sort PicDescriptors primary by there sliceLocation
class compare_PicDescriptor_SliceLocation
{
  public:
  bool operator() ( ipPicDescriptor* &first, ipPicDescriptor* &second)
  {
    bool returnValue = false;
#ifdef CHILI_PLUGIN_VERSION_CODE
    interSliceGeometry_t* isg1 = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    interSliceGeometry_t* isg2 = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    if( pFetchSliceGeometryFromPic( first, isg1 ) && pFetchSliceGeometryFromPic( second, isg2 ) )
    {
      if( isg1->sl < isg2->sl )
      {
        returnValue = true;
      }
    }
    delete isg1;
    delete isg2;
#endif
    return returnValue;
  }
};

// helper class to sort Vectors
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

// helper class to round
double mitk::PicDescriptorToNode::Round(double number, unsigned int decimalPlaces)
{
  double d = pow( 10.0, decimalPlaces );
  double x;
  if( number > 0 )
    x = floor( number * d + 0.5 ) / d;
  else
    x = floor( number * d - 0.5 ) / d;
  return x;
}

// constructor
mitk::PicDescriptorToNode::PicDescriptorToNode()
{
}

// destructor
mitk::PicDescriptorToNode::~PicDescriptorToNode()
{
}

// the set-function
void mitk::PicDescriptorToNode::SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID )
{
  m_SeriesOID = inputSeriesOID;
  m_PicDescriptorList = inputPicDescriptorList;
}

// the get-function
std::vector< mitk::DataTreeNode::Pointer > mitk::PicDescriptorToNode::GetOutput()
{
  return m_Output;
}

// the "main"-function
void mitk::PicDescriptorToNode::GenerateNodes()
{
  m_Output.clear();
  m_PossibleOutputs.clear();

  if( m_PicDescriptorList.size() > 0 && m_SeriesOID != "" )
  {
    CreatePossibleOutputs();
    SortByImageNumber();
    SeperateOutputsBySpacing();
    SeperateOutputsByTime();
    SplitDummiVolumes();
    SortBySliceLocation();
    CreateNodesFromOutputs();
    //DebugOutput();
  }
  else std::cout<<"PicDescriptorToNode-WARNING: No SeriesOID or PicDescriptorList set."<<std::endl;
}

void mitk::PicDescriptorToNode::CreatePossibleOutputs()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  // all slices without isg get ignored
  // sort by frameOfRefferenceUID --> all slices of one geometry have the same one
  // sort by "NormalenVektor" --> if they are parallel, the slices located one above the other
  // sort by pixelSpacing --> separation of extent
  // sort by SeriesDescription
  std::list< ipPicDescriptor* >::iterator picDescriptorCur = m_PicDescriptorList.begin();
  std::list< ipPicDescriptor* >::iterator picDescriptorEnd = m_PicDescriptorList.end();

  while( picDescriptorCur != picDescriptorEnd )
  {
    Vector3D rightVector, downVector, pixelSpacing, normale;
    std::string currentSeriesDescription = "";
    bool foundMatch; // true if the searched output exist
    int maxCount, curCount; // parameter to searching outputs

    //check intersliceGeomtry
    interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    if( !pFetchSliceGeometryFromPic( (*picDescriptorCur), isg ) )
    {
      //PicDescriptor without a geometry not able to sort in a volume
      std::cout<<"PicDescriptorToNode-WARNING: Found image without SliceGeometry. Image ignored."<<std::endl;
      picDescriptorCur++;
      delete isg;
      continue;
    }

    //"NormalenVektor"
    vtk2itk( isg->u, rightVector );
    vtk2itk( isg->v, downVector );
    normale[0] = ( ( rightVector[1]*downVector[2] ) - ( rightVector[2]*downVector[1] ) );
    normale[1] = ( ( rightVector[2]*downVector[0] ) - ( rightVector[0]*downVector[2] ) );
    normale[2] = ( ( rightVector[0]*downVector[1] ) - ( rightVector[1]*downVector[0] ) );

    //PixelSpacing
    vtk2itk( isg->ps, pixelSpacing );

    //SeriesDescription
    ipPicTSV_t* seriesDescriptionTag = ipPicQueryTag( (*picDescriptorCur), tagSERIES_DESCRIPTION );
    if( seriesDescriptionTag )
    {
      currentSeriesDescription = static_cast<char*>( seriesDescriptionTag->value );
    }
    if( currentSeriesDescription == "" )
    {
      ipPicTSV_t *tsv;
      void* data;
      ipUInt4_t len;
      tsv = ipPicQueryTag( (*picDescriptorCur), "SOURCE HEADER" );
      if( tsv )
      {
        if( dicomFindElement( (unsigned char*) tsv->value, 0x0008, 0x103e, &data, &len ) )
        {
          currentSeriesDescription = (char*)data;
        }
      }
    }

    //initialize searching
    foundMatch = false;
    curCount = 0;
    maxCount = m_PossibleOutputs.size();

    // searching for equal output
    while( curCount < maxCount && !foundMatch )
    {
      //check RefferenceUID, Pixelspacing and SeriesDescription
      if( isg->forUID == m_PossibleOutputs[ curCount ].refferenceUID && Equal(pixelSpacing, m_PossibleOutputs[ curCount ].pixelSpacing) && currentSeriesDescription == m_PossibleOutputs[ curCount ].seriesDescription )
      {
        //check if vectors are parallel (only if they have a lowest common multiple)
        foundMatch = true; // --> found the right output
        float referenceForParallelism = 0;
        if( normale[0] != m_PossibleOutputs[ curCount ].normale[0] )
        {
          referenceForParallelism = normale[0]/m_PossibleOutputs[ curCount ].normale[0];
        }
        if( normale[1] != m_PossibleOutputs[ curCount ].normale[1] )
        {
          if( referenceForParallelism == 0 )
          {
            referenceForParallelism = normale[1]/m_PossibleOutputs[ curCount ].normale[1];
          }
          else
          {
            if( referenceForParallelism != normale[1]/m_PossibleOutputs[ curCount ].normale[1] )
            {
              foundMatch = false;
            }
          }
        }
        if( normale[2] != m_PossibleOutputs[ curCount ].normale[2] )
        {
          if( referenceForParallelism == 0 )
          {
            referenceForParallelism = normale[2]/m_PossibleOutputs[ curCount ].normale[2];
          }
          else
          {
            if( referenceForParallelism != normale[2]/m_PossibleOutputs[ curCount ].normale[2] )
            {
              foundMatch = false;
            }
          }
        }
        if( !foundMatch )
        {
          curCount ++;
        }
      }
      else curCount ++; // the next output
    }

    // match found?
    if( foundMatch )
    {
      // add to an existing Output
      m_PossibleOutputs[curCount].descriptors.push_back( (*picDescriptorCur) );
    }
    else
    // create a new Output
    {
      DifferentOutputs newOutput;
      newOutput.refferenceUID = isg->forUID;
      newOutput.seriesDescription = currentSeriesDescription;
      newOutput.normale = normale;
      newOutput.pixelSpacing = pixelSpacing;
      newOutput.numberOfSlices = 0;
      newOutput.numberOfTimeSlices = 0;
      newOutput.differentTimeSlices = false;
      newOutput.descriptors.clear();
      newOutput.descriptors.push_back( (*picDescriptorCur) );
      m_PossibleOutputs.push_back( newOutput );
    }
    picDescriptorCur ++;
    delete isg;
  }
#endif
}

//sort the descriptors of the PossibleOutputs
void mitk::PicDescriptorToNode::SortByImageNumber()
{
  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    compare_PicDescriptor_ImageNumber cPI;
    m_PossibleOutputs[n].descriptors.sort( cPI );
  }
}

void mitk::PicDescriptorToNode::SortBySliceLocation()
{
  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    compare_PicDescriptor_SliceLocation cPS;
    m_PossibleOutputs[n].descriptors.sort( cPS );
  }
}

// separation on spacing and set minimum timslices and minimum slices
void mitk::PicDescriptorToNode::SeperateOutputsBySpacing()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    std::list< ipPicDescriptor* >::iterator iter = m_PossibleOutputs[n].descriptors.begin();
    std::list< ipPicDescriptor* >::iterator iterend = m_PossibleOutputs[n].descriptors.end();
    bool InitializedSpacingAndTime = false;
    int numberOfSlices = 0, currentCount = 0, numberOfTimeSlices = 0;
    interSliceGeometry_t* isg;
    Vector3D spacing, tempSpacing, origincur, originb4;
    //end() return a iterator after the last element, but we want the last element
    iterend--;

    //set the default-spacing to output
    isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
    if( !pFetchSliceGeometryFromPic( (*iter), isg ) )
    {
      delete isg;
      return;
    }

    vtk2itk( isg->ps, spacing );
    if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
      spacing.Fill(1.0);
    for (unsigned int i = 0; i < 3; ++i)
      spacing[i] = Round( spacing[i], 2 );
    m_PossibleOutputs[n].sliceSpacing = spacing;

    //start checking for spacing if the output contain more than one slice
    if( m_PossibleOutputs[n].descriptors.size() > 1 )
    {
      vtk2itk( isg->o, origincur );

      while( iter != iterend )
      {
        originb4 = origincur;
        iter ++;
        currentCount ++;

        if( !pFetchSliceGeometryFromPic( (*iter), isg ) )
        {
          delete isg;
          return;
        }

        vtk2itk( isg->o, origincur );

        // they are equal
        if( Equal( originb4, origincur ) )
          numberOfTimeSlices ++;
        else
        {
          // they are different
          tempSpacing = origincur - originb4;
          spacing[2] = tempSpacing.GetNorm();
          spacing[2] = Round( spacing[2], 2 );

          // spacing and timeslices not initialized yet
          if( !InitializedSpacingAndTime )
          {
            numberOfSlices ++;
            InitializedSpacingAndTime = true;
            m_PossibleOutputs[n].numberOfTimeSlices = numberOfTimeSlices;
            m_PossibleOutputs[n].sliceSpacing = spacing;
            numberOfTimeSlices = 0;
          }
          else
          // spacing and timeslices initialized
          {
            if( spacing == m_PossibleOutputs[n].sliceSpacing )
            {
              numberOfSlices ++;
              if( m_PossibleOutputs[n].numberOfTimeSlices != numberOfTimeSlices )
              {
                m_PossibleOutputs[n].differentTimeSlices = true;
                if( m_PossibleOutputs[n].numberOfTimeSlices > numberOfTimeSlices )
                  m_PossibleOutputs[n].numberOfTimeSlices = numberOfTimeSlices;
              }
              numberOfTimeSlices = 0;
            }
            else
            {
              // the current found spacing get illustrated in one Output, the remaining slices get relocated in a new Output and checked the same way later
              DifferentOutputs newOutput;
              newOutput.refferenceUID = m_PossibleOutputs[n].refferenceUID;
              newOutput.seriesDescription = m_PossibleOutputs[n].seriesDescription;
              newOutput.normale = m_PossibleOutputs[n].normale;
              newOutput.pixelSpacing = m_PossibleOutputs[n].pixelSpacing;
              newOutput.numberOfSlices = - 1;
              newOutput.numberOfTimeSlices = - 1;
              newOutput.differentTimeSlices = false;
              newOutput.descriptors.clear();
              // we dont want to cut until the last element, we cut after the last element
              iterend ++;
              newOutput.descriptors.assign( iter, iterend );
              iterend --;
              m_PossibleOutputs[n].descriptors.resize( currentCount );
              m_PossibleOutputs.push_back( newOutput );
              iterend = iter;
            }
          }
        }
      }
    }
    m_PossibleOutputs[n].numberOfSlices = numberOfSlices;

    //check last slice for time
    if( m_PossibleOutputs[n].numberOfTimeSlices != numberOfTimeSlices )
    {
      m_PossibleOutputs[n].differentTimeSlices = true;
      if( numberOfTimeSlices < m_PossibleOutputs[n].numberOfTimeSlices )
        m_PossibleOutputs[n].numberOfTimeSlices = numberOfTimeSlices;
    }

    // 2D-Dataset or timesliced 2D-Dataset
    if( m_PossibleOutputs[n].descriptors.size() == 1 || !InitializedSpacingAndTime )
    {
      m_PossibleOutputs[n].sliceSpacing = spacing;
      m_PossibleOutputs[n].numberOfTimeSlices = numberOfTimeSlices;
    }
    delete isg;
  }
#endif
}

//separate the PossibleOutputs by time
void mitk::PicDescriptorToNode::SeperateOutputsByTime()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    if( m_PossibleOutputs[n].differentTimeSlices )
    {
      int curTime = 0, lastTime = 0;
      bool deleteIterator = false;
      Vector3D origincur, originb4;

      DifferentOutputs* timeOutput = NULL;

      interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      if( !pFetchSliceGeometryFromPic( (*m_PossibleOutputs[n].descriptors.begin()), isg ) )
      {
        delete isg;
        return;
      }

      vtk2itk( isg->o, origincur );

      std::list< ipPicDescriptor* >::iterator iter = m_PossibleOutputs[n].descriptors.begin();
      std::list< ipPicDescriptor* >::iterator iterend = m_PossibleOutputs[n].descriptors.end();
      iterend--;

      while( iter != iterend )
      {
        if( deleteIterator )
        //the ipPicDescriptor was saved to "timeOuput", so we can delete them from the current Output
        {
          iter = m_PossibleOutputs[n].descriptors.erase( iter );
          deleteIterator = false;
        }
        else
        {
          iter ++;
          originb4 = origincur;
        }

        //get the current origin
        if( !pFetchSliceGeometryFromPic( (*iter), isg ) )
        {
          delete isg;
          return;
        }

        vtk2itk( isg->o, origincur );

        //check them
        if( Equal( originb4, origincur ) )
        {
          curTime ++;
          if( curTime > m_PossibleOutputs[n].numberOfTimeSlices )
          {
            deleteIterator = true;
            if( timeOutput == NULL )
            {
              //create a "new" Output
              timeOutput = new DifferentOutputs;
              timeOutput->refferenceUID = m_PossibleOutputs[n].refferenceUID;
              timeOutput->seriesDescription = m_PossibleOutputs[n].seriesDescription;
              timeOutput->normale = m_PossibleOutputs[n].normale;
              timeOutput->pixelSpacing = m_PossibleOutputs[n].pixelSpacing;
              timeOutput->sliceSpacing = m_PossibleOutputs[n].sliceSpacing;
              timeOutput->numberOfSlices = - 1;
              timeOutput->numberOfTimeSlices = - 1;
              timeOutput->differentTimeSlices = false;
              timeOutput->descriptors.clear();
              timeOutput->descriptors.push_back( (*iter) );
            }
            else
            {
              //add to output
              timeOutput->descriptors.push_back( (*iter) );
            }
          }
        }
        else
        //they are not equal
        {
          if( curTime > m_PossibleOutputs[n].numberOfTimeSlices )
          //set slice and timeslice
          {
            timeOutput->numberOfSlices ++;
            if( timeOutput->numberOfTimeSlices == -1 )
            {
              timeOutput->numberOfTimeSlices = ( curTime - m_PossibleOutputs[n].numberOfTimeSlices - 1 );
            }
            else
            {
              if( curTime < timeOutput->numberOfTimeSlices )
              {
                timeOutput->numberOfTimeSlices = ( curTime - m_PossibleOutputs[n].numberOfTimeSlices - 1 );
                timeOutput->differentTimeSlices = true;
              }
              else
              {
                timeOutput->differentTimeSlices = true;
              }
            }
          }
          if( curTime == m_PossibleOutputs[n].numberOfTimeSlices && timeOutput != NULL )
          {
            // "close" output
            // its possible that "push_back" have to increase the capacity, then all iterator become void
            // so we have to set the iterator new
            ipPicDescriptor* tempDescriptor = (*iter);
            m_PossibleOutputs.push_back( (*timeOutput) );
            iter = m_PossibleOutputs[n].descriptors.begin();
            iterend = m_PossibleOutputs[n].descriptors.end();
            iterend--;
            while( (*iter) != tempDescriptor ) iter++;
            delete timeOutput;
            timeOutput = NULL;
          }
          lastTime = curTime;
          curTime = 0;
        }
      }

      //dont forget to check the last slice
      if( deleteIterator )
      //delete the slice
        iter = m_PossibleOutputs[n].descriptors.erase( iter );

      if( curTime > m_PossibleOutputs[n].numberOfTimeSlices )
      //set slice and timeslice
      {
        timeOutput->numberOfSlices ++;
        if( timeOutput->numberOfTimeSlices == -1 )
          timeOutput->numberOfTimeSlices = ( curTime - m_PossibleOutputs[n].numberOfTimeSlices - 1 );
        else
          if( curTime < timeOutput->numberOfTimeSlices )
          {
            timeOutput->numberOfTimeSlices = ( curTime - m_PossibleOutputs[n].numberOfTimeSlices - 1 );
            timeOutput->differentTimeSlices = true;
          }
          else
            timeOutput->differentTimeSlices = true;
      }

      if( timeOutput != NULL )
      // "close" output
      {
        m_PossibleOutputs.push_back( (*timeOutput) );
        delete timeOutput;
      }

      delete isg;
    }
    m_PossibleOutputs[n].differentTimeSlices = false;
  }
#endif
}

void mitk::PicDescriptorToNode::SplitDummiVolumes()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  Vector3D spacing;
  interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );

  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    if( m_PossibleOutputs[n].numberOfSlices == 1 && m_PossibleOutputs[n].numberOfTimeSlices == 0 && m_PossibleOutputs[n].differentTimeSlices == false )
    {
      //create a "new" Output
      DifferentOutputs new2DOutput;
      new2DOutput.refferenceUID = m_PossibleOutputs[n].refferenceUID;
      new2DOutput.seriesDescription = m_PossibleOutputs[n].seriesDescription;
      new2DOutput.normale = m_PossibleOutputs[n].normale;
      new2DOutput.pixelSpacing = m_PossibleOutputs[n].pixelSpacing;

      //set the spacing for the new output
      if( !pFetchSliceGeometryFromPic( m_PossibleOutputs[n].descriptors.front(), isg ) )
      {
        delete isg;
        return;
      }

      vtk2itk( isg->ps, spacing );
      if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
        spacing.Fill(1.0);
      for (unsigned int i = 0; i < 3; ++i)
        spacing[i] = Round( spacing[i], 2 );
      new2DOutput.sliceSpacing = spacing;

      new2DOutput.numberOfSlices = 0;
      new2DOutput.numberOfTimeSlices = 0;
      new2DOutput.differentTimeSlices = false;
      new2DOutput.descriptors.clear();
      new2DOutput.descriptors.push_back( m_PossibleOutputs[n].descriptors.front() );
      m_PossibleOutputs[n].descriptors.pop_front();

      //set the spacing for the source output
      if( !pFetchSliceGeometryFromPic( m_PossibleOutputs[n].descriptors.front(), isg ) )
      {
        delete isg;
        return;
      }
      vtk2itk( isg->ps, spacing );
      if( spacing[0] == 0 && spacing[1] == 0 && spacing[2] == 0 )
        spacing.Fill(1.0);
      for (unsigned int i = 0; i < 3; ++i)
        spacing[i] = Round( spacing[i], 2 );
      m_PossibleOutputs[n].sliceSpacing = spacing;

      m_PossibleOutputs[n].numberOfSlices = 0;
      m_PossibleOutputs.push_back( new2DOutput );
    }
  }
  delete isg;
#endif
}

//create the mitk::DataTreeNodes from the possible Outputs
void mitk::PicDescriptorToNode::CreateNodesFromOutputs()
{
#ifdef CHILI_PLUGIN_VERSION_CODE
  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    //check the count of slices with the numberOfSlices and numberOfTimeSlices
    if( (unsigned int)( ( m_PossibleOutputs[n].numberOfSlices+1 ) * ( m_PossibleOutputs[n].numberOfTimeSlices+1 ) ) != m_PossibleOutputs[n].descriptors.size() )
      std::cout<<"PicDescriptorToNode-ERROR: For Output"<<n<<" ("<<m_PossibleOutputs[n].seriesDescription<<") calculated slicecount is not equal to the existing slices. Output closed."<<std::endl;
    else
      if(  m_PossibleOutputs[n].differentTimeSlices == true )
        std::cout<<"PicDescriptorToNode-ERROR: Output"<<n<<" ("<<m_PossibleOutputs[n].seriesDescription<<") have different numbers of timeslices. Function SeperateOutputsByTime() dont work right. Output closed."<<std::endl;
    else
    {
      //create mitk::Image
      int slice = 0, time = 0;
      interSliceGeometry_t* isg = (interSliceGeometry_t*) malloc ( sizeof(interSliceGeometry_t) );
      Point3D origin;
      Vector3D rightVector, downVector, origincur, originb4;
      ipPicDescriptor* header;
      Image::Pointer resultImage;
      DataTreeNode::Pointer node = mitk::DataTreeNode::New();
      PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
      SlicedGeometry3D::Pointer slicedGeometry = SlicedGeometry3D::New();
      TimeSlicedGeometry::Pointer timeSliceGeometry = TimeSlicedGeometry::New();

      header = ipPicCopyHeader( m_PossibleOutputs[n].descriptors.front(), NULL );

      //2D
      if( m_PossibleOutputs[n].numberOfSlices == 0 )
      {
        if( m_PossibleOutputs[n].numberOfTimeSlices == 0 )
        {
          header->dim = 2;
          header->n[2] = 0;
          header->n[3] = 0;
        }
        // +t
        else
        {
          header->dim = 4;
          header->n[2] = 1;
          header->n[3] = m_PossibleOutputs[n].numberOfTimeSlices+1;
        }
      }
      //3D
      else
      {
        if( m_PossibleOutputs[n].numberOfTimeSlices == 0 )
        {
          header->dim = 3;
          header->n[2] = m_PossibleOutputs[n].numberOfSlices+1;
          header->n[3] = 0;
        }
        // +t
        else
        {
          header->dim = 4;
          header->n[2] = m_PossibleOutputs[n].numberOfSlices+1;
          header->n[3] = m_PossibleOutputs[n].numberOfTimeSlices+1;
        }
      }
      resultImage = Image::New();
      resultImage->Initialize( header );
      if( !pFetchSliceGeometryFromPic( m_PossibleOutputs[n].descriptors.front(), isg ) )
      {
        delete isg;
        return;
      }
      vtk2itk( isg->u, rightVector );
      vtk2itk( isg->v, downVector );
      vtk2itk( isg->o, origin );

      //its possible that a 2D-Image have no right- or down-Vector,but its not possible to initialize a [0,0,0] vector
      if( rightVector[0] == 0 && rightVector[1] == 0 && rightVector[2] == 0 )
        rightVector[0] = 1;
      if( downVector[0] == 0 && downVector[1] == 0 && downVector[2] == 0 )
        downVector[2] = -1;

      //set the timeBounds
      ScalarType timeBounds[] = {0.0, 1.0};
      planegeometry->InitializeStandardPlane( resultImage->GetDimension(0), resultImage->GetDimension(1), rightVector, downVector, &m_PossibleOutputs[n].sliceSpacing );
      planegeometry->SetOrigin( origin );
      planegeometry->SetFrameOfReferenceID( FrameOfReferenceUIDManager::AddFrameOfReferenceUID( m_PossibleOutputs[n].refferenceUID.c_str() ) );
      planegeometry->SetTimeBounds( timeBounds );
      slicedGeometry->InitializeEvenlySpaced( planegeometry, resultImage->GetDimension(2) );

      timeSliceGeometry->InitializeEvenlyTimed( slicedGeometry, resultImage->GetDimension(3) );
      timeSliceGeometry->TransferItkToVtkTransform();
      resultImage->SetGeometry( timeSliceGeometry );
      std::list< ipPicDescriptor* >::iterator iter = m_PossibleOutputs[n].descriptors.begin();
      std::list< ipPicDescriptor* >::iterator iterend = m_PossibleOutputs[n].descriptors.end();

      //add the slices to the created mitk::Image
      while( iter != iterend )
      {
        resultImage->SetPicSlice( (*iter), slice, time );
        iter ++;
        if( time < m_PossibleOutputs[n].numberOfTimeSlices )
        {
          time ++;
        }
        else
        {
          time = 0;
          slice ++;
        }
      }

      //if all okay create a node, add the NumberOfSlices, NumberOfTimeSlices, SeriesOID, name and data
      if( resultImage->IsInitialized() && resultImage.IsNotNull() )
      {
        node->SetData( resultImage );
        DataTreeNodeFactory::SetDefaultImageProperties( node );

        if( m_PossibleOutputs[n].seriesDescription == "" )
          m_PossibleOutputs[n].seriesDescription = "no SeriesDescription";
        node->SetProperty( "name", new StringProperty( m_PossibleOutputs[n].seriesDescription ) );
        node->SetProperty( "NumberOfSlices", new IntProperty( m_PossibleOutputs[n].numberOfSlices+1 ) );
        node->SetProperty( "NumberOfTimeSlices", new IntProperty( m_PossibleOutputs[n].numberOfTimeSlices+1 ) );
        if( m_SeriesOID != "" )
          node->SetProperty( "SeriesOID", new StringProperty( m_SeriesOID ) );
/*
        mitk::PropertyList::Pointer tempPropertyList = CreatePropertyListFromPicTags( m_PossibleOutputs[n].descriptors.front() );
        for( mitk::PropertyList::PropertyMap::const_iterator iter = tempPropertyList->GetMap()->begin(); iter != tempPropertyList->GetMap()->end(); iter++ )
        {
          node->SetProperty( iter->first.c_str(), iter->second.first );
        }
*/
        m_Output.push_back( node );
      }
      delete isg;
    }
  }
#endif
}

/*
const mitk::PropertyList::Pointer mitk::PicDescriptorToNode::CreatePropertyListFromPicTags( ipPicDescriptor* imageToExtractTagsFrom )
{
  if( !imageToExtractTagsFrom || !imageToExtractTagsFrom->info || !imageToExtractTagsFrom->info->tags_head )
    return NULL;

  PropertyList::Pointer resultPropertyList = PropertyList::New();
  _ipPicTagsElement_t* currentTagNode = imageToExtractTagsFrom->info->tags_head;

  // Extract ALL tags from the PIC header
  while (currentTagNode)
  {
    ipPicTSV_t* currentTag = currentTagNode->tsv;
    // process this tag
    BaseProperty::Pointer newProperty = new PicHeaderProperty( currentTag );
    std::string propertyName = "CHILI: ";
    propertyName += currentTag->tag;
    resultPropertyList->SetProperty( propertyName.c_str(), newProperty );
    // proceed to the next tag
    currentTagNode = currentTagNode->next;
  }

  return resultPropertyList;
}
*/

// show all PossibleOutputs and the descriptors
void mitk::PicDescriptorToNode::DebugOutput()
{
  for( unsigned int n = 0; n < m_PossibleOutputs.size(); n++)
  {
    //the Head of all ipPicDescriptors
    std::cout << "-----------" << std::endl;
    std::cout << "PicDescriptorToNode-Output" << n << std::endl;
    std::cout << "ReferenceUID:" << m_PossibleOutputs[n].refferenceUID << std::endl;
    std::cout << "SeriesDescription:" << m_PossibleOutputs[n].seriesDescription << std::endl;
    std::cout << "Normale:" << m_PossibleOutputs[n].normale << std::endl;
    std::cout << "PixelSpacing:" << m_PossibleOutputs[n].pixelSpacing << std::endl;
    std::cout << "SliceSpacing:" << m_PossibleOutputs[n].sliceSpacing << std::endl;
    std::cout << "NumberOfSlices:" << m_PossibleOutputs[n].numberOfSlices << std::endl;
    std::cout << "NumberOfTimeSlices:" << m_PossibleOutputs[n].numberOfTimeSlices << std::endl;
    std::cout << "DifferentTimeSlices (bool):" << m_PossibleOutputs[n].differentTimeSlices << std::endl;
    std::cout << "-----------" << std::endl;

    //every single descriptor by the ImageNumber
    for( std::list< ipPicDescriptor* >::iterator it = m_PossibleOutputs[n].descriptors.begin(); it != m_PossibleOutputs[n].descriptors.end(); it++ )
    {
      int imageNumber = 0;
      ipPicTSV_t *tsv;
      void* data;
      ipUInt4_t len;
      tsv = ipPicQueryTag( (*it), "SOURCE HEADER" );
      if( tsv )
      {
        bool ok = dicomFindElement( (unsigned char*) tsv->value, 0x0020, 0x0013, &data, &len );
        if( ok )
          sscanf( (char *) data, "%d", &imageNumber );
      }
      if( imageNumber == 0)
      {
        ipPicTSV_t* imagenumberTag = ipPicQueryTag( (*it), tagIMAGE_NUMBER );
        if( imagenumberTag && imagenumberTag->type == ipPicInt )
          imageNumber = *( (int*)(imagenumberTag->value) );
      }
      std::cout<<"Image: "<<imageNumber<<std::endl;
    }
    std::cout<<""<<std::endl;
  }
}
