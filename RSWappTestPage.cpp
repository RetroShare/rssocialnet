#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include "RSWappTestPage.h"

#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WStackedWidget>
#include <Wt/WMenu>
#include <Wt/WBorderLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WDefaultLayout>

#include <Wt/WAbstractTableModel>
#include <Wt/WModelIndex>
#include <Wt/WItemDelegate>
#include <Wt/WTableView>

#include <Wt/WColor>

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

    // Add menu items using the default lazy loading policy.
    menu->addItem("feed", w1);
    menu->addItem("tableView", tableView);
    menu->addItem("eins", new Wt::WText("eins"));
    menu->addItem("zwei", new Wt::WText("zwei"));
}
