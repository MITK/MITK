package debug;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.Connection;
import model.StartState;
import model.StateMachinesDiagram;
import model.States;

import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.ListViewer;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.ViewPart;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

public class DebugEventsList extends ViewPart implements IDoubleClickListener {

	private static DebugEventsList view;
	private static ListViewer viewer = null;
	private static Map transitionToEvent = new HashMap();
	private static Map countEvents = new HashMap();
	private static States activeState = null;
	private static Connection activeCon = null;
	private static Connection doubleClickedCon = null;
	private static StateMachinesDiagram diagram1 = null;
	private static List allStates = null;
	private static String instanceAddress = null;
	private static int counter = 1;
	
	public DebugEventsList() {
		super();
		counter = 1;
		view = this;
	}
	
	public static void addEventToList(String eventId) {
		ReadActionAndEventDOMTree eventNames = DOMGetInstance.getActionAndEventInstance();
		String event = eventNames.getEventName(eventId);
		String countEvent = Integer.toString(counter) + " " + event;
		counter++;
		viewer.add(countEvent);
		countEvents.put(countEvent, event);
		if (!(activeState == null)) {
			List allConnections = activeState.getSourceConnections();
			for (int i = 0; i < allConnections.size(); i++) {
				Connection connection =  (Connection) allConnections.get(i);
				if (event.equals(connection.getEvent())) {
					if (!(doubleClickedCon == null)) {
						doubleClickedCon.setInactive();
						doubleClickedCon.getTarget().setInactive();
					}
					if (!(activeCon == null)) {
						activeCon.setInactive();
					}
					activeCon = connection;
					activeCon.setActive();
					transitionToEvent.put(countEvent, connection);
					if (!(activeState == null)) {
						activeState.setInactive();
					}
					activeState = activeCon.getTarget();
					activeCon.getTarget().setActive();
					break;
				}
			}
		}
	}
	
	/**
	 * hides this view
	 */
	public static void closeView() {
		IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage();
		page1.hideView(view);
	}
	
	public void createPartControl(Composite parent) {
		viewer = new ListViewer(parent);
		viewer.addDoubleClickListener(this);
	}

	public void setFocus() {
		viewer.getControl().setFocus();
	}

	public void doubleClick(DoubleClickEvent event) {
		String eventString = (String) ((IStructuredSelection)viewer.getSelection()).getFirstElement();
		if (!(doubleClickedCon == null)) {
			doubleClickedCon.setInactive();
			//doubleClickedCon.getTarget().setInactive();
		}
		if (!(activeCon == null)) {
			activeCon.setInactive();
		}
		if(!(transitionToEvent.get(eventString) == null)) {
			if (!(doubleClickedCon == null)) {
				doubleClickedCon.getTarget().setInactive();
			}
			doubleClickedCon = (Connection) transitionToEvent.get(eventString);
			doubleClickedCon.setActive();
			if (!(activeState == null)) {
				activeState.setInactive();
			}
			doubleClickedCon.getTarget().setActive();
		}
	}
	
	public static void clear() {
		if (!(diagram1 == null)) {
			diagram1.resetFromDebugMode();
		}
		viewer.getList().removeAll();
	}

	public static void setDiagram(StateMachinesDiagram diagram) {
		diagram1 = diagram;
		allStates = diagram1.getChildren();
		for (int i = 0; i < allStates.size(); i++) {
			if ((States)allStates.get(i) instanceof StartState) {
				activeState = (States)allStates.get(i);
				activeState.setActive();
				break;
			}
		}
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchPart#dispose()
	 */
	public void dispose() {
		diagram1.resetFromDebugMode();
		PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
		closeView();
	}
	
	public static void setInstanceAddress(String instanceAddress1) {
		instanceAddress = instanceAddress1;
	}
	
	public static String getInstanceAddress() {
		return instanceAddress;
	}
}
