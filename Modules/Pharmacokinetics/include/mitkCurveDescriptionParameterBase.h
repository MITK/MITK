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
#include <MitkPharmacokineticsExports.h>

namespace mitk
{
  /** \class CurveDescriptionParameterBase
   * \brief Abstract base class for functors that compute descriptive scalar values for a time-concentration curve.
   *
   * Derived classes implement specific curve descriptors such as Area Under the Curve (AUC),
   * Time to Peak, Maximum, Mean Residence Time, etc. Each descriptor can produce one or more
   * named result values.
   *
   * \note Derived classes must be thread-safe because GetCurveDescriptionParameter() and
   * GetDescriptionParameterName() may be called concurrently from multi-threaded generators
   * (e.g. PixelBasedDescriptionParameterImageGenerator).
   *
   * \sa AreaUnderTheCurveDescriptionParameter, AreaUnderFirstMomentDescriptionParameter,
   *     MaximumCurveDescriptionParameter, TimeToPeakCurveDescriptionParameter,
   *     MeanResidenceTimeDescriptionParameter, CurveParameterFunctor
   */
class MITKPHARMACOKINETICS_EXPORT CurveDescriptionParameterBase : public itk::Object
  {
  public:
    typedef CurveDescriptionParameterBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(CurveDescriptionParameterBase, itk::Object);

    /** \brief Array type for curve values. */
    typedef itk::Array<double> CurveType;
    /** \brief Array type for the curve time grid. */
    typedef itk::Array<double> CurveGridType;

    /** \brief Scalar result type for a single description parameter value. */
    typedef double CurveDescriptionParameterResultType;
    /** \brief String type for description parameter names. */
    typedef std::string CurveDescriptionParameterNameType;

    /** \brief Vector of computed description parameter results. */
    typedef std::vector<CurveDescriptionParameterResultType> DescriptionParameterResultsType;
    /** \brief Vector of description parameter names. */
    typedef std::vector<CurveDescriptionParameterNameType> DescriptionParameterNamesType;

    /** \brief Computes and returns the description parameter values for the given curve.
     *  \param[in] curve The curve values (e.g. concentration over time).
     *  \param[in] grid The time grid corresponding to the curve values.
     *  \pre \p curve and \p grid must have the same size.
     *  \return Vector of computed description parameter values. */
    DescriptionParameterResultsType GetCurveDescriptionParameter(const CurveType& curve, const CurveGridType& grid) const;

    /** \brief Returns the names of all description parameters computed by this class.
     *  \post The order of names matches the order of results returned by GetCurveDescriptionParameter().
     *  \return Vector of parameter name strings. */
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
