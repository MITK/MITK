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
#ifndef _itkAdaptiveThresholdIterator_txx
#define _itkAdaptiveThresholdIterator_txx

#include "itkAdaptiveThresholdIterator.h"
#include "mitkProgressBar.h"
#include <math.h>


namespace itk
{

  template<class TImage, class TFunction>
  AdaptiveThresholdIterator<TImage, TFunction>
    ::AdaptiveThresholdIterator(ImageType *imagePtr,
    FunctionType *fnPtr,
    IndexType startIndex)
    :m_FineDetectionMode(false), m_DetectionStop(false)
  {
    this->m_OutputImage = imagePtr;
    m_Function = fnPtr;
    m_StartIndices.push_back ( startIndex );

    // Set up the temporary image
    this->InitializeIterator();
  }

  template<class TImage, class TFunction>
  AdaptiveThresholdIterator<TImage, TFunction>
    ::AdaptiveThresholdIterator(ImageType *imagePtr,
    FunctionType *fnPtr,
    std::vector<IndexType>& startIndex)
    :m_FineDetectionMode(false), m_DetectionStop(false)
  {
    this->m_OutputImage = imagePtr;
    m_Function = fnPtr;
    unsigned int i;
    for (i = 0; i < startIndex.size(); i++ )
    {
      m_StartIndices.push_back ( startIndex[i] );
    }

    // Set up the temporary image
    this->InitializeIterator();
  }

  template<class TImage, class TFunction>
  AdaptiveThresholdIterator<TImage, TFunction>
    ::AdaptiveThresholdIterator(ImageType *imagePtr,
    FunctionType *fnPtr)
    :m_FineDetectionMode(false), m_DetectionStop(false)
  {
    this->m_OutputImage = imagePtr; //here we store the image, we have to wite the result to
    m_Function = fnPtr;

    // Set up the temporary image
    this->InitializeIterator();
  }


  template<class TImage, class TFunction>
  void AdaptiveThresholdIterator<TImage, TFunction>
    ::InitializeIterator()
  {

    // Get the origin and spacing from the image in simple arrays
    m_ImageOrigin  = this->m_OutputImage->GetOrigin();
    m_ImageSpacing = this->m_OutputImage->GetSpacing();
    m_ImageRegion  = this->m_OutputImage->GetBufferedRegion();

    this->InitRegionGrowingState();

    m_VoxelCounter = 0;
    m_LastVoxelNumber = 0;
    m_CurrentLeakageRatio = 0;
    m_DetectedLeakagePoint = 0;
  }

  template <class TImage, class TFunction>
  void AdaptiveThresholdIterator<TImage, TFunction>
    ::SetExpansionDirection(bool upwards)
  {
    m_UpwardsExpansion = upwards;
  }

  template <class TImage, class TFunction>
  void AdaptiveThresholdIterator<TImage, TFunction>
    ::IncrementRegionGrowingState()
  {
    //make the progressbar go one step further
    if(!m_FineDetectionMode)
      mitk::ProgressBar::GetInstance()->Progress();

    //updating the thresholds
    if (m_UpwardsExpansion)
    {
      this->ExpandThresholdUpwards();
    }
    else
    {
      this->ExpandThresholdDownwards();
    }

    //leakage-detection
    int criticalValue = 2000;// calculate a bit more "intelligent"

    if (! m_FineDetectionMode)
    {
      int diff = m_VoxelCounter - m_LastVoxelNumber;
      if (diff > m_CurrentLeakageRatio)
      {
        m_CurrentLeakageRatio = diff;
        m_DetectedLeakagePoint = m_RegionGrowingState;
      }
      m_LastVoxelNumber = m_VoxelCounter;
      m_VoxelCounter = 0;
    }
    else //fine leakage detection
    {
      //counting voxels over interations; if above a critical value (to be extended) then set this to leakage
      int diff = m_VoxelCounter - m_LastVoxelNumber;
      //std::cout<<"diff is: "<<diff<<"\n";
      if (diff <= criticalValue && (!m_DetectionStop))
      {
        //m_CurrentLeakageRatio = diff;
        m_DetectedLeakagePoint = m_RegionGrowingState + 1; //TODO check why this is needed
        //std::cout<<"setting CurrentLeakageRatio to: "<<diff<<" and leakagePoint to: "<<m_DetectedLeakagePoint<<"\n";
      }
      else
      {
        m_DetectionStop = true;
        // std::cout<<"\n\n[ITERATOR] detection stop!!!\n";
      }
      m_LastVoxelNumber = m_VoxelCounter;
      m_VoxelCounter = 0;
    }
    //increment the counter
    m_RegionGrowingState++;
  }

  template <class TImage, class TFunction>
  void AdaptiveThresholdIterator<TImage, TFunction>
    ::ExpandThresholdUpwards()
  {
    int upper = (int) m_Function->GetUpper();
    upper++;
    m_Function->ThresholdBetween(m_MinTH, upper);

  }


  template <class TImage, class TFunction>
  void AdaptiveThresholdIterator<TImage, TFunction>
    ::ExpandThresholdDownwards()
  {
    int lower = (int) m_Function->GetLower();
    lower--;
    m_Function->ThresholdBetween(lower, m_MaxTH);

  }

  template<class TImage, class TFunction>
  void AdaptiveThresholdIterator<TImage, TFunction>
    ::InitRegionGrowingState()
  {
    this->m_RegionGrowingState = 1;

  }

  template<class TImage, class TFunction>
  int
    AdaptiveThresholdIterator<TImage, TFunction>
    ::EstimateDistance(IndexType tempIndex)
  {
    PixelType value = this->m_Function->GetInputImage()->GetPixel(tempIndex);
    PixelType minPixel = PixelType(m_MinTH);
    PixelType maxPixel = PixelType(m_MaxTH);

    if (value > maxPixel || value < minPixel)
    {
      return 0;
    }
    if (m_UpwardsExpansion)
    {
      return (int)(value - m_SeedPointValue );
    }
    else
    {
      return (int)( m_SeedPointValue  - value );
    }
  }

  template<class TImage, class TFunction>
  void
    AdaptiveThresholdIterator<TImage, TFunction>
    ::SetMinTH(int min)
  {
    m_MinTH = min;
  }

  template<class TImage, class TFunction>
  void
    AdaptiveThresholdIterator<TImage, TFunction>
    ::SetMaxTH(int max)
  {
    m_MaxTH = max;
  }

  template<class TImage, class TFunction>
  int
    AdaptiveThresholdIterator<TImage, TFunction>
    ::GetSeedPointValue()
  {
    return this->m_SeedPointValue;
  }

  template<class TImage, class TFunction>
  void
    AdaptiveThresholdIterator<TImage, TFunction>
    ::GoToBegin()
  {
    m_QueueMap.clear();

    m_SeedPointValue = 0;

    IndexType seedIndex = m_StartIndices[0];

    bool doAverage = false; //enable or disable manually!
    if (doAverage)
    {
      //loops for creating the sum of the N27-neighborhood around the seedpoint
      for (int i = -1; i<=1; i++)
      {
        for (int j = -1; j<=1; j++)
        {
          for (int k = -1; k<=1; k++)
          {
            seedIndex[0]=seedIndex[0]+i;
            seedIndex[1]=seedIndex[1]+j;
            seedIndex[2]=seedIndex[2]+k;

            m_SeedPointValue += (int) m_Function->GetInputImage()->GetPixel(seedIndex);
          }
        }
      }

      //value of seedpoint computed from mean of N27-neighborhood
      m_SeedPointValue = m_SeedPointValue / 27;
    }
    else
    {
      m_SeedPointValue = (int) m_Function->GetInputImage()->GetPixel(seedIndex);
    }

    this->CheckSeedPointValue();

    m_InitializeValue = (this->CalculateMaxRGS() +1);
    if(!m_FineDetectionMode)
      mitk::ProgressBar::GetInstance()->AddStepsToDo(m_InitializeValue-1);

    //only initialize with zeros for the first segmention (raw segmentation mode)
    if(!m_FineDetectionMode)
    {
      this->m_OutputImage->FillBuffer((PixelType) 0 );
    }
    if (m_UpwardsExpansion)
    {
      m_Function->ThresholdBetween(m_MinTH, m_SeedPointValue);
    }
    else
    {
      m_Function->ThresholdBetween(m_SeedPointValue, m_MaxTH);
    }

    this->m_IsAtEnd = true;

    seedIndex = m_StartIndices[0];// warum noch mal? Steht doch schon in Zeile 224

    if( this->m_OutputImage->GetBufferedRegion().IsInside (seedIndex ) &&
      this->m_SeedPointValue > this->m_MinTH && this->m_SeedPointValue < this->m_MaxTH)
    {
      // Push the seed onto the queue
      this->InsertIndexTypeIntoQueueMap(m_RegionGrowingState, seedIndex);

      // Obviously, we're at the beginning
      this->m_IsAtEnd = false;

      this->m_OutputImage->SetPixel(seedIndex, (m_InitializeValue - m_RegionGrowingState));

    }
  }



  template<class TImage, class TFunction>
  void
    AdaptiveThresholdIterator<TImage, TFunction>
    ::CheckSeedPointValue ()
  {
    //checks, if the value, that has been averaged over the N-27 neighborhood aorund the seedpoint is still inside
    //the thresholds-ranges. if not, the actual value of the seedpoint (not averaged) is used
    if (m_SeedPointValue < m_MinTH || m_SeedPointValue > m_MaxTH)
    {
      m_SeedPointValue = (int) m_Function->GetInputImage()->GetPixel(m_StartIndices[0]);
    }
  }


  template<class TImage, class TFunction>
  unsigned int
    AdaptiveThresholdIterator<TImage, TFunction>
    ::CalculateMaxRGS ()
  {
    if (m_UpwardsExpansion)
    {
      return (m_MaxTH-m_SeedPointValue);
    }
    else
    {
      return (m_SeedPointValue-m_MinTH);
    }
  }


  template<class TImage, class TFunction>
  bool
    AdaptiveThresholdIterator<TImage, TFunction>
    ::IsPixelIncluded(const IndexType & index) const
  {
    //checks, if grayvalue of current voxel is inside the currently used thresholds
    return this->m_Function->EvaluateAtIndex(index);
  }


  template<class TImage, class TFunction>
  void
    AdaptiveThresholdIterator<TImage, TFunction>
    ::InsertIndexTypeIntoQueueMap (unsigned int key, IndexType index)
  {
    //first check if the key-specific queue already exists
    if (m_QueueMap.count(key) == 0)
    {
      //if queue doesn´t exist, create it, push the IndexType onto it
      //and insert it into the map

      IndexQueueType newQueue;
      newQueue.push(index);

      typedef typename QueueMapType::value_type KeyIndexQueue;

      m_QueueMap.insert(KeyIndexQueue(key, newQueue));
    }
    else
    {
      //if queue already exists in the map, push IndexType onto its specific queue
      (* (m_QueueMap.find(key)) ).second.push(index);
    }
  }





  template<class TImage, class TFunction>
  void
    AdaptiveThresholdIterator<TImage, TFunction>
    ::DoExtendedFloodStep()
  {

    // The index in the front of the queue should always be
    // valid and be inside since this is what the iterator
    // uses in the Set/Get methods. This is ensured by the
    // GoToBegin() method.


    typename QueueMapType::iterator currentIt = m_QueueMap.find(m_RegionGrowingState);


    if (currentIt == m_QueueMap.end())
    {
      this->IncrementRegionGrowingState();

    }
    else
    {
      IndexQueueType* currentQueue = & (* currentIt).second;

      // Take the index in the front of the queue
      const IndexType & topIndex = currentQueue->front();

      // Iterate through all possible dimensions
      // NOTE: Replace this with a ShapeNeighborhoodIterator
      for(unsigned int i=0; i<NDimensions; i++)
      {
        // The j loop establishes either left or right neighbor (+-1)
        for(int j=-1; j<=1; j+=2)
        {
          IndexType tempIndex;

          // build the index of a neighbor
          for(unsigned int k=0; k<NDimensions; k++)
          {
            if( i!=k )
            {
              tempIndex.m_Index[k] = topIndex[k];
            }
            else
            {
              tempIndex.m_Index[k] = topIndex[k] + j;
            }
          } // end build the index of a neighbor

          // If this is a valid index and have not been tested,
          // then test it.
          if( m_ImageRegion.IsInside( tempIndex ) )
          {
            //check if voxel hasn´t already been processed
            if( this->m_OutputImage->GetPixel( tempIndex ) == 0 )
            {
              // if it is inside, push it into the queue
              if(  this->IsPixelIncluded( tempIndex ) )
              {
                //hier wird Voxel in momentan aktiven Stack und ins OutputImage geschrieben
                this->InsertIndexTypeIntoQueueMap( (m_RegionGrowingState ),tempIndex);
                this->m_OutputImage->SetPixel( tempIndex, (m_InitializeValue - m_RegionGrowingState));
              }
              else  // If the pixel is not inside the current threshold
              {
                int distance = this->EstimateDistance(tempIndex); // [!] sollte nicht estimateDistance sondern calculateDistance() heißen!
                if (distance != 0)
                {
                  //hier wird Voxel in entsprechenden Stack und ins OutputImage geschrieben
                  this->InsertIndexTypeIntoQueueMap( (distance),tempIndex);
                  this->m_OutputImage->SetPixel( tempIndex, (m_InitializeValue -  distance));

                }
              }
            }
          }
        } // end left/right neighbor loop
      } // end check all neighbors

      // Now that all the potential neighbors have been
      // inserted we can get rid of the pixel in the front
      currentQueue->pop();
      m_VoxelCounter++;

      if( currentQueue->empty())
      {
        //if currently used queue is empty
        this->IncrementRegionGrowingState();
      }
    }
    if (m_RegionGrowingState >= (m_InitializeValue) || m_DetectionStop)
    {
      this->m_IsAtEnd = true;
      // std::cout << "RegionGrowing finished !" << std::endl;
      // std::cout << "Detected point of leakage: " << m_DetectedLeakagePoint << std::endl;
    }

  }

}//end namespace itk

#endif
