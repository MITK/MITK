/*
 * Created on 05.07.2005
 */
package commands;

import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Rectangle;

import org.eclipse.gef.commands.Command;

import model.EndState;
import model.StartState;
import model.States;
import model.StateMachinesDiagram;

/**
 * @author Daniel
 */
public class StatesCreateCommand extends Command {

	/** The new state. */
	private States newState;

	/** StateMachinesDiagram to add to. */
	private final StateMachinesDiagram parent;

	/** The bounds of the new State. */
	private Rectangle bounds;

	/**
	 * Create a command that will add a new State to a StateMachinesDiagram.
	 * 
	 * @param newState
	 *            the new State that is to be added
	 * @param parent
	 *            the StateMachinesDiagram that will hold the new element
	 * @param bounds
	 *            the bounds of the new state; the size can be (-1, -1) if not
	 *            known
	 * @throws IllegalArgumentException
	 *             if any parameter is null, or the request does not provide a
	 *             new State instance
	 */
	public StatesCreateCommand(States newState, StateMachinesDiagram parent,
			Rectangle bounds) {
		this.newState = newState;
		this.parent = parent;
		this.bounds = bounds;
		setLabel("State creation");
	}

	/**
	 * Can execute if all the necessary information has been provided.
	 * 
	 * @see org.eclipse.gef.commands.Command#canExecute()
	 */
	public boolean canExecute() {
		return newState != null && parent != null && bounds != null;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute() {
		newState.setStateName("Statename");
		newState.setParent(parent);
		newState.setStateId(Integer.toString(parent.getMaxID()));
		if (newState instanceof StartState) {
			((StartState) newState).setStartState();
		}
		if (newState instanceof EndState) {
			((EndState) newState).setEndState();
		}
		newState.setLocation(bounds.getLocation());
		Dimension size = new Dimension();
		size.height = 50;
		size.width = 100;
		newState.setSize(size);
		redo();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo() {
		parent.addChild(newState);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo() {
		parent.removeChild(newState);
	}

}