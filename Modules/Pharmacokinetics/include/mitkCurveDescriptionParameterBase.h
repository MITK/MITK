/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCurveDescriptionParameterBase_h
#define mitkCurveDescriptionParameterBase_h

#include <iostream>

#include <itkArray.h>
#include <itkArray2D.h>
#include <itkObject.h>
#include <mitkModelBase.h>
#include "MitkPharmacokineticsExports.h"

namespace mitk
{
  /** Base class for functor that compute descriptive values for
  a curve (e.g. like Area under the Curve, Time to peek, maximum,...)
  @remark The derived classes must be implemented thread safe because GetCurveDescriptionParameter()
  and GetDescriptionParameterName() of one instance may be called in
  multi-threaded context (e.g. DescriptionParameterImageGeneratorBase
  and derived classes). */
class MITKPHARMACOKINETICS_EXPORT CurveDescriptionParameterBase : public itk::Object
  {
  public:
    typedef CurveDescriptionParameterBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(CurveDescriptionParameterBase, itk::Object);

    typedef itk::Array<double> CurveType;
    typedef itk::Array<double> CurveGridType;

    typedef double CurveDescriptionParameterResultType;
    typedef std::string CurveDescriptionParameterNameType;

    typedef std::vector<CurveDescriptionParameterResultType> DescriptionParameterResultsType;
    typedef std::vector<CurveDescriptionParameterNameType> DescriptionParameterNamesType;

    /** Returns the concrete description values for a curve.
     * @pre Curve value vector and curve grid must have the same size*/
    DescriptionParameterResultsType GetCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const;

    /**Return the names of all descrition values that will be computed by the class.
     * @post The order of names equales the order of the results of GetCurveDescriptionParameter().*/
    virtual DescriptionParameterNamesType GetDescriptionParameterName() const = 0 ;

  protected:

    /** Slot to implement the computation of the descriptor values.*/
    virtual DescriptionParameterResultsType ComputeCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const = 0;

    CurveDescriptionParameterBase();
    ~CurveDescriptionParameterBase() override;

  private:

    //No copy constructor allowed
    CurveDescriptionParameterBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented

   };
}

#endif
