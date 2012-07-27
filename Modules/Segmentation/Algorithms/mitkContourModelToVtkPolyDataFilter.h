///*===================================================================
//
//The Medical Imaging Interaction Toolkit (MITK)
//
//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.
//
//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.
//
//See LICENSE.txt or http://www.mitk.org for details.
//
//===================================================================*/
//
//#ifndef _MITK_CONTOURMODEL_TO_POLYDATA_FILTER_H_
//#define _MITK_CONTOURMODEL_TO_POLYDATA_FILTER_H_
//
//
//#include "mitkCommon.h"
//#include "SegmentationExports.h"
//
//
//#include "mitkContourModel.h"
//#include "mitkContourModelSource.h"
//
//#include <vtkPolyData.h>
//#include <vtkPolyDataMapper.h>
//
//
//namespace mitk {
//
//  class Segmentation_EXPORT ContourModelToVtkPolyDataFilter : public BaseProcess
//  {
//
//  public:
//    /** Standard class typedefs. */
//    mitkClassMacro( ContourModelToVtkPolyDataFilter,BaseProcess );
//
//    /** Method for creation through the object factory. */
//    itkNewMacro(Self);
//
//    typedef vtkPolyData OutputType;
//
//    typedef mitk::ContourModel InputType;
//
//
//    /**
//    * Allocates a new output object and returns it. Currently the
//    * index idx is not evaluated.
//    * @param idx the index of the output for which an object should be created
//    * @returns the new object
//    */
//    virtual itk::DataObject::Pointer MakeOutput ( unsigned int idx );
//
//    /**
//    * Allows to set the output of the point set source. 
//    * @param output the intended output of the point set source
//    */
//    virtual void SetOutput( OutputType* output );
//
//    virtual void GraftOutput(OutputType *output);
//    virtual void GraftNthOutput(unsigned int idx, OutputType *output);
//
//    /**
//    * Returns the output with index 0 of the point set source
//    * @returns the output
//    */
//    virtual OutputType* GetOutput();
//
//    /**
//    * Returns the n'th output of the point set source
//    * @param idx the index of the wanted output
//    * @returns the output with index idx.
//    */
//    virtual OutputType* GetOutput ( unsigned int idx );
//
//
//    /** Set/Get the image input of this process object.  */
//    virtual void SetInput( const InputType *input);
//    virtual void SetInput( unsigned int idx, const InputType * input);
//    const InputType * GetInput(void);
//    const InputType * GetInput(unsigned int idx);
//
//  protected:
//    ContourModelToVtkPolyDataFilter();
//    ~ContourModelToVtkPolyDataFilter();
//
//    virtual void GenerateData();
//
//  };
//}
//#endif