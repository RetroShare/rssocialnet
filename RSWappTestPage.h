#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WDialog>

// have to include WTableView before TokenQueue.h, else compiler throws errors
#include <Wt/WTableView>
#include "../../retroshare-gui/src/util/TokenQueue.h"

class TokenQueueWt;
class IdentityModel;

class RSWappTestPage : public Wt::WCompositeWidget, public TokenResponse
{
public:
    RSWappTestPage(Wt::WContainerWidget* parent);
    void showNewIdDialog();
    void newIdDialogDone(Wt::WDialog::DialogCode code);

    virtual void loadRequest(const TokenQueueBase *queue, const TokenRequest &req);
private:
    Wt::WContainerWidget *_impl;

    IdentityModel * idModel;

    Wt::WDialog *newIdDialog;
    Wt::WLineEdit *newIdDialogLineEdit;
    TokenQueueWt *tokenQueue;
};
