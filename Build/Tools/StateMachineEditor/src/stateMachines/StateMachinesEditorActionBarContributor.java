/*
 * Created on 05.07.2005
 */
package stateMachines;

import org.eclipse.jface.action.IToolBarManager;
import org.eclipse.jface.action.Separator;
import org.eclipse.ui.actions.ActionFactory;

import org.eclipse.gef.editparts.ZoomManager;
import org.eclipse.gef.ui.actions.ActionBarContributor;
import org.eclipse.gef.ui.actions.DeleteRetargetAction;
import org.eclipse.gef.ui.actions.GEFActionConstants;
import org.eclipse.gef.ui.actions.RedoRetargetAction;
import org.eclipse.gef.ui.actions.UndoRetargetAction;
import org.eclipse.gef.ui.actions.ZoomComboContributionItem;
import org.eclipse.gef.ui.actions.ZoomInRetargetAction;
import org.eclipse.gef.ui.actions.ZoomOutRetargetAction;

import actions.AddRetargetAction;
import actions.ChangeRetargetAction;
import actions.ChangeRetargetEvent;
import actions.RemoveRetargetAction;

/**
 * @author Daniel
 */
public class StateMachinesEditorActionBarContributor extends
		ActionBarContributor {

	/**
	 * Create actions managed by this contributor.
	 * 
	 * @see org.eclipse.gef.ui.actions.ActionBarContributor#buildActions()
	 */
	protected void buildActions() {
		addRetargetAction(new DeleteRetargetAction());
		addRetargetAction(new UndoRetargetAction());
		addRetargetAction(new RedoRetargetAction());
		addRetargetAction(new ZoomOutRetargetAction());
		addRetargetAction(new ZoomInRetargetAction());
		addRetargetAction(new ChangeRetargetAction());
		addRetargetAction(new AddRetargetAction());
		addRetargetAction(new RemoveRetargetAction());
		addRetargetAction(new ChangeRetargetEvent());
	}

	/**
	 * Add actions to the given toolbar.
	 * 
	 * @see org.eclipse.ui.part.EditorActionBarContributor#contributeToToolBar(org.eclipse.jface.action.IToolBarManager)
	 */
	public void contributeToToolBar(IToolBarManager toolBarManager) {
		toolBarManager.add(getAction(ActionFactory.DELETE.getId()));
		toolBarManager.add(getAction(ActionFactory.UNDO.getId()));
		toolBarManager.add(getAction(ActionFactory.REDO.getId()));
		toolBarManager.add(new Separator());
		toolBarManager.add(getAction(GEFActionConstants.ZOOM_OUT));
		toolBarManager.add(getAction(GEFActionConstants.ZOOM_IN));
		toolBarManager.add(new Separator());	
		String[] zoomStrings = new String[] {	ZoomManager.FIT_ALL, 
												ZoomManager.FIT_HEIGHT, 
												ZoomManager.FIT_WIDTH	};
		toolBarManager.add(new ZoomComboContributionItem(getPage(), zoomStrings));
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.ui.actions.ActionBarContributor#declareGlobalActionKeys()
	 */
	protected void declareGlobalActionKeys() {
		// currently none
	}
}
