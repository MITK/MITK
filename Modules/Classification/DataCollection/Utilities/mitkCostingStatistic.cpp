#include <mitkCostingStatistic.h>
#include <mitkDataCollection.h>

#include <mitkCollectionStatistic.h>
#include <sstream>

// DataCollection Stuff
#include <mitkDataCollectionImageIterator.h>
//stl stuff
#include <sstream>

static void EnsureDataImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target)
{
  typedef itk::Image<double, 3> FeatureImage;
  typedef itk::Image<unsigned char, 3> LabelImage;

  if (collection->HasElement(origin))
  {
    LabelImage::Pointer originImage = dynamic_cast<LabelImage*>(collection->GetData(origin).GetPointer());
    MITK_INFO << "Creating new Element";
    if (!collection->HasElement(target) && originImage.IsNotNull())
    {
      MITK_INFO << "New image necessary";
      FeatureImage::Pointer image = FeatureImage::New();
      image->SetRegions(originImage->GetLargestPossibleRegion());
      image->SetSpacing(originImage->GetSpacing());
      image->SetOrigin(originImage->GetOrigin());
      image->SetDirection(originImage->GetDirection());
      image->Allocate();

      collection->AddData(dynamic_cast<itk::DataObject*>(image.GetPointer()),target,"");
    }
  }
  for (std::size_t i = 0; i < collection->Size();++i)
  {
    mitk::DataCollection* newCol = dynamic_cast<mitk::DataCollection*>(collection->GetData(i).GetPointer());
    if (newCol != 0)
    {
      EnsureDataImageInCollection(newCol, origin, target);
    }
  }
}

static void EnsureLabelImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target)
{
  typedef itk::Image<unsigned char, 3> FeatureImage;
  typedef itk::Image<unsigned char, 3> LabelImage;

  if (collection->HasElement(origin))
  {
    LabelImage::Pointer originImage = dynamic_cast<LabelImage*>(collection->GetData(origin).GetPointer());
    MITK_INFO << "Creating new Element";
    if (!collection->HasElement(target) && originImage.IsNotNull())
    {
      MITK_INFO << "New image necessary";
      FeatureImage::Pointer image = FeatureImage::New();
      image->SetRegions(originImage->GetLargestPossibleRegion());
      image->SetSpacing(originImage->GetSpacing());
      image->SetOrigin(originImage->GetOrigin());
      image->SetDirection(originImage->GetDirection());
      image->Allocate();

      collection->AddData(dynamic_cast<itk::DataObject*>(image.GetPointer()),target,"");
    }
  }
  for (std::size_t i = 0; i < collection->Size();++i)
  {
    mitk::DataCollection* newCol = dynamic_cast<mitk::DataCollection*>(collection->GetData(i).GetPointer());
    if (newCol != 0)
    {
      EnsureLabelImageInCollection(newCol, origin, target);
    }
  }
}

void mitk::CostingStatistic::SetCollection(mitk::DataCollection::Pointer collection)
{
  m_Collection = collection;
}

mitk::DataCollection::Pointer mitk::CostingStatistic::GetCollection()
{
  return m_Collection;
}

bool mitk::CostingStatistic::UpdateCollection()
{
  EnsureDataImageInCollection(m_Collection, m_MaskName, m_CombinedProbabilityA);
  EnsureDataImageInCollection(m_Collection, m_MaskName, m_CombinedProbabilityB);
  EnsureLabelImageInCollection(m_Collection, m_MaskName, m_CombinedLabelName);

  std::vector<DataCollectionImageIterator<double, 3> > iterProbA;
  std::vector<DataCollectionImageIterator<double, 3> > iterProbB;
  for (int i = 0; i < m_ProbabilityClassA.size(); ++i)
  {
    DataCollectionImageIterator<double, 3> iter(m_Collection, m_ProbabilityClassA[i]);
    iterProbA.push_back(iter);
  }
  for (int i = 0; i < m_ProbabilityClassB.size(); ++i)
  {
    DataCollectionImageIterator<double, 3> iter(m_Collection, m_ProbabilityClassB[i]);
    iterProbB.push_back(iter);
  }
  DataCollectionImageIterator<double, 3> iterCombineA(m_Collection, m_CombinedProbabilityA);
  DataCollectionImageIterator<double, 3> iterCombineB(m_Collection, m_CombinedProbabilityB);
  DataCollectionImageIterator<unsigned char, 3> iterMask(m_Collection, m_MaskName);

  while (!iterMask.IsAtEnd())
  {
    if (iterMask.GetVoxel() > 0)
    {
      double probA = 0;
      double probB = 0;
      for (int i = 0; i < iterProbA.size(); ++i)
      {
        probA += iterProbA[i].GetVoxel();
      }
      for (int i = 0; i < iterProbB.size(); ++i)
      {
        probB += iterProbB[i].GetVoxel();
      }
      iterCombineA.SetVoxel(probA * 100);
      iterCombineB.SetVoxel(probB * 100);
    }
    else
    {
      iterCombineA.SetVoxel(0.0);
      iterCombineB.SetVoxel(0.0);
    }

    ++iterCombineA;
    ++iterCombineB;
    ++iterMask;
    for (int i = 0; i < iterProbA.size(); ++i)
    {
      ++(iterProbA[i]);
    }
    for (int i = 0; i < iterProbB.size(); ++i)
    {
      ++(iterProbB[i]);
    }
  }
  return 0;
}

bool mitk::CostingStatistic::CalculateClass(double threshold)
{
  DataCollectionImageIterator<unsigned char, 3> iterMask(m_Collection, m_MaskName);
  DataCollectionImageIterator<unsigned char, 3> iterLabel(m_Collection, m_CombinedLabelName);
  DataCollectionImageIterator<double, 3> iterCombineA(m_Collection, m_CombinedProbabilityA);
  DataCollectionImageIterator<double, 3> iterCombineB(m_Collection, m_CombinedProbabilityB);

  while (!iterMask.IsAtEnd())
  {
    if (iterMask.GetVoxel() > 0)
    {
      double probA = iterCombineA.GetVoxel() / (iterCombineA.GetVoxel() + iterCombineB.GetVoxel());
      probA *= 100;
      iterLabel.SetVoxel(probA >= threshold ? 1 : 2);
    }
    else
    {
      iterLabel.SetVoxel(0);
    }

    ++iterMask;
    ++iterLabel;
    ++iterCombineA;
    ++iterCombineB;
  }

  return true;
}

bool mitk::CostingStatistic::WriteStatistic(std::ostream &out,std::ostream &sout, double stepSize, std::string shortLabel)
{
  UpdateCollection();
  for (double threshold = 0 ; threshold <= 100; threshold  += stepSize)
  {
    CalculateClass(threshold);

    std::stringstream ss;
    ss << shortLabel << ";" << threshold;

    mitk::CollectionStatistic stat;
    stat.SetCollection(m_Collection);
    stat.SetClassCount(2);
    stat.SetGoldName("GTV");
    stat.SetTestName(m_CombinedLabelName);
    stat.SetMaskName(m_MaskName);
    stat.Update();

    stat.Print(out, sout,false, ss.str());
  }
  return true;
}