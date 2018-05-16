#ifndef mitkAdditionalRFData_h
#define mitkAdditionalRFData_h

#include <vigra/multi_array.hxx>


namespace mitk
{
  class AdditionalRFDataAbstract
  {
  public:
    // This function is necessary to be able to do dynamic casts
    virtual void NoFunction() = 0;
    virtual ~AdditionalRFDataAbstract() {};
  };

  class NoRFData : public AdditionalRFDataAbstract
  {
  public:
    virtual void NoFunction() { return; }
    virtual ~NoRFData() {};
  };

  class PURFData : public AdditionalRFDataAbstract
  {
  public:
    vigra::ArrayVector<double> m_Kappa;
    virtual void NoFunction();
    virtual ~PURFData() {};
  };
}

#endif //mitkAdditionalRFData_h
