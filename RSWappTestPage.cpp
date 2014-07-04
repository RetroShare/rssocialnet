#include "RSWappTestPage.h"

#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WStackedWidget>
#include <Wt/WMenu>
#include <Wt/WBorderLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WDefaultLayout>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WBreak>

#include <Wt/WAbstractTableModel>
#include <Wt/WModelIndex>
#include <Wt/WItemDelegate>
#include <Wt/WTableView>

#include <Wt/WColor>

#include "rswall.h"

#include "sonet/FirstStepsWidget.h"
#include "sonet/NewsfeedWidget.h"
#include "sonet/CreateWallWidget.h"
#include "sonet/WallChooserWidget.h"
#include "sonet/WallTestWidget.h"
#include "sonet/AvatarWidget.h"

#include <retroshare/rsidentity.h>
#include "RSWApplication.h"

class IdentityModel: public Wt::WAbstractTableModel{
public:
    IdentityModel(): WAbstractTableModel(), mTokenQueue(RSWApplication::ifaces().mIdentity->getTokenService())
    {
        update();
        mTokenQueue.tokenReady().connect(this, &IdentityModel::tokenReady);
    }

    void update()
    {
        RsTokReqOptions opts;
        opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;

        uint32_t token;
        RSWApplication::ifaces().mIdentity->getTokenService()->requestGroupInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts);

        mTokenQueue.queueToken(token);
    }
    virtual void tokenReady(uint32_t token, bool /*ok*/)
    {
        std::cerr << "IdentityModel::tokenReady()" << std::endl;
        reset();
        mIds.clear();
        RSWApplication::ifaces().mIdentity->getGroupData(token, mIds);
    }

    int rowCount(const Wt::WModelIndex &parent=Wt::WModelIndex()) const
    {
        if(!parent.isValid()){
            return mIds.size();
        }else{
            return 0;
        }
    }

    int columnCount(const Wt::WModelIndex &parent=Wt::WModelIndex()) const
    {
        if(!parent.isValid()){
            return 4;
        }else{
            return 0;
        }
    }

    boost::any data(const Wt::WModelIndex &index, int role) const
    {
        const RsGxsIdGroup &id = mIds[index.row()];
        switch(role){
        case Wt::DisplayRole:
            switch(index.column()){
            case 0:
                return Wt::WString::fromUTF8(id.mMeta.mGroupName);
            case 1:
                return Wt::WString::fromUTF8(id.mMeta.mGroupId.toStdString());
            case 2:
                return Wt::WString::fromUTF8(id.mPgpId.toStdString());
            }
            break;
        case Wt::CheckStateRole:
            switch(index.column())
            {
            case 3:
                // not sure if a gxs-id should be cerated from the grp id
                // or if the gxs-id is somewhere in the grp info
                bool subscribed;
                RsGxsId identity = RsGxsId(id.mMeta.mGroupId);
                bool ok = RsWall::rsWall->isAuthorSubscribed(identity, subscribed);
                if(ok)
                {
                    if(subscribed)
                    {
                        return true;
                    }
                    return false;
                }
                else
                {
                    return false;
                }
            }
            break;
        }
        return boost::any();
    }
    // no effect?
    virtual Wt::WFlags<Wt::ItemFlag> flags(const Wt::WModelIndex &index) const
    {
        if(index.column()==3)
        {
            return Wt::ItemIsSelectable | Wt::ItemIsUserCheckable;
        }
        else
        {
            return Wt::ItemIsSelectable;
        }
    }
    virtual bool setData(const Wt::WModelIndex &index, const boost::any &value, int role)
    {
        std::cerr<<"model: setData()"<<std::endl;
        const RsGxsIdGroup &id = mIds[index.row()];
        if(index.column() == 3)
        {
            bool subscribe = boost::any_cast<bool>(value);
            RsGxsId identity(id.mMeta.mGroupId);
            RsWall::rsWall->subscribeToAuthor(identity, subscribe);
            return true;
        }
        return false;
    }

private:
    std::vector<RsGxsIdGroup> mIds;
    RsWall::TokenQueueWt2 mTokenQueue;
};

class FeedDelegate: public Wt::WItemDelegate{
public:
    virtual Wt::WWidget* update(Wt::WWidget *widget, const Wt::WModelIndex &index, Wt::WFlags<Wt::ViewItemRenderFlag> /*flags*/)
    {
        Wt::WText *text = dynamic_cast<Wt::WText*>(widget);
        if(!text){
            text = new Wt::WText();
        }
        text->setText(Wt::WString::fromUTF8(boost::any_cast<std::string>(index.data(Wt::UserRole)).c_str()));
        return text;
    }
};

class FeedModel: public Wt::WAbstractTableModel{
public:
    FeedModel(): WAbstractTableModel()
    {

    }

    int rowCount(const Wt::WModelIndex &parent=Wt::WModelIndex()) const
    {
        if(!parent.isValid()){
            return mContent.size();
        }else{
            return 0;
        }
    }

    int columnCount(const Wt::WModelIndex &parent=Wt::WModelIndex()) const
    {
        if(!parent.isValid()){
            return 1;
        }else{
            return 0;
        }
    }

    boost::any data(const Wt::WModelIndex &index, int role) const
    {
        switch(role){
        case Wt::UserRole:
            return mContent[index.row()];
        default:
            return boost::any();
        }
    }

    // call this before using this model
    // todo: handle addition and removal of items when in use
    void addSomething(std::string something)
    {
        mContent.push_back(something);
    }
private:
    std::vector<std::string> mContent;
};

class FeedWidget: public Wt::WContainerWidget{
public:
    FeedWidget(): WContainerWidget()
    {
        Wt::WBorderLayout *layout = new Wt::WBorderLayout();
        this->setLayout(layout);

        Wt::WImage *avatar = new Wt::WImage();
        avatar->resize(80, 80);
        // load image from memory
        //Wt::WMemoryResource();
        Wt::WText *text = new Wt::WText(Wt::WString("hi there"));

        // a BorderLayout can only handle one widget in each region
        // else will crash
        // docs also say only one widget
        // so make another container and place widgets there
        mCenterContainer = new Wt::WContainerWidget();

        mCenterContainer->addWidget(text);
        // put avatarin own container, to prevent stretching from BorderLayout
        Wt::WContainerWidget *avatarContainer = new Wt::WContainerWidget();
        avatarContainer->addWidget(avatar);
        layout->addWidget(avatarContainer, Wt::WBorderLayout::West);
        layout->addWidget(mCenterContainer, Wt::WBorderLayout::Center);

    }
    void postWidget_addReply(WWidget *child){
        mCenterContainer->addWidget(child);
    }

private:
    Wt::WContainerWidget *mCenterContainer;
};

RSWappTestPage::RSWappTestPage(Wt::WContainerWidget *parent):
    WCompositeWidget(parent)
{
    tokenQueue = new RsWall::TokenQueueWt2(RSWApplication::ifaces().mIdentity->getTokenService());
    tokenQueue->tokenReady().connect(this, &RSWappTestPage::tokenReady);
    // allow automatic destruction
    Wt::WObject::addChild(tokenQueue);

    setImplementation(_impl = new Wt::WContainerWidget());

    Wt::WBorderLayout *layout = new Wt::WBorderLayout();
    _impl->setLayout(layout);

    // Create a stack where the contents will be located.
    Wt::WStackedWidget *contents = new Wt::WStackedWidget();

    Wt::WMenu *menu = new Wt::WMenu(contents, Wt::Vertical);
    layout->addWidget(menu, Wt::WBorderLayout::West);
    layout->addWidget(contents, Wt::WBorderLayout::Center);
    //menu->setStyleClass("nav nav-pills nav-stacked");
    menu->setWidth(150);

    FeedWidget *w1 = new FeedWidget();
    FeedWidget *w2 = new FeedWidget();
    FeedWidget *w3 = new FeedWidget();
    w1->postWidget_addReply(w2);
    w1->postWidget_addReply(w3);

    Wt::WTableView *tableView = new Wt::WTableView();
    FeedModel *model = new FeedModel();
    model->addSomething(std::string("eins"));
    model->addSomething(std::string("zwei<br/>zwei punkt zwei"));
    model->addSomething(std::string("drei"));
    // this does not work
    // table has fixed 20px height, not good
    // table renders all rows with same height, not good
    // other idea: just add button to load more posts
    //tableView->setRowHeight(Wt::WLength::Auto);
    tableView->setModel(model);
    tableView->setItemDelegate(new FeedDelegate());
    tableView->setHeaderHeight(0);
    tableView->setColumnBorder(Wt::WColor(0,0,0,0));

    // IdentityModel test
    Wt::WTableView *tableViewIdentities = new Wt::WTableView();
    IdentityModel *identityModel = new IdentityModel();
    idModel = identityModel;
    tableViewIdentities->setModel(identityModel);

    Wt::WPushButton *buttonNewId = new Wt::WPushButton("create new identity");
    buttonNewId->clicked().connect(this, &RSWappTestPage::showNewIdDialog);

    Wt::WContainerWidget *idContainer = new Wt::WContainerWidget();
    idContainer->addWidget(buttonNewId);
    idContainer->addWidget(tableViewIdentities);

    // Add menu items using the default lazy loading policy.
    //menu->addItem("feed", w1);
    //menu->addItem("tableView", tableView);
    //menu->addItem("IdentityView", idContainer);

    menu->addItem("CreateWallWidget", new RsWall::CreateWallWidget());
    menu->addItem("WallChooserWidget", new RsWall::WallChooserWidget());
    menu->addItem("WallTestWidget", new RsWall::WallTestWidget());
    menu->addItem("AvatarWidget", new RsWall::AvatarWidgetWt(false));
}

void RSWappTestPage::showNewIdDialog()
{
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

    newIdDialog->finished().connect(this, &RSWappTestPage::newIdDialogDone);
    newIdDialog->show();
}

void RSWappTestPage::newIdDialogDone(Wt::WDialog::DialogCode code)
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
        tokenQueue->queueToken(token);

    }
    delete newIdDialog;
}

void RSWappTestPage::tokenReady(uint32_t /*token*/, bool /*ok*/){
    idModel->update();
}
