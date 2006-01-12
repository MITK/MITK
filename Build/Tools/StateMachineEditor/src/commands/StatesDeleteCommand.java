/*
 * Created on 05.07.2005
 */
package commands;

import java.util.Iterator;
import java.util.List;

import org.eclipse.gef.commands.Command;

import model.Connection;
import model.States;
import model.StateMachinesDiagram;

/**
 * @author Daniel
 */
public class StatesDeleteCommand extends Command {
	/** State to remove. */
	private final States child;

	/** StateMachinesDiagram to remove from. */
	private final StateMachinesDiagram parent;

	/** Holds a copy of the outgoing connections of child. */
	private List sourceConnections;

	/** Holds a copy of the incoming connections of child. */
	private List targetConnections;

	/** True, if child was removed from its parent. */
	private boolean wasRemoved;

	/**
	 * Create a command that will remove the state from its parent.
	 * 
	 * @param parent
	 *            the StateMachinesDiagram containing the child
	 * @param child
	 *            the State to remove
	 * @throws IllegalArgumentException
	 *             if any parameter is null
	 */
	public StatesDeleteCommand(StateMachinesDiagram parent, States child) {
		if (parent == null || child == null) {
			throw new IllegalArgumentException();
		}
		setLabel("state deletion");
		this.parent = parent;
		this.child = child;
	}

	/**
	 * Reconnects a List of Connections with their previous endpoints.
	 * 
	 * @param connections
	 *            a non-null List of connections
	 */
	private void addConnections(List connections) {
		for (Iterator iter = connections.iterator(); iter.hasNext();) {
			Connection conn = (Connection) iter.next();
			conn.reconnect();
		}
	}
	
	/**
	 * Reconnects a List of Connections with their previous endpoints.
	 * 
	 * @param connections
	 *            a non-null List of connections
	 */
	private void addConnections2(List connections) {
		for (Iterator iter = connections.iterator(); iter.hasNext();) {
			Connection conn = (Connection) iter.next();
			if (!(conn.getSource() == conn.getTarget())) {
				conn.reconnect();
			}
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#canUndo()
	 */
	public boolean canUndo() {
		return wasRemoved;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute() {
		// store a copy of incoming & outgoing connections before proceeding
		sourceConnections = child.getSourceConnections();
		targetConnections = child.getTargetConnections();
		redo();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo() {
		// remove the child and disconnect its connections
		wasRemoved = parent.removeChild(child);
		if (wasRemoved) {
			removeConnections(sourceConnections);
			removeConnections(targetConnections);
		}
	}

	/**
	 * Disconnects a List of Connections from their endpoints.
	 * 
	 * @param connections
	 *            a non-null List of connections
	 */
	private void removeConnections(List connections) {
		for (Iterator iter = connections.iterator(); iter.hasNext();) {
			Connection conn = (Connection) iter.next();
			conn.disconnect();
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo() {
		// add the child and reconnect its connections
		if (parent.addChild(child)) {
			addConnections(sourceConnections);
			addConnections2(targetConnections);
		}
	}
}
