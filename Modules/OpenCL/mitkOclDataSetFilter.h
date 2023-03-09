/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOclDataSetFilter_h
#define mitkOclDataSetFilter_h

#include "mitkOclFilter.h"
#include "mitkOclDataSet.h"

#define FILTER_UCHAR 0
#define FILTER_SHORT 1

namespace mitk
{
class OclFilter;
class OclDataSetFilter;

   /**
    * \brief The OclDataSetFilter is the topmost class for all filter which take DataSets as input.
    *
    * The input DataSet can be intialized via an oclDataSet or a pointer to the data
    * This makes it possible to create a filter pipeline of GPU-based filters
    * and to bind this part into the CPU (ITK) filter pipeline.
    */
class MITKOPENCL_EXPORT OclDataSetFilter: public OclFilter
{
public:
  /**
   * @brief SetInput SetInput Set the input DataSet (as mitk::OclDataSet).
   * @param DataSet The DataSet in mitk::OclDataSet.
   */
  void SetInput(mitk::OclDataSet::Pointer DataSet);

  /**
   * @brief SetInput Set the input DataSet (as a pointer to the data).
   * @param DataSet The DataSet in mitk::OclDataSet.
   * @param size
   * @param BpE
   */
  void SetInput(void* DataSet, unsigned int size, unsigned int BpE);

  /**
  * @brief SetInput Set the input DataSet (as mitk::Image).
  * @param image The DataSet in mitk::OclDataSet.
  */
  void SetInput(mitk::Image::Pointer image);

protected:
  OclDataSetFilter();

  virtual ~OclDataSetFilter();

  /** The input DataSet */
  mitk::OclDataSet::Pointer m_Input;
  unsigned int m_CurrentSize;
};
}
#endif
