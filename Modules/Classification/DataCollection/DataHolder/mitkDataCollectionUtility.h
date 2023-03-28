/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkDataCollectionUtility_h
#define mitkDataCollectionUtility_h


#include <mitkDataCollection.h>

/**
  \brief Follow Up Storage - Class to facilitate loading/accessing structured follow-up data

  Data is into a collection that may contain further (sub) collections or images.
*/

namespace mitk
{
  class DataCollectionUtility
  {
  public:
    template <class OriginImageType, class TargetImageType>
    static void EnsureImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target);

    static void Compile()
    {
      mitk::DataCollection::Pointer col = mitk::DataCollection::New();
      std::string blub = "haha";
      typedef itk::Image<double, 3> ImType;
      EnsureImageInCollection<ImType, ImType>(col, blub, blub);
  };
} // end namespace

template <class OriginImageType, class TargetImageType>
void
mitk::DataCollectionUtility::EnsureDataImageInCollection(mitk::DataCollection::Pointer collection, std::string origin, std::string target)
{
    typedef typename TargetImageType FeatureImage;
    typedef typename OriginImageType LabelImage;

    if (collection->HasElement(origin))
    {
        LabelImage::Pointer originImage = dynamic_cast<LabelImage*>(collection->GetData(origin).GetPointer());
        if (!collection->HasElement(target) && originImage.IsNotNull())
        {
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
};

#endif
