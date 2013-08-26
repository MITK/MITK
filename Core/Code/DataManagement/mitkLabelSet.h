/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef _mitkLabelSet_H_
#define _mitkLabelSet_H_

#include <MitkExports.h>
#include <mitkCommon.h>
#include <mitkMessage.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkLabel.h>

namespace mitk
{
//
//Documentation
// @brief LabelSet containing the labels corresponding to a segmentation session.
// @ingroup Data
//
class MITK_CORE_EXPORT LabelSet : public itk::Object
{
public:

    mitkClassMacro( LabelSet, itk::Object );

    itkNewMacro( Self );

    typedef std::vector <mitk::Label::Pointer>    LabelContainerType;
    typedef LabelContainerType::const_iterator    LabelContainerConstIteratorType;
    typedef LabelContainerType::iterator          LabelContainerIteratorType;

    /** \brief Returns a const iterator poiting to the begining of the container.
    */
    LabelContainerConstIteratorType IteratorBegin();

    /** \brief Returns a const iterator pointing to the end of the container.
    */
    LabelContainerConstIteratorType IteratorEnd();

    /** \brief
    */
    itkSetStringMacro(LastModified);

    /** \brief
    */
    itkGetStringMacro(LastModified);

    /** \brief
    */
    itkSetStringMacro(Owner);

    /** \brief
    */
    itkGetStringMacro(Owner);

    /** \brief
    */
    itkGetStringMacro(Name);

    /** \brief
    */
    itkSetStringMacro(Name);

    /** \brief
    */
    bool HasLabel(int) const;

    /** \brief
    */
    void SetActiveLabel(int);

    /** \brief
    */
    void RemoveLabel(int);

    /** \brief
    */
    void RemoveSelectedLabels();

    /** \brief
    */
    void RemoveLabels(int begin, int count);

    /** \brief
    */
    void AddLabel(const mitk::Label& label);

    /** \brief
    */
    void AddLabel(const std::string& name, const mitk::Color& color);

    /** \brief
    */
    void RenameLabel(int, const std::string&, const mitk::Color&);

    /** \brief
    */
    virtual int GetNumberOfLabels() const;

    /** \brief
    */
    virtual void SetAllLabelsVisible(bool);

    /** \brief
    */
    virtual void SetAllLabelsLocked(bool);

    /** \brief
    */
    virtual void RemoveAllLabels();

    /** \brief
    */
    virtual void SetLabelVisible(int index, bool value);

    /** \brief
    */
    virtual bool GetLabelVisible(int index);

    /** \brief
    */
    virtual void SetLabelLocked(int index, bool value);

    /** \brief
    */
    virtual bool GetLabelLocked(int index);

    /** \brief
    */
    virtual void SetLabelSelected(int index, bool value);

    /** \brief
    */
    virtual bool GetLabelSelected(int index);

    /** \brief
    */
    virtual void SetLabelOpacity(int index, float value);

    /** \brief
    */
    virtual float GetLabelOpacity(int index);

    /** \brief
    */
    virtual void SetLabelVolume(int index, float value);

    /** \brief
    */
    virtual float GetLabelVolume(int index);

    /** \brief
    */
    virtual void SetLabelName(int index, const std::string &name);

    /** \brief
    */
    virtual std::string GetLabelName(int index);

    /** \brief
    */
    virtual void SetLabelColor(int index, const mitk::Color &color);

    /** \brief
    */
    virtual const mitk::Color& GetLabelColor(int index);

    /** \brief
    */
    virtual const mitk::Label* GetActiveLabel() const { return m_ActiveLabel; };

    /** \brief
    */
    virtual mitk::Label::ConstPointer GetLabel(int index) const;

    /** \brief
    */
    virtual int GetActiveLabelIndex() const;

    /** \brief
    */
    virtual void SetLabelCenterOfMassIndex(int index, const mitk::Point3D& center);

    /** \brief
    */
    virtual const mitk::Point3D& GetLabelCenterOfMassIndex(int index);

    /** \brief
    */
    virtual void SetLabelCenterOfMassCoordinates(int index, const mitk::Point3D& center);

    /** \brief
    */
    virtual const mitk::Point3D& GetLabelCenterOfMassCoordinates(int index);

    /** \brief
    */
    virtual void ResetLabels();

    void Initialize(const LabelSet* other);

    /** \brief
    */
    static bool IsSelected(mitk::Label::Pointer label);

protected:

    LabelSet();
    virtual ~LabelSet();

    LabelSet(const LabelSet& other);

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    LabelContainerType m_LabelContainer;

    mitk::Label* m_ActiveLabel;

    std::string m_Owner;

    std::string m_LastModified;

    std::string m_Name;

private:

  // purposely not implemented
  LabelSet& operator=(const LabelSet&);

  itk::LightObject::Pointer InternalClone() const;

};

} // namespace mitk

#endif // _mitkLabelSet_H_
