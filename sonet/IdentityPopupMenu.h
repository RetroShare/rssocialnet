#pragma once

#include <Wt/WPopupMenu>
#include <Wt/WSignal>
#include <Wt/WDialog>

#include <retroshare/rsidentity.h>

#include "TokenQueueWt2.h"

namespace RsWall{
class IdentityPopupMenu: public Wt::WPopupMenu
{
public:
    IdentityPopupMenu();
    RsIdentityDetails getCurrentIdentity();
    Wt::Signal<RsIdentityDetails>& identitySelectionChanged(){ return changedSignal; }
private:
    void updateIdentities();
    void onSelectionChanged(Wt::WMenuItem* item);

    void newIdDialogDone(Wt::WDialog::DialogCode code);
    void onTokenReady(uint32_t token, bool ok);
    Wt::WDialog *newIdDialog;
    Wt::WLineEdit *newIdDialogLineEdit;

    Wt::Signal<RsIdentityDetails> changedSignal;
    std::map<Wt::WMenuItem*, RsIdentityDetails> idDetails;
    RsIdentityDetails currentId;
    Wt::WMenuItem* newIdMenuItem;

    TokenQueueWt2 tokenQueue;
};
}//namespace RsWall
