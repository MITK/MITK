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
    void NoFunction() override { return; }
    ~NoRFData() override {};
  };

  class PURFData : public AdditionalRFDataAbstract
  {
  public:
    vigra::ArrayVector<double> m_Kappa;
    void NoFunction() override;
    ~PURFData() override {};
  };
}

#endif //mitkAdditionalRFData_h
