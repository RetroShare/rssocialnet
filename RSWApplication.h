#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

using namespace Wt;

class RsPlugInInterfaces ;

class RSWApplication: public WApplication
{
	public:
		RSWApplication(const WEnvironment& env,const RsPlugInInterfaces& interf);

	private:
		WLineEdit *nameEdit_;
		WText *greeting_;

		void greet();
};

