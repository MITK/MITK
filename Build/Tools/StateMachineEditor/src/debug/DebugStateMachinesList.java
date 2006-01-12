package debug;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.StateMachinesDiagram;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.ListViewer;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.part.ViewPart;
import org.jdom.Element;

import stateMachines.StateMachinesEditor;
import stateMachinesList.StateMachinesList;

import dom.DOMGetInstance;
import dom.ReadDOMTree;

public class DebugStateMachinesList extends ViewPart implements IDoubleClickListener {
	
	private static DebugStateMachinesList view;
	private static ListViewer viewer = null;
	private static DebugServerThread server1 = null;
	//private static Map instanceToName = new HashMap();
	private static Map nameToInstance = new HashMap();
	private static Map countNameToInstance = new HashMap();
	private static Map instanceToCountName = new HashMap();
	private static int counter = 1;
	
	public DebugStateMachinesList() {
		super();
		view = this;
		counter = 1;
	}
	
	public static void setServer(DebugServerThread server) {
		server1 = server;
	}
	
	/**
	 * hides this view
	 */
	public static void closeView() {
		IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
		page1.hideView(view);
		DebugEventsList.closeView();
	}
	
	public static void addStateMachineToList() {
		List messages = new ArrayList();
		server1.getNewMessages(messages);
		for (int i = 0; i < messages.size(); i++) {
			Message message = (Message) messages.get(i);
			//instanceToName.put(message.getName(), message.getInstanceAddress());
			String countName = Integer.toString(counter) + " " + message.getName();
			counter++;
			countNameToInstance.put(countName, message.getInstanceAddress());
			nameToInstance.put(message.getInstanceAddress(), message.getName());
			viewer.add(countName);
		}
		messages.clear();
	}
	
	
	/*public static void addStateMachineToList2(String machineName) {
		List list = Collections.synchronizedList(new ArrayList());
		list.add(machineName);
		synchronized(list) {
		      Iterator i = list.iterator(); // Must be in synchronized block
		      while (i.hasNext())
		    	  viewer.add(i.next());
		  }
		String machine = machineName;
		System.out.println(machine + "hier");
		System.out.println(list);
		//viewer.add(list.get(0));
	}*/
	
	public void createPartControl(Composite parent) {
		viewer = new ListViewer(parent);
		viewer.addDoubleClickListener(this);
		
		/*javax.swing.Timer t= new javax.swing.Timer(1000,
			      new ActionListener() {
			         public void actionPerformed(ActionEvent e) {
			        	 List messages = new ArrayList();
				    	  if (!(server1 == null)) {
							server1.getNewMessages(messages);
							for (int i = 0; i < messages.size(); i++) {
								Message message = (Message) messages.get(i);
								instanceToName.put(message.getName(), message.getInstanceAddress());
								nameToInstance.put(message.getInstanceAddress(), message.getName());
								System.out.println(message.getName() + "         hier");
								viewer.add(message.getName());
							}
				    	  }
			         }
			      });
			  t.start(); */
		
		/*int delay = 1000; //milliseconds
		  ActionListener taskPerformer = new ActionListener() {
		      public void actionPerformed(ActionEvent evt) {
		    	  List messages = new ArrayList();
		    	  if (!(server1 == null)) {
					server1.getNewMessages(messages);
					for (int i = 0; i < messages.size(); i++) {
						Message message = (Message) messages.get(i);
						instanceToName.put(message.getName(), message.getInstanceAddress());
						nameToInstance.put(message.getInstanceAddress(), message.getName());
						System.out.println(message.getName() + "         hier");
						//viewer.add(message.getName());
					}
		    	  }
		      }
		  };
		  new Timer(delay, taskPerformer).start();*/
	}

	public void setFocus() {
		viewer.getControl().setFocus();
	}

	public void doubleClick(DoubleClickEvent event) {
        // opens the selected statemachine editor in debug mode
		String counterFileName = (String) ((IStructuredSelection)viewer.getSelection()).getFirstElement();
		String  instanceAddress = (String) countNameToInstance.get(counterFileName);
		String fileName = (String) nameToInstance.get(instanceAddress);
		final IFile machineName = DOMGetInstance.getContainer().getFile(new Path(fileName + ".states"));
		try {
			Element machine = null;
			ReadDOMTree tree = DOMGetInstance.getInstance();
			List allMachines = tree.getStateMachines();
			for (int i = 0; i < allMachines.size(); i++) {
				machine = (Element) allMachines.get(i);
				if (machine.getAttributeValue("NAME").equals(fileName)) {
					break;
				}
			}
			if (!(machine == null)) {
				StateMachinesDiagram diagram = new StateMachinesDiagram(machine);
				tree.addDiagram(diagram);
				PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
				IDE.openEditor(PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(), machineName, true);
				diagram.setDebugMode();
				IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
				page1.showView("DebugEventsList");
				DebugEventsList.clear();
				DebugEventsList.setDiagram(diagram);
				DebugEventsList.setInstanceAddress(instanceAddress);
				EventsForStatemachine stateMachineEvents = server1.getEventsForStatemachine(instanceAddress);
				if (!(stateMachineEvents == null)) {
					List allEvents = stateMachineEvents.getEvents();
					for (int i = 0; i < allEvents.size(); i++) {
						DebugEventsList.addEventToList((String) allEvents.get(i));
					}
				}
			}
		} catch (PartInitException e) {
			e.printStackTrace();
		}	
	}

	public static void removeStateMachineFromList(String instanceAddress) {
		ReadDOMTree tree = DOMGetInstance.getInstance();
		String machineName = (String) nameToInstance.get(instanceAddress);
		StateMachinesDiagram diagram = tree.getStateMachinesDiagram(machineName);
		// close the editor
		if (!(diagram == null)) {
			StateMachinesEditor editor1 = diagram.getEditor();
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeEditor(editor1, true);
			tree.removeDiagram(diagram);
		}
		String countMachineName = (String) instanceToCountName.get(instanceAddress);
		viewer.remove(countMachineName);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchPart#dispose()
	 */
	public void dispose() {
		closeView();
		PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
		StateMachinesList.setDebugMode(false);
	}
}
