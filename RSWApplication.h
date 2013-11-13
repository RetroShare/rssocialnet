#include <WApplication>
#include <WBreak>
#include <WContainerWidget>
#include <WLineEdit>
#include <WPushButton>
#include <WText>

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

