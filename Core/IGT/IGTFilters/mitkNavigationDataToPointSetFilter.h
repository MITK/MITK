/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-08-25 18:10:57 +0200 (Mo, 25 Aug 2008) $
Version:   $Revision: 15062 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mitkNavigationDataToPointSetFilter_h__
#define _mitkNavigationDataToPointSetFilter_h__

#include "mitkCommon.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include "mitkNavigationData.h"

namespace mitk {

  /**
  *
  * @brief Converts a navigation data to a 3D pointset. Or several navigation data into a 4D pointset.
  *
  * 
  *
  * @ingroup Navigation
  * 
  */
  class NavigationDataToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(NavigationDataToPointSetFilter, PointSetSource);
    itkNewMacro(Self);


    /**
    * @brief sets the size for the ring buffer in Mode4D
    * this size affects the maximum timesteps of the output pointset
    * 
    */
    itkSetMacro(RingBufferSize, unsigned int)

    virtual void GenerateOutputInformation();

    virtual void GenerateData();

    const mitk::NavigationData* GetInput(void);

    virtual void SetInput(const mitk::NavigationData *NavigationData);
   
    virtual void SetInput(const mitk::NavigationData *input, unsigned int index);

    enum OperationMode
    {
      Mode3D,
      Mode4D
    };

    /**
    * @brief sets the mode of this filter 
    * the output can be a pointset with the points of the current navigation data inputs (Mode3D),
    * or everytime when the update method is called the current navigation data inputs are attached to the output pointset (Mode4D)
    */
    virtual void SetOperationMode(OperationMode mode);

  protected:
    NavigationDataToPointSetFilter();

    virtual ~NavigationDataToPointSetFilter();

    virtual void GenerateDataMode3D();

    virtual void GenerateDataMode4D();

  protected: 
    
    OperationMode m_OperationMode;

    unsigned int m_RingBufferSize;

    int m_CurrentTimeStep;

  };

} // namespace mitk

#endif // _mitkNavigationDataToPointSetFilter_h__
