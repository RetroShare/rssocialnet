#include "IdentityPopupMenu.h"

#include <Wt/WText>
#include <Wt/WLineEdit>
#include <Wt/WBreak>
#include <Wt/WPushButton>

#include "RsGxsUpdateBroadcastWt.h"
#include "SonetUtil.h"
#include "RSWApplication.h"

#include "WebUITimer.h"

/*
problem: how to keep the list of identities updated when the user creates a new one?
- can use event from rsIndetity?
- poll every x seconds?
*/

namespace RsWall{

IdentityPopupMenu::IdentityPopupMenu():
    WPopupMenu(), newIdDialog(0), newIdDialogLineEdit(0), newIdMenuItem(0),
    tokenQueue(RSWApplication::ifaces().mIdentity->getTokenService())
{
    // this does not work
    // don't know if own mistake or rsidentities does not forward events
    RsGxsUpdateBroadcastWt::get(RSWApplication::ifaces().mIdentity)->grpsChanged().connect(this, &IdentityPopupMenu::updateIdentities);
    // rsidentity should forward events
    // maybe the own ids are not cached immediately?

    // this should work to refresh when creating a new id
    //tokenQueue.tokenReady().connect(this, &IdentityPopupMenu::onTokenReady);

    itemSelected().connect(this, &IdentityPopupMenu::onSelectionChanged);

    // delay the loading of the ids
    // this allows other to construct this widget and connect to the signal
    // before we load the identities
    WebUITimer::singleShotNextTick(this, &IdentityPopupMenu::updateIdentities);
}

RsIdentityDetails IdentityPopupMenu::getCurrentIdentity()
{
    return currentId;
}

void IdentityPopupMenu::updateIdentities()
{
    bool ok = true;
    std::list<RsGxsId> ids;
    RSWApplication::ifaces().mIdentity->getOwnIds(ids);
    std::vector<RsIdentityDetails> details;
    std::list<RsGxsId>::iterator lit;
    for(lit = ids.begin(); lit != ids.end(); lit++)
    {
        RsIdentityDetails detail;
        ok &= RSWApplication::ifaces().mIdentity->getIdDetails(*lit, detail);
        if(ok) { details.push_back(detail); }
    }
    if(ok)
    {
        // remove old menue items
        if(newIdMenuItem){ removeItem(newIdMenuItem); delete newIdMenuItem; }
        std::map<Wt::WMenuItem*, RsIdentityDetails>::iterator mit;
        for(mit = idDetails.begin(); mit != idDetails.end(); mit++)
        {
            removeItem(mit->first);
            delete mit->first;
        }
        idDetails.clear();

        // add new ones
        std::vector<RsIdentityDetails>::iterator it;
        for(it = details.begin(); it != details.end(); it++){
            Wt::WMenuItem* item = addItem(SonetUtil::formatGxsId(*it));
            idDetails[item] = *it;
            currentId = *it;
        }
        changedSignal.emit(currentId);

        // commented, because i don't know how to remove separators
        //addSeparator();
        newIdMenuItem = addItem("create new identity");
    }
    else
    {
        // id details not cached, try later
        WebUITimer::singleShotNextTick(this, &IdentityPopupMenu::updateIdentities);
    }
}

void IdentityPopupMenu::onSelectionChanged(Wt::WMenuItem* item)
{
    if(item == newIdMenuItem)
    {
        // copy of void RSWappTestPage::showNewIdDialog()
        newIdDialog = new Wt::WDialog("create a new Identity");
        new Wt::WText("name", newIdDialog->contents());
        Wt::WLineEdit *edit = new Wt::WLineEdit(newIdDialog->contents());
        newIdDialogLineEdit = edit;
        new Wt::WBreak(newIdDialog->contents());

        Wt::WPushButton *ok = new Wt::WPushButton("Ok", newIdDialog->contents());
        Wt::WPushButton *cancel = new Wt::WPushButton("cancel", newIdDialog->contents());
        // these events will accept() the Dialog
        edit->enterPressed().connect(newIdDialog, &Wt::WDialog::accept);
        ok->clicked().connect(newIdDialog, &Wt::WDialog::accept);
        cancel->clicked().connect(newIdDialog, &Wt::WDialog::reject);

        newIdDialog->finished().connect(this, &IdentityPopupMenu::newIdDialogDone);
        newIdDialog->show();
    }
    else
    {
        std::map<Wt::WMenuItem*, RsIdentityDetails>::iterator mit;
        for(mit = idDetails.begin(); mit != idDetails.end(); mit++)
        {
            if(mit->first == item)
            {
                currentId = mit->second;
            }
        }
        changedSignal.emit(currentId);
    }
}

void IdentityPopupMenu::newIdDialogDone(Wt::WDialog::DialogCode code)
{
    if (code == Wt::WDialog::Accepted){
        std::cerr << "create new id clicked ok" << std::endl;
        RsIdentityParameters params;
        params.nickname = newIdDialogLineEdit->text().toUTF8();
        // todo: pgp password callback
        params.isPgpLinked = false;

        uint32_t token = 0;
        RSWApplication::ifaces().mIdentity->createIdentity(token, params);
        // queue callback when id was created, to updates views then
        //tokenQueue->queueToken(token);
        // lets see if the grp-changed event gets broadcasted to update the list of ids then
        tokenQueue.queueToken(token);
    }
    delete newIdDialog;
}

void IdentityPopupMenu::onTokenReady(uint32_t /*token*/, bool /*ok*/)
{
    updateIdentities();
}
}//namespace RsWall
