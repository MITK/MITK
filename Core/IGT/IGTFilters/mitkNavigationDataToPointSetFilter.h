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

  /**Documentation
  *
  * \brief This filter converts navigation datas to a 3D pointset. Or several navigation datas over time into a 4D pointset.
  *
  * The mode can be set with the SetOperationMode(OperationMode mode) method. Default is Mode3D.
  *
  * \ingroup IGT
  * 
  */
  class NavigationDataToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(NavigationDataToPointSetFilter, PointSetSource);
    itkNewMacro(Self);


    /**Documentation
    * /brief Sets the size for the ring buffer in Mode4D.
    * The size determines the maximum timesteps of the output PointSet
    * 
    */
    itkSetMacro(RingBufferSize, unsigned int)

    /**Documentation
    * \brief filter execute method
    *
    */
    virtual void GenerateData();

    /**Documentation
    * \brief Sets one input NavigationData
    */
    virtual void SetInput(const mitk::NavigationData *NavigationData);
   
    /**Documentation
    * \brief Sets one input NavigationData at a specific index
    */
    virtual void SetInput(const mitk::NavigationData *input, unsigned int index);

    /**Documentation
    * \brief There are two different operation modes. 
    *
    * Mode3D:   every input is stored into one entry of the output PointSet
    * Mode4D:   the same as Mode3D but the PointSet has time with a buffer size set with SetBufferSize
    *
    */
    enum OperationMode
    {
      Mode3D,
      Mode4D
    };

    /**Documentation
    * \brief Sets the mode of this filter.
    * the output can be a pointset with the points of the current navigation data inputs (Mode3D),
    * or everytime when the update method is called the current navigation data inputs are attached to the output pointset (Mode4D)
    */
    virtual void SetOperationMode(OperationMode mode);

  protected:
    NavigationDataToPointSetFilter();

    virtual ~NavigationDataToPointSetFilter();

    /**Documentation
    * \brief Generates the output for Mode3D
    *
    */
    virtual void GenerateDataMode3D();

    /**Documentation
    * \brief Generates the output for Mode4D
    *
    */
    virtual void GenerateDataMode4D();

  protected: 
    
    OperationMode m_OperationMode; ///< Stores the mode. See enum OperationMode

    unsigned int m_RingBufferSize; ///< Stores the ringbuffer size

    unsigned int m_CurrentTimeStep; ///< Indicates the current timestamp

  };

} // namespace mitk

#endif // _mitkNavigationDataToPointSetFilter_h__
