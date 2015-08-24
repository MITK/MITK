#ifndef mitkCollectionStatistic_h
#define mitkCollectionStatistic_h

#include <MitkDataCollectionExports.h>

#include <mitkDataCollection.h>

#include <iostream>

namespace mitk {
  struct MITKDATACOLLECTION_EXPORT StatisticData
  {
    unsigned int m_TruePositive;
    unsigned int m_FalsePositive;
    unsigned int m_TrueNegative;
    unsigned int m_FalseNegative;

    double m_DICE;
    double m_Jaccard;
    double m_Sensitivity;
    double m_Specificity;

    StatisticData() :
      m_TruePositive(0), m_FalsePositive(0), m_TrueNegative(0), m_FalseNegative(0),
      m_DICE(0), m_Jaccard(0), m_Sensitivity(0), m_Specificity(0)
    {}
  };

  class MITKDATACOLLECTION_EXPORT CollectionStatistic
  {
  public:
    typedef std::vector<StatisticData> DataVector;
    typedef std::vector<DataVector> MultiDataVector;

    void SetCollection(DataCollection::Pointer collection);
    DataCollection::Pointer GetCollection();

    void SetClassCount (size_t count);
    size_t GetClassCount();

    void SetGoldName(std::string name);
    std::string GetGoldName();

    void SetTestName(std::string name);
    std::string GetTestName();

    void SetMaskName(std::string name) {m_MaskName = name; }

    void Print(std::ostream& out, std::ostream& sout = std::cout, bool withHeader = false, std::string label = "None");
    bool Update();
    int IsInSameVirtualClass(unsigned char gold, unsigned char test);

    unsigned char ToGoldIndex(unsigned char value);
    unsigned char ToTestIndex(unsigned char value);
  private:
    size_t m_ClassCount;
    std::string m_GroundTruthName;
    std::string m_TestName;
    std::string m_MaskName;
    DataCollection::Pointer m_Collection;

    std::vector<unsigned char> m_ConnectionGold;
    std::vector<unsigned char> m_ConnectionTest;
    std::vector<unsigned char> m_ConnectionClass;
    size_t m_VituralClassCount;

    MultiDataVector m_ImageClassStatistic;
    std::vector<std::string> m_ImageNames;
    DataVector m_ImageStatistic;
    StatisticData m_MeanCompleteStatistic;
    StatisticData m_CompleteStatistic;
  };
}

#endif // mitkCollectionStatistic_h
