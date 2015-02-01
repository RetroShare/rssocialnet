#include <QIcon>
#include <QMessageBox>

#include <retroshare/rsplugin.h>
#include <retroshare/rsversion.h>
#include <util/rsdir.h>

#include "WebUIPlugin.h"
#include "WebUImain.h"
#include <gui/RsWebUIConfig.h>

#include "p3wallservice.h"

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
    uint32_t RETROSHARE_PLUGIN_revision = RS_REVISION_NUMBER ;

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

void WebUIPlugin::getPluginVersion(int& major,int& minor,int& build,int& svn_rev) const
{
    major = 0 ;
    minor = 0 ;
    svn_rev = RS_REVISION_NUMBER ;
}

ConfigPage *WebUIPlugin::qt_config_page() const
{
//	static RsWebUIConfig *cfg_widget = NULL ;
	
//	if(cfg_widget == NULL)
RsWebUIConfig	*cfg_widget = new RsWebUIConfig ;

	return cfg_widget ;
}

QDialog *WebUIPlugin::qt_about_page() const
{
	static QMessageBox *about_dialog = NULL ;

	if(about_dialog == NULL)
	{
		about_dialog = new QMessageBox() ;

		QString text ;
        text += QObject::tr("<h3>RetroShare Social network plugin, based on the WebUI plugin</h3>* Contributors: Cyril Soler<br/>") ;
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
}

void WebUIPlugin::setInterfaces(RsPlugInInterfaces &interfaces)
{
	std::cerr << "Setting plugin interfaces for WebUI plugin..." << std::endl;
	plugin_interfaces = interfaces ;

    // TODO: change data directory
    // it would be better to save data for this plugin in its own folder
    // (like zeroreserve: <sslid>/zeroreserve)
    // idea: use different folders for different plugin versions
    std::cerr << "Starting p3WallService" << std::endl;
    std::string dataDir = interfaces.mGxsDir + "rssocialnet_v0";
    RsDirUtil::checkCreateDirectory(dataDir);
    wall_ds = new RsDataService(dataDir, "wall_db",
                                RsWall::RS_SERVICE_TYPE_WALL, NULL, "todo: encrypt db with secure password");
    wall = new RsWall::p3WallService(wall_ds, NULL, interfaces.mGxsIdService, interfaces.mIdentity);
    wall_ns = new RsGxsNetService(
                RsWall::RS_SERVICE_TYPE_WALL, wall_ds, interfaces.mRsNxsNetMgr,
                wall, wall->getServiceInfo(),
                interfaces.mGxsIdService, interfaces.mGxsCirlces, interfaces.mPgpAuxUtils,
                true    // group auto sync
            );
    RsWall::rsWall = wall;

    wall->start();
    wall_ns->start();

	std::cerr << "Starting the WebUI" << std::endl;
    RSWebUI::start(interfaces) ;
}

void WebUIPlugin::stop()
{
    // shutdown in reverse order

	std::cerr << "Stopping the WebUI" << std::endl;
	RSWebUI::stop();

    wall_ns->join();
    wall->join();

    // wall_ns, wall_ds owned by wall (RsGenExchange)
    // delete wall_ns;
    delete wall;
    // delete wall_ds;
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

