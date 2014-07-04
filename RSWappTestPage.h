#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WDialog>
#include <Wt/WTableView>

#include "sonet/TokenQueueWt2.h"

class IdentityModel;

class RSWappTestPage : public Wt::WCompositeWidget
{
public:
    RSWappTestPage(Wt::WContainerWidget* parent = 0);
    void showNewIdDialog();
    void newIdDialogDone(Wt::WDialog::DialogCode code);

    virtual void tokenReady(uint32_t token, bool ok);
private:
    Wt::WContainerWidget *_impl;

    IdentityModel * idModel;

    Wt::WDialog *newIdDialog;
    Wt::WLineEdit *newIdDialogLineEdit;
    RsWall::TokenQueueWt2 *tokenQueue;
};
