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

#ifndef _MITKNAVIGATIONDATATOPOINTSETFILTER_H__
#define _MITKNAVIGATIONDATATOPOINTSETFILTER_H__

#include "mitkCommon.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include "mitkNavigationData.h"

namespace mitk {

  /**Documentation
  *
  * \brief This filter creates mitk::PointSet objects from mitk::NavigaitionData objects
  *
  * This filter has two modes that can be set with SetOperationMode(). 
  * - Mode3D:   every input NavigationData is processed into one output pointset.  For each call to Update() a point with the ND position will be added to the PointSet
  * - Mode4D:   one output pointset is generated that contains one point for each input NavigationData. Each call to Update() adds a new timestep to the PointSet that contains new positions for the points.
  *
  * \ingroup IGT
  * 
  */
  class MITK_IGT_EXPORT NavigationDataToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(NavigationDataToPointSetFilter, PointSetSource);
    itkNewMacro(Self);
    
    /**Documentation
    * \brief There are two different operation modes. 
    *
    * - Mode3D:   every input NavigationData is processed into one output pointset that contains a point with the ND position for each Update()
    * - Mode4D:   one output pointset is generated that contains one point for each input NavigationData. Each call to Update() adds a new timestep to the PointSet that contains new positions for the points.
    * The RingBufferSize limits the number of points in the 3D mode and the number of timesteps in the 4D mode.
    */
    enum OperationMode
    {
      Mode3D,
      Mode4D
    };

    /**Documentation
    * \brief Sets the size for the ring buffer.
    *
    * The size determines the maximum number of timesteps in 4D mode and the number of points in 3D mode of the output PointSet
    */
    itkSetMacro(RingBufferSize, unsigned int)

    /**
    * \brief filter execute method
    */
    virtual void GenerateData();

    /**
    * \brief Sets one input NavigationData
    */
    virtual void SetInput(const mitk::NavigationData *NavigationData);
   
    /**
    * \brief Sets the input NavigationData at a specific index
    */
    virtual void SetInput(unsigned int idx, const NavigationData* nd);

    /**
    * \brief Returns the input of this filter
    */
    const mitk::NavigationData* GetInput();

    /**
    * \brief Returns the input number idx of this filter
    */
    const mitk::NavigationData* GetInput(unsigned int idx);

    /**
    * \brief Sets the mode of this filter.
    *
    * See OperationMode for the behaviour in the different modes
    */
    virtual void SetOperationMode(OperationMode mode);

    /**
    * \brief returns the mode of this filter.
    *
    * See OperationMode for the behaviour in the different modes
    */
    itkGetConstMacro(OperationMode, OperationMode);

    //virtual void SetOperationMode(OperationMode mode);

  protected:
    NavigationDataToPointSetFilter();

    virtual ~NavigationDataToPointSetFilter();

    /**
    * \brief Generates the output for Mode3D
    *
    */
    virtual void GenerateDataMode3D();

    /**
    * \brief Generates the output for Mode4D
    */
    virtual void GenerateDataMode4D();

    /**
    * \brief create output objects according to OperationMode for all inputs
    */
    virtual void CreateOutputsForAllInputs();

    OperationMode m_OperationMode;  ///< Stores the mode. See enum OperationMode
    unsigned int m_RingBufferSize;  ///< Stores the ringbuffer size
    unsigned int m_CurrentTimeStep; ///< Indicates the current timestamp
  };
} // namespace mitk
#endif // _MITKNAVIGATIONDATATOPOINTSETFILTER_H__
