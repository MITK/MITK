#ifndef MITK_KALMAN_FILTER_H
#define MITK_KALMAN_FILTER_H

// mitk
#include <mitkCommon.h>

// itk
#include <itkObject.h>
#include "itkObjectFactory.h"

#include <cv.h>

namespace mitk
{
/**
* 
* DOES NOT YET WORK!
*
*/
  class KalmanFilter : public itk::Object
  {

  public:
  
    // only needed for smartpointer
    mitkClassMacro(KalmanFilter,itk::Object);
    itkNewMacro(Self);

    KalmanFilter();
    virtual ~KalmanFilter();

    float ProcessValue( float input );
    void SetMeasurementNoise( float measurementNoise );
    void SetProcessNoise( float processNoise );
    void ResetFilter();

  protected:

  private:

    CvKalman* m_cvKalman;
    CvMat* m_StateVector; // x_k

    bool m_IsInitialized;

  }; // end class
} // end namespace
#endif // MITK_KALMAN_FILTER_H
