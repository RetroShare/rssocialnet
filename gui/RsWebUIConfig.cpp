#include <QTimer>
#include "RsWebUIConfig.h"

RsWebUIConfig::RsWebUIConfig(QWidget * parent, Qt::WindowFlags flags)
    : ConfigPage(parent, flags)
{
    /* Invoke the Qt Designer generated object setup routine */
    ui.setupUi(this);
}

RsWebUIConfig::~RsWebUIConfig() {}

/** Loads the settings for this page */
void RsWebUIConfig::load()
{
    loadSettings();
}


void RsWebUIConfig::loadSettings() 
{
}

bool RsWebUIConfig::save(QString &/*errmsg*/) 
{
	return true;
}

void RsWebUIConfig::on_portChanged(int port)
{
}
void RsWebUIConfig::on_enableSwitch(bool b)
{
	ui.params_GB->setEnabled(b) ;
}

