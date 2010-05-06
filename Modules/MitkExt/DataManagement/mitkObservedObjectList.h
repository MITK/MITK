#ifndef MITKObservedObjectList_H
#define MITKObservedObjectList_H

#include <vtkObject.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include <MitkExtExports.h>
#include <set>

namespace mitk {

class ObjectObserver;

///
/// a class holding
/// a list of itk objects and/or vtk objects.
/// the main idea of the class
/// is that it listens to the modified
/// and delete events of the objects
/// and informs associated objectobservers.
/// This eases the handling of the sometimes complicated
/// event handling mechanisms.
///
class MitkExt_EXPORT ObservedObjectList : public itk::Object
{
public:
    mitkClassMacro(ObservedObjectList, itk::Object);
    itkFactorylessNewMacro(Self);

    virtual ~ObservedObjectList();

    void AddObserver( ObjectObserver* observer );
    void RemoveObserver( ObjectObserver* observer );

    void AddObject ( itk::Object * obj );
    void RemoveObject ( itk::Object * obj );
//    const itk::Object * GetItkObject (unsigned int i) const;
//    unsigned int GetSizeOfItkObjects () const;

    void AddObject ( vtkObject * obj );
    void RemoveObject ( vtkObject * obj );
//    const vtkObject * GetVtkObject (unsigned int i) const;
//    unsigned int GetSizeOfVtkObjects () const;

    ///
    /// itk event function
    ///
    void OnItkObjectModified(const itk::Object *caller
                                            , const itk::EventObject &event);
    ///
    /// vtk event function
    ///
    static void OnVtkObjectModified(vtkObject *, unsigned long eid
                                     , void* clientdata, void *calldata);
protected:
    ObservedObjectList();
protected:
    std::set<ObjectObserver*> m_ObjectObservers;
    std::set<const itk::Object *> m_ItkObjects;
    std::set<const vtkObject *> m_VtkObjects;
    ///
    /// \brief Holds all tags of Modified Event Listeners.
    ///
    std::map<const itk::Object *, unsigned long> m_ItkObjectModifiedTags;
    ///
    /// \brief Holds all tags of Modified Event Listeners.
    ///
    std::map<const itk::Object *, unsigned long> m_ItkObjectDeleteTags;
    ///
    /// \brief Holds all tags of Modified Event Listeners.
    ///
    std::map<const vtkObject *, unsigned long> m_VtkObjectModifiedTags;
    ///
    /// \brief Holds all tags of Modified Event Listeners.
    ///
    std::map<const vtkObject *, unsigned long> m_VtkObjectDeleteTags;
};

} // namespace mitk

#endif // MITKObservedObjectList_H
