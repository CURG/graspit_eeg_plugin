#include "BCI/BCIStateMachine.h"

#include "BCI/states/objectSelectionState.h"
#include "BCI/states/graspSelectionState.h"
#include "BCI/states/placementLocationSelectionState.h"
#include "BCI/states/confirmationState.h"
#include "BCI/states/activateRefinementState.h"
#include "BCI/states/executionState.h"
#include "BCI/states/stoppedExecutionState.h"
#include "BCI/states/objectRecognitionState.h"

#include "BCI/bciService.h"

BCIStateMachine::BCIStateMachine(BCIControlWindow *_bciControlWindow, BCIService *_bciService):
    bciControlWindow(_bciControlWindow),
    bciService(_bciService)
{

    csm = bciService->csm;
    ros::NodeHandle *n = new ros::NodeHandle("");

    ObjectRecognitionState *objectRecognitionState = new ObjectRecognitionState(bciControlWindow, csm, n);
    ObjectSelectionState *objectSelectionState = new ObjectSelectionState(bciControlWindow, csm);
    GraspSelectionState *graspSelectionState = new GraspSelectionState(bciControlWindow, csm);
    ActivateRefinementState *activateRefinementState = new ActivateRefinementState(bciControlWindow, csm);
    GraspSelectionState *finalGraspSelectionState = new GraspSelectionState(bciControlWindow, csm);
    ConfirmationState *confirmationState = new ConfirmationState(bciControlWindow, csm);
    ExecutionState *executionState = new ExecutionState(bciControlWindow, csm, n);
    StoppedExecutionState *stoppedExecutionState = new StoppedExecutionState(bciControlWindow, csm);

    objectRecognitionState->addStateTransition(bciService, SIGNAL(finishedRecognition()), objectSelectionState);

    objectSelectionState->addStateTransition(bciService,SIGNAL(goToNextState1()), graspSelectionState);
    objectSelectionState->addStateTransition(objectSelectionState,SIGNAL(goToNextState()), graspSelectionState);
    objectSelectionState->addSelfTransition(bciService, SIGNAL(exec()), objectSelectionState, SLOT(onSelect()));
    objectSelectionState->addSelfTransition(bciService, SIGNAL(rotLat()), objectSelectionState, SLOT(onNext()));

    objectSelectionState->addStateTransition(bciService,SIGNAL(goToPreviousState()), objectRecognitionState);

    graspSelectionState->addStateTransition(graspSelectionState, SIGNAL(goToActivateRefinementState()), activateRefinementState);
    graspSelectionState->addStateTransition(graspSelectionState, SIGNAL(goToObjectSelectionState()), objectSelectionState);
    graspSelectionState->addStateTransition(graspSelectionState, SIGNAL(goToConfirmationState()), confirmationState);

    activateRefinementState->addStateTransition(bciService, SIGNAL(goToNextState1()), graspSelectionState);

    confirmationState->addStateTransition(confirmationState, SIGNAL(goToExecutionState()), executionState);
    confirmationState->addStateTransition(confirmationState, SIGNAL(goToPreviousState()), graspSelectionState);

    executionState->addStateTransition(executionState, SIGNAL(goToStoppedExecutionState()), stoppedExecutionState);

    stoppedExecutionState->addStateTransition(stoppedExecutionState, SIGNAL(goToExecutionState()), executionState);
    stoppedExecutionState->addStateTransition(stoppedExecutionState, SIGNAL(goToObjectSelectionState()), objectSelectionState);

    stateMachine.addState(objectRecognitionState);
    stateMachine.addState(objectSelectionState);
    stateMachine.addState(graspSelectionState);
    stateMachine.addState(activateRefinementState);
    stateMachine.addState(finalGraspSelectionState);
    stateMachine.addState(confirmationState);
    stateMachine.addState(executionState);
    stateMachine.addState(stoppedExecutionState);

    stateMachine.setInitialState(objectRecognitionState);
}

void BCIStateMachine::start()
{
    stateMachine.start();
    bciControlWindow->currentFrame->show();
}


