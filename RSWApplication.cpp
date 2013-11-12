#include "RSWApplication.h"

RSWApplication::RSWApplication(const WEnvironment& env)
   : WApplication(env)
{
	setTitle("Hello world");               // application title

   root()->addWidget(new WText("Your name, please ? "));
   // show some text
   nameEdit_ = new WLineEdit(root());    // allow text input
   nameEdit_->setFocus();                // give focus

   WPushButton *b = new WPushButton("Greet me.", root());

   // create a button
	//
   b->setMargin(5, Wt::Left);       // add 5 pixels margin
   root()->addWidget(new WBreak());      // insert a line break

   greeting_ = new WText(root());        // empty text

   // Connect signals with slots

   b->clicked().connect(this, &RSWApplication::greet);
   nameEdit_->enterPressed().connect(this, &RSWApplication::greet);
}

void RSWApplication::greet()
{
    greeting_->setText("Hello there, " + nameEdit_->text());
}

