#ifndef MITKvtkObjectSet_H
#define MITKvtkObjectSet_H

#include <vtkObject.h>
#include <mitkCommon.h>
#include <MitkExtExports.h>
#include <set>

class vtkObjectObserver;

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
class MitkExt_EXPORT vtkObjectSet
{
public:
    vtkObjectSet();
    virtual ~vtkObjectSet();

    void AddObserver( vtkObjectObserver* observer );
    void RemoveObserver( vtkObjectObserver* observer );

    void AddObject ( vtkObject * obj );
    void RemoveObject ( vtkObject * obj );

    ///
    /// vtk event function
    ///
    static void OnObjectModified(vtkObject *, unsigned long eid
                                     , void* clientdata, void *calldata);
protected:
    std::set<vtkObjectObserver*> m_vtkObjectObservers;
    std::set<const vtkObject*> m_Objects;
    ///
    /// \brief Holds all tags of Modified Event Listeners.
    ///
    std::map<const vtkObject *, unsigned long> m_ObjectModifiedTags;
    ///
    /// \brief Holds all tags of Modified Event Listeners.
    ///
    std::map<const vtkObject *, unsigned long> m_ObjectDeleteTags;
};

#endif // MITKvtkObjectSet_H
