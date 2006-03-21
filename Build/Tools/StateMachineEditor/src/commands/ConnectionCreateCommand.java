/*
 * Created on 05.07.2005
 */
package commands;

import org.eclipse.gef.commands.Command;
import org.jdom.Element;

import model.Connection;
import model.States;

/**
 * @author Daniel
 */
public class ConnectionCreateCommand extends Command {
	/** The connection instance. */
	private Connection connection;

	/** Start endpoint for the connection. */
	private final States source;

	/** Target endpoint for the connection. */
	private States target;

	/**
	 * Instantiate a command that can create a connection between two states.
	 * 
	 * @param source
	 *            the source endpoint (a non-null State instance)
	 * @throws IllegalArgumentException
	 *             if source is null
	 */
	public ConnectionCreateCommand(States source) {
		if (source == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Connection creation");
		this.source = source;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#canExecute()
	 */
	public boolean canExecute() {
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute() {
		// create a new connection between source and target
		Element trans = new Element("transition");
		trans.setNamespace(source.getStateElement().getNamespace());
		trans.setAttribute("NAME", "name");
		trans.setAttribute("EVENT_ID", "event");
		// action
		connection = new Connection(source, target, trans);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo() {
		connection.reconnect();
	}

	/**
	 * Set the target endpoint for the connection.
	 * 
	 * @param target
	 *            that target endpoint (a non-null State instance)
	 * @throws IllegalArgumentException
	 *             if target is null
	 */
	public void setTarget(States target) {
		if (target == null) {
			throw new IllegalArgumentException();
		}
		this.target = target;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo() {
		connection.disconnect();
	}
}
