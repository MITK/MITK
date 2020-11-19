/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitk_TubeGraphProperty_h
#define _mitk_TubeGraphProperty_h

#include <MitkTubeGraphExports.h>

#include "mitkTubeGraph.h"
#include <mitkBaseProperty.h>
#include <mitkColorProperty.h>

#include <itkObject.h>
#include <map>
#include <vector>

namespace mitk
{
  /**
  * \brief Property for tube graphs
  */
  class MITKTUBEGRAPH_EXPORT TubeGraphProperty : public BaseProperty
  {
  public:
    mitkClassMacro(TubeGraphProperty, BaseProperty);
    itkNewMacro(TubeGraphProperty);

    struct LabelGroup
    {
      struct Label
      {
        std::string labelName;
        bool isVisible;
        Color labelColor;
      };
      std::string labelGroupName;
      std::vector<Label *> labels;
    };

    ///////////////TYPEDEF///////////////
    typedef TubeGraph::TubeDescriptorType TubeDescriptorType;
    typedef std::vector<TubeGraphProperty::LabelGroup *> LabelGroupSetType;
    typedef std::pair<TubeDescriptorType, std::string> TubeToLabelGroupType;
    /////////////////////////////////////

    struct Annotation
    {
      std::string name;
      std::string description;
      TubeDescriptorType tube;
    };

    /**
    * Checks, if a given tube is visible
    * @param tube the tube id of the tube to check
    * @returns true, if the tube with id is visible or false otherwise
    */
    bool IsTubeVisible(const TubeDescriptorType &tube);

    /**
    * Sets a tube active.
    * @param tube the tube id of the tube, which has to be set active
    * @param active true, if the tube should be active or false if not.
    */
    void SetTubeActive(const TubeDescriptorType &tube, const bool &active);

    /**
      * Sets tubes active.
      *
      */
    void SetTubesActive(std::vector<TubeDescriptorType> &tubes);

    /**
    * Checks, if a given tube is activated
    * @param tube the to check
    * @returns true, if the tube with id is active or false otherwise
    */
    bool IsTubeActive(const TubeDescriptorType &tube);

    std::vector<TubeDescriptorType> GetActiveTubes();

    Color GetColorOfTube(const TubeDescriptorType &tube);

    void SetTubesToLabels(std::map<TubeToLabelGroupType, std::string> tubeToLabelMap);

    std::map<TubeToLabelGroupType, std::string> GetTubesToLabels();

    /**
    * Deactivates all tubes
    */
    void DeactivateAllTubes();

    void AddAnnotation(Annotation *annotation);
    Annotation *GetAnnotationByName(std::string annotation);
    std::vector<Annotation *> GetAnnotations();
    void RemoveAnnotation(Annotation *annotation);

    void AddLabelGroup(LabelGroup *labelGroup, unsigned int position); // Add LG by name and vec<labelName>??
    void RemoveLabelGroup(LabelGroup *labelGroup);
    LabelGroupSetType GetLabelGroups();
    unsigned int GetNumberOfLabelGroups();
    unsigned int GetIndexOfLabelGroup(LabelGroup *labelGroup);
    LabelGroup *GetLabelGroupByName(std::string labelGroup);

    void SetLabelVisibility(LabelGroup::Label *label, bool isVisible);
    void SetLabelColor(LabelGroup::Label *label, Color color);
    void RenameLabel(LabelGroup *labelGroup, LabelGroup::Label *label, std::string newName);
    void SetLabelForActivatedTubes(LabelGroup *labelGroup, LabelGroup::Label *label);

    LabelGroup::Label *GetLabelByName(LabelGroup *labelGroup, std::string labelName);

    std::string GetValueAsString() const override;

  protected:
    TubeGraphProperty();
    TubeGraphProperty(const TubeGraphProperty &other);
    ~TubeGraphProperty() override;

  private:
    std::vector<TubeDescriptorType> m_ActiveTubes;
    LabelGroupSetType m_LabelGroups;
    std::map<TubeToLabelGroupType, std::string> m_TubeToLabelsMap;
    std::vector<Annotation *> m_Annotations;

    bool TubeDescriptorsCompare(const TubeDescriptorType &tube1, const TubeDescriptorType &tube2);

    // purposely not implemented
    TubeGraphProperty &operator=(const TubeGraphProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
    itk::LightObject::Pointer InternalClone() const override;
  };

} // namespace mitk

#endif /* _mitk_TubeGraphProperty_h */
