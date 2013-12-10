/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "plAnimComponent.h"
#include "plActivatorBaseComponent.h"
#include "plNoteTrackDlgComp.h"
#include "MaxMain/plMaxNode.h"

#include <iparamm2.h>
#include "resource.h"
#pragma hdrstop

#include "plClickDragComponent.h"


#include "pnSceneObject/plSceneObject.h"
#include "pnKeyedObject/hsKeyedObject.h"

#include "plPhysical/plCollisionDetector.h"  // MM
#include "plModifier/plLogicModifier.h"
#include "plModifier/plAxisAnimModifier.h"
#include "pnModifier/plConditionalObject.h"
#include "plPhysical/plPickingDetector.h"
#include "pfConditional/plActivatorConditionalObject.h"
#include "pfConditional/plFacingConditionalObject.h"
#include "pfConditional/plObjectInBoxConditionalObject.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plCursorChangeMsg.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "plMessage/plAnimCmdMsg.h"

#include "MaxConvert/plConvert.h"

#include "plResponderComponent.h"

#include "MaxMain/plPhysicalProps.h"
#include "plNotetrackAnim.h"

#include "plPhysical/plSimDefs.h"

void DummyCodeIncludeFuncClickDrag() {}

CLASS_DESC(plClickDragComponent, gClickDragDesc, "Dragable",  "Dragable", COMP_TYPE_DETECTOR, CLICK_DRAG_CID)


enum
{
    kClickDragDirectional,
    kClickDragDegrees,
    kClickDragUseProxy,
    kClickDragProxy,
    kClickDragUseRegion,
    kClickDragProxyRegion,
    kClickDragUseX,
    kClickDragUseY,
    kClickDragAnimX,
    kClickDragAnimY,
    kClickDragUseLoopX,
    kClickDragUseLoopY,
    kClickDragLoopAnimX,
    kClickDragLoopAnimY,
    kClickDragToggle,
    kClickDragAllOrNothing,
    kClickDragOneShot,
    kClikDragBoundsType,
    kClikDragEnabled,
};

class plClickDragComponentProc : public ParamMap2UserDlgProc
{
protected:
    plComponentNoteTrackDlg fNoteTrackDlgX;
    plComponentNoteTrackDlg fNoteTrackDlgY;
    IParamBlock2 *fPB;

public:
    BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            fPB = map->GetParamBlock();
            
            fNoteTrackDlgX.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX),
                                nil,
                                kClickDragAnimX,
                                -1,
                                fPB,
                                fPB->GetOwner());

            fNoteTrackDlgX.Load();

            EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIMX), true);
            
            fNoteTrackDlgY.Init(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y),
                                nil,
                                kClickDragAnimY,
                                -1,
                                fPB,
                                fPB->GetOwner());

            fNoteTrackDlgY.Load();

            EnableWindow(GetDlgItem(hWnd, IDC_COMP_CLICK_DRAG_ANIM_Y), true);
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_CLICK_DRAG_ANIMX)
            {
                fNoteTrackDlgX.AnimChanged();
                return TRUE;
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_CLICK_DRAG_ANIM_Y)
            {
                fNoteTrackDlgY.AnimChanged();
                return TRUE;
            }
            break;
        }
        return false;   
    }

    void DeleteThis()
    {
        fNoteTrackDlgX.DeleteCache();
        fNoteTrackDlgY.DeleteCache();
    }
};
static plClickDragComponentProc gClickDragComponentProc;

ParamBlockDesc2 gClickDragBlock
(
    
    plComponent::kBlkComp, _T("ClickDragComp"), 0, &gClickDragDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_DETECTOR_CLICK_DRAGGABLE, IDS_COMP_DETECTOR_CLICK_DRAGGABLE, 0, 0, &gClickDragComponentProc,

    kClickDragDirectional,      _T("directional"),      TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_OMNI,
        end,

    kClickDragDegrees, _T("degrees"),   TYPE_INT,   P_ANIMATABLE, 0,    
        p_range, 1, 180,
        p_default, 180,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT, 
        IDC_COMP_CLICK_DRAG_DEG,    IDC_COMP_CLICK_DRAG_DEGSPIN, SPIN_AUTOSCALE,
        end,

    kClickDragUseProxy,     _T("useProxy"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_USEPROXYPHYS,
        p_enable_ctrls, 1, kClickDragProxy,
        end,

    kClickDragProxy, _T("proxyPrimitave"),  TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_DRAG_PROXY,
        p_sclassID,  GEOMOBJECT_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        end,

    kClickDragProxyRegion, _T("proxyRegion"),   TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_CLICK_DRAG_PROXYREGION,
        p_sclassID,  GEOMOBJECT_CLASS_ID,
        p_prompt, IDS_COMP_PHYS_CHOSEN_BASE,
        end,


    kClickDragUseX,     _T("useXAnim"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_USEX,
        end,
    
    kClickDragAnimX,    _T("XanimName"),        TYPE_STRING,    0, 0,
        end,

    kClickDragUseY,     _T("useYAnim"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_USEYANIM,
        end,
    
    kClickDragAnimY,    _T("YanimName"),        TYPE_STRING,    0, 0,
        end,

    kClickDragUseLoopX,     _T("useLoopXAnim"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_USE_LOOPX,
        end,
    
    kClickDragUseLoopY,     _T("useLoopYAnim"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_USE_LOOPY,
        end,

    kClickDragAllOrNothing,     _T("allOrNot"),     TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_COMP_CLICK_DRAG_ALLORNOT,
        end,

    kClickDragOneShot,      _T("oneshot"),      TYPE_BOOL,              0, 0,
        p_ui,               TYPE_SINGLECHEKBOX, IDC_ONESHOT,
        end,

    kClikDragBoundsType,    _T("BoundingConditions"),       TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 4, IDC_RADIO_BSPHERE, IDC_RADIO_BBOX, IDC_RADIO_BHULL, IDC_RADIO_PICKSTATE,
        p_vals,                     plSimDefs::kSphereBounds,       plSimDefs::kBoxBounds,      plSimDefs::kHullBounds,     plSimDefs::kProxyBounds,
        p_default, plSimDefs::kHullBounds,
        end,

    kClikDragEnabled,       _T("enabled"),      TYPE_BOOL,          0, 0,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_ENABLE,
        p_default, TRUE,
        end,

    end
);

plClickDragComponent::plClickDragComponent()
{
    fClassDesc = &gClickDragDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

const plClickDragComponent::LogicKeys& plClickDragComponent::GetAxisKeys()
{
    return fAxisKeys;
}

plKey plClickDragComponent::GetAxisKey(plMaxNode* node)
{
    LogicKeys::const_iterator it = fAxisKeys.find(node);
    if (it != fAxisKeys.end())
        return(it->second);

    return nil;
}

void plClickDragComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
    if( fCompPB->GetInt(kClickDragUseProxy) )
    {
        INode* dragNode = fCompPB->GetINode(kClickDragProxy);
        if( dragNode )
            nonDrawables.Append(1, &dragNode);
    }
    INode* boundsNode = fCompPB->GetINode(kClickDragProxyRegion);
    if( boundsNode )
        nonDrawables.Append(1, &boundsNode);

}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plClickDragComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{

    plActivatorBaseComponent::SetupProperties(node, pErrMsg);

    // Phys Props for the Clickable itself.
    plMaxNode *dragNode = node;
    if (fCompPB->GetInt(kClickDragUseProxy))
    {
        dragNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxy);
        if (dragNode)
            dragNode->SetDrawable(false);
        else
            dragNode = node;
    }

    if(dragNode)
    {
        plPhysicalProps *physProps = dragNode->GetPhysicalProps();
        physProps->SetLOSUIItem(true, dragNode, pErrMsg);
        // only if movable will it have mass (then it will keep track of movements in PhysX)
        if ( dragNode->IsMovable() || dragNode->IsTMAnimatedRecur() )
            physProps->SetMass(1.0, dragNode, pErrMsg);
        physProps->SetGroup( plSimDefs::kGroupStatic, dragNode, pErrMsg);
        physProps->SetFriction(0.0, dragNode, pErrMsg);
//      physProps->SetReportGroup( plPhysicsGroups::kLocalAvatars, dragNode, pErrMsg);
        physProps->SetBoundsType(fCompPB->GetInt(kClikDragBoundsType), dragNode, pErrMsg);
    }

    // Phys Properties for the auto-generated Detector Region...
    plMaxNode* boundsNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxyRegion);
    if(boundsNode)
    {
        plPhysicalProps *physPropsDetector = boundsNode->GetPhysicalProps();
//      physPropsDetector->SetAllowLOS(true, boundsNode, pErrMsg);
        physPropsDetector->SetProxyNode(boundsNode, node, pErrMsg);
        physPropsDetector->SetBoundsType(plSimDefs::kHullBounds, boundsNode, pErrMsg);
        // only if movable will it have mass (then it will keep track of movements in PhysX)
        if ( boundsNode->IsMovable() || boundsNode->IsTMAnimatedRecur() )
            physPropsDetector->SetMass(1.0, boundsNode, pErrMsg);

        physPropsDetector->SetGroup(plSimDefs::kGroupDetector, boundsNode, pErrMsg );
        //boundsNode->GetPhysicalProps()->SetAllCollideGroups(0);
        physPropsDetector->SetReportGroup(1<<plSimDefs::kGroupAvatar, boundsNode, pErrMsg );
    }

    fAxisKeys.clear();
    return true;
}

bool plClickDragComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plMaxNode *dragNode = node;
    if (fCompPB->GetInt(kClickDragUseProxy))
    {
        dragNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxy);
        if (dragNode)
            dragNode->SetDrawable(false);
        else
            dragNode = node;
    }

    plActivatorBaseComponent::PreConvert(node, pErrMsg);
    plLogicModifier *logic = plLogicModifier::ConvertNoRef(fLogicModKeys[node]->GetObjectPtr());

    if (fCompPB->GetInt(kClickDragOneShot))
        logic->SetFlag(plLogicModBase::kOneShot);

    plLocation loc = node->GetLocation();
    plSceneObject *obj = node->GetSceneObject();

    // do the same thing for axis animation controllers.
    plAxisAnimModifier* pAxis = new plAxisAnimModifier;
    plKey axisKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pAxis, loc);
    hsgResMgr::ResMgr()->AddViaNotify(axisKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
    logic->AddNotifyReceiver(axisKey);
    
    fAxisKeys[node] = axisKey;

    return true;
}

bool plClickDragComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    //
    // Error checking
    //
    plMaxNode* dragProxyNode = node;
    if (fCompPB->GetInt(kClickDragUseProxy))
    {
        dragProxyNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxy);
        if (!dragProxyNode || !dragProxyNode->CanConvert())
        {
            pErrMsg->Set(true,
                        "Draggable Error",
                        "The Draggaable '%s' on node '%s' is set to use a proxy but doesn't have one, or it didn't convert.\n"
                        "The node the Draggable is attached to will be used instead.",
                        GetINode()->GetName(), node->GetName()).Show();
            pErrMsg->Set(false);

            dragProxyNode = node;
        }
    }

    plMaxNode* boundsNode = (plMaxNode*)fCompPB->GetINode(kClickDragProxyRegion);
    if (!boundsNode || !boundsNode->CanConvert())
    {
        pErrMsg->Set(true,
                    "Draggable Error",
                    "The Draggable '%s' on node '%s' has a required region that is missing, or didn't convert.\n"
                    "The export will be aborted.",
                    GetINode()->GetName(), node->GetName()).Show();
        return false;
    }

    plLocation loc = node->GetLocation();
    plSceneObject *obj = node->GetSceneObject();
    
    plKey logicKey = fLogicModKeys[node];
    plLogicModifier *logic = plLogicModifier::ConvertNoRef(logicKey->GetObjectPtr());
    logic->fMyCursor = plCursorChangeMsg::kCursorOpen;

    // Create the detector
    plDetectorModifier *detector = nil;
    detector = new plPickingDetector;

    // Register the detector
    plKey detectorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), detector, loc);
    hsgResMgr::ResMgr()->AddViaNotify(detectorKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);

    // set up the axis anim controller
    plKey axisKey = fAxisKeys[node];
    plAxisAnimModifier* pAxis = plAxisAnimModifier::ConvertNoRef(axisKey->GetObjectPtr());

    // attach the animation controller to the animation objects:
    // find an animation controller:
    hsTArray<plKey> receivers;
    IGetReceivers(node, receivers);
    
    int i;
    for (i = 0; i < receivers.Count(); i++)
        pAxis->GetNotify()->AddReceiver(receivers[i]);

    pAxis->SetNotificationKey(logicKey);
    uint32_t count = node->NumAttachedComponents();
    bool bHasAnim = false;
    plAnimComponentBase* pAnim = nil;

    for (i = 0; i < count; i++)
    {
        plComponentBase *comp = node->GetAttachedComponent(i);
        if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
        {
            pAnim = (plAnimComponentBase*)comp;
            break;
        }
    }
    if (!pAnim)
    {
        pErrMsg->Set(true, "WARNING", "Object %s has click-drag component attached but NO animation component!", ((INode*)node)->GetName()).Show();
        pErrMsg->Set(false);
    }
    else
    {
        if (fCompPB->GetInt(kClickDragUseX))
        {
            pAxis->SetXAnim( pAnim->GetModKey(node) );
        }   
        else // take out this else when we support multiple channels
        if (fCompPB->GetInt(kClickDragUseY))
        {
            pAxis->SetYAnim( pAnim->GetModKey(node) );
        }
        pAxis->SetAllOrNothing(fCompPB->GetInt(kClickDragAllOrNothing));

        // add callbacks for beginning and end of animation
        plEventCallbackMsg* pCall1 = new plEventCallbackMsg;
        pCall1->fEvent = kBegin;
        pCall1->fRepeats = -1;
        pCall1->AddReceiver(axisKey);
        
        plEventCallbackMsg* pCall2 = new plEventCallbackMsg;
        pCall2->fEvent = kEnd;
        pCall2->fRepeats = -1;
        pCall2->AddReceiver(axisKey);

        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
        plString tempAnimName = pAnim->GetAnimName();
        if (tempAnimName.IsNull())
        {
            //pMsg->SetAnimName(ENTIRE_ANIMATION_NAME);
            pMsg->SetAnimName(pAnim->GetModKey(node)->GetName());
            pAxis->SetAnimLabel(ENTIRE_ANIMATION_NAME);
        }
        else
        {
            //pMsg->SetAnimName(tempAnimName);
            pMsg->SetAnimName(pAnim->GetModKey(node)->GetName());
            pAxis->SetAnimLabel(tempAnimName);
        }
        

        pMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
        pMsg->AddCallback(pCall1);
        pMsg->AddCallback(pCall2);

        hsRefCnt_SafeUnRef( pCall1 );
        hsRefCnt_SafeUnRef( pCall2 );

        pMsg->AddReceiver( pAnim->GetModKey(node) );
        plgDispatch::MsgSend(pMsg);
    }

    // create and register the CONDITIONS for the DETECTOR's Logic Modifier
    plActivatorConditionalObject* activatorCond = new plActivatorConditionalObject;
    plKey activatorKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), activatorCond, loc);

    //
    // Create required region
    //
    // need a player in box condition here...
    // first a detector-any for the box
    plObjectInVolumeDetector* pCDet = new plObjectInVolumeDetector(plCollisionDetector::kTypeAny);
    plKey cDetKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pCDet, loc);
    hsgResMgr::ResMgr()->AddViaNotify(cDetKey, new plObjRefMsg(boundsNode->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
    pCDet->AddLogicObj(logicKey);

    // then an object-in-box condition for the logic mod
    plObjectInBoxConditionalObject* boxCond = new plObjectInBoxConditionalObject;
    plKey boxCondKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), boxCond, loc);
    logic->AddCondition(boxCond);

    // How do we feel about player facing
    plFacingConditionalObject* facingCond = new plFacingConditionalObject;
    facingCond->SetDirectional(fCompPB->GetInt(kClickDragDirectional));
    int deg = fCompPB->GetInt(kClickDragDegrees);
    if (deg > 180)
        deg = 180;
    float rad = hsDegreesToRadians(deg);
    facingCond->SetTolerance(cos(rad));
    plKey facingKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), facingCond, loc);
    
    detector->AddLogicObj(logicKey);     // send messages to this logic component
    activatorCond->SetActivatorKey(detectorKey); // Tells the activator condition to look for stimulus from the detector
    
    logic->AddCondition(activatorCond); // add this activator condition
    logic->AddCondition(facingCond);

    logic->SetDisabled(fCompPB->GetInt(kClikDragEnabled) == 0);

    
    // If this is for the SceneViewer, set the local only flag since the read function will never be called
    if (plConvert::Instance().IsForSceneViewer())
        logic->SetLocalOnly(true);

    return true;
}

bool plClickDragComponent::DeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
    fAxisKeys.clear();
    return plActivatorBaseComponent::DeInit( node, pErrMsg ); 
}
