#include "BCI/states/objectSelectionState.h"
#include "BCI/bciService.h"
#include "state_views/objectSelectionView.h"
#include <Inventor/nodes/SoAnnotation.h>
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"

#include "include/graspitGUI.h"
#include <QGLWidget>


using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;
using bci_experiment::WorldController;


ObjectSelectionState::ObjectSelectionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm,
                                           QState* parent):
    State("ObjectSelectionState", parent),
    bciControlWindow(_bciControlWindow),
    csm(_csm)
{
    objectSelectionView = new ObjectSelectionView(this,bciControlWindow->currentFrame);
    objectSelectionView->hide();
    //this->addSelfTransition(getWorld(), SIGNAL(numElementsChanged()), this, SLOT(onNewObjectFound()));
}


void ObjectSelectionState::onEntry(QEvent *e)
{
    objectSelectionView->show();

    WorldController::getInstance()->highlightAllBodies();
    GraspableBody *currentTarget = OnlinePlannerController::getInstance()->getCurrentTarget();
    state_timer.start();
    //Don't draw guides in this phase
    OnlinePlannerController::getInstance()->stopTimedUpdate();
    OnlinePlannerController::getInstance()->destroyGuides();
    WorldController::getInstance()->highlightCurrentBody(currentTarget);
    OnlinePlannerController::getInstance()->setSceneLocked(false);
    OnlinePlannerController::getInstance()->showRobots(false);
    csm->pipeline=new Pipeline(csm->control_scene_separator, QString("pipeline_object_selection.png"), -0.7 , 0.7, 0.0);
    csm->clearTargets();
    std::shared_ptr<Target>  t1 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("target_active.png"),
                                                                      -1.4, -0.6, 0.0, QString("Next\nObject")));

    std::shared_ptr<Target>  t2 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("target_background.png"),
                                                                      -1.4, -0.8, 0.0, QString("Select\nObject")));

    std::shared_ptr<Target>  t3 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("target_background.png"),
                                                                      -1.4, -1 , 0.0, QString("Rerun\nVision")));

    QObject::connect(t1.get(), SIGNAL(hit()), this, SLOT(onNext()));
    QObject::connect(t2.get(), SIGNAL(hit()), this, SLOT(onSelect()));
    QObject::connect(t3.get(), SIGNAL(hit()), this, SLOT(onGoBack()));

    csm->addTarget(t1);
    csm->addTarget(t2);
    csm->addTarget(t3);
}


void ObjectSelectionState::onExit(QEvent *e)
{

    SoDB::writelock();
     csm->control_scene_separator->removeChild(csm->pipeline->sprite_root);
     SoDB::writeunlock();
     csm->next_target=0;
    delete csm->pipeline;

    objectSelectionView->hide();


    csm->clearTargets();
    float time=(float) state_timer.elapsed()/1000;

    QFile log("/home/srihari/ros/graspit_bci_ws/src/graspit_bci_plugin/log.txt");
    if(log.open(QIODevice::ReadWrite | QIODevice::Text|QIODevice::Append))
    {
        QTextStream stream( &log );
        stream << "Time Elapsed in Object Selection State: " <<time<<" Seconds."<< endl;
    }

    std::cout << "Finished onExit of Object Selection State." << std::endl;


}


void ObjectSelectionState::onNext()
{
    static QTime activeTimer;
    qint64 minElapsedMSecs = 1200;
    if(!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs)
    {

        activeTimer.start();
        GraspableBody *newTarget = OnlinePlannerController::getInstance()->incrementCurrentTarget();
        WorldController::getInstance()->highlightCurrentBody(newTarget);
    }
    csm->setCursorPosition(-1,0,0);
}

void ObjectSelectionState::onSelect()
{
    BCIService::getInstance()->emitGoToNextState1();
}

void ObjectSelectionState::onGoBack()
{
    BCIService::getInstance()->emitGoToPreviousState();
}

//void ObjectSelectionState::onNewObjectFound()
//{
//    GraspableBody *currentTarget = OnlinePlannerController::getInstance()->getCurrentTarget();

//    if(currentTarget)
//    {
//        WorldController::getInstance()->highlightCurrentBody(currentTarget);
//    }

//}
