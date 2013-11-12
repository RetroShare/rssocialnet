#include <QIcon>
#include <QMessageBox>

#include <retroshare/rsplugin.h>
#include <util/rsversion.h>

#include "WebUIPlugin.h"
#include "WebUImain.h"

static void *inited = new WebUIPlugin() ;

extern "C" {
	// This is *the* functions required by RS plugin system to give RS access to the plugin.
	// Be careful to:
	// - always respect the C linkage convention
	// - always return an object of type RsPlugin*
	//
	void *RETROSHARE_PLUGIN_provide()
	{
		static WebUIPlugin *p = new WebUIPlugin() ;

		return (void*)p ;
	}

	// This symbol contains the svn revision number grabbed from the executable. 
	// It will be tested by RS to load the plugin automatically, since it is safe to load plugins
	// with same revision numbers, assuming that the revision numbers are up-to-date.
	//
	uint32_t RETROSHARE_PLUGIN_revision = SVN_REVISION_NUMBER ;

	// This symbol contains the svn revision number grabbed from the executable. 
	// It will be tested by RS to load the plugin automatically, since it is safe to load plugins
	// with same revision numbers, assuming that the revision numbers are up-to-date.
	//
	uint32_t RETROSHARE_PLUGIN_api = RS_PLUGIN_API_VERSION ;
}

QIcon *WebUIPlugin::qt_icon() const
{
	static QIcon *icon = new QIcon(":images/emblem-web.png") ;

	return icon ;
}

void WebUIPlugin::getPluginVersion(int& major,int& minor,int& svn_rev) const
{
	major = 5 ;
	minor = 4 ;
	svn_rev = SVN_REVISION_NUMBER ;
}

QDialog *WebUIPlugin::qt_about_page() const
{
	static QMessageBox *about_dialog = NULL ;

	if(about_dialog == NULL)
	{
		about_dialog = new QMessageBox() ;

		QString text ;
		text += QObject::tr("<h3>RetroShare WebUI plugin</h3>* Contributors: Cyril Soler<br/>") ;
		text += QObject::tr("<p>The WebUI plugin provides a web interface to Retroshare.") ;
		text += QObject::tr("It is easily configurable from the Config->WebUI page, and is based on Wt (Witty)</p>.") ;

		about_dialog->setText(text) ;
		about_dialog->setStandardButtons(QMessageBox::Ok) ;
	}

	return about_dialog ;
}

WebUIPlugin::WebUIPlugin()
{
	mPlugInHandler = NULL;
	mPeers = NULL;

	RSWebUI::start() ;
}

void WebUIPlugin::setInterfaces(RsPlugInInterfaces &interfaces)
{
    mPeers = interfaces.mPeers;
}

void WebUIPlugin::setPlugInHandler(RsPluginHandler *pgHandler)
{
    mPlugInHandler = pgHandler;
}

std::string WebUIPlugin::getShortPluginDescription() const
{
	return std::string("This plugin provides a WebUI to retroshare.") ;
}

std::string WebUIPlugin::getPluginName() const
{
	return std::string("WebUI Plugin");
}

