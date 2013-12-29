#include <iostream>
#include <QTimer>
#include "RsWebUIConfig.h"
#include "WebUImain.h"

RsWebUIConfig::RsWebUIConfig(QWidget * parent, Qt::WindowFlags flags)
    : ConfigPage(parent, flags)
{
    /* Invoke the Qt Designer generated object setup routine */
    ui.setupUi(this);
	 _current_mask = 0 ;

	 QObject::connect(ui.IPmask_LE,SIGNAL(textChanged(const QString&)),this,SLOT(on_IPmaskChanged(const QString&))) ;
}

RsWebUIConfig::~RsWebUIConfig() {}

/** Loads the settings for this page */
void RsWebUIConfig::load()
{
    loadSettings();

	 on_IPmaskChanged(ui.IPmask_LE->text()) ;
}

static QString IPmaskToString(uint32_t ip_mask)
{
	QString out ;

	for(int i=0;i<4;++i)
	{
		out += QString::number(ip_mask & 0xff) ;
		if(i < 3)
			out += "." ;
		ip_mask >>= 8 ;
	}
	return out ;
}
static uint32_t stringToIPmask(const QString IP_string,bool& ok)
{
	return 0 ;
}

void RsWebUIConfig::loadSettings() 
{
	ui.port_SB->setValue( RSWebUI::port() ) ;
	ui.IPmask_LE->setText( IPmaskToString(RSWebUI::ipMask()) ) ;
}

bool RsWebUIConfig::save(QString &/*errmsg*/) 
{
	std::cerr << "Saving: checking params..." << std::endl;

	if(_old_port != ui.port_SB->value() || _old_mask != _current_mask)
	{
		std::cerr << "RsWebUIConfig::save() setting new port to " << ui.port_SB->value() << ", and mask = " << _current_mask << std::endl;

		RSWebUI::setPort(ui.port_SB->value()) ;
		RSWebUI::setIPMask(_current_mask) ;

		RSWebUI::restart() ;

		std::cerr << "Restarted Web UI." << std::endl;
	}
	else
		std::cerr << "No changes." << std::endl;

	return true;
}

void RsWebUIConfig::on_IPmaskChanged(const QString& IPmask)
{
	bool b ;

	uint32_t ipm = stringToIPmask(IPmask,b) ;
QColor color ;

	if(b)
	{
		_current_mask = ipm ;

		color = QApplication::palette().color(QPalette::Active,QPalette::Base) ;
	}
	else
		color = QApplication::palette().color(QPalette::Disabled,QPalette::Base) ;

	QPalette palette = ui.IPmask_LE->palette();
	palette.setColor(ui.IPmask_LE->backgroundRole(), color);
	ui.IPmask_LE->setPalette(palette);
}
void RsWebUIConfig::on_enableSwitch(bool b)
{
	ui.params_GB->setEnabled(b) ;
}

QString RsWebUIConfig::helpText() const
{
	int port = ui.port_SB->value() ;

	return QString("<p>This plugin provides a web interface to control Retroshare with your web browser. \
			          Once enabled, you can connect to Retroshare on the same machine, using \
						 <a href=\"http://localhost:%1\">http://localhost:%1</a></p> \
						 <p>The IP mask can be configured to allow only a restricted set of IP ranges. \
						 ").arg(port); 
}


