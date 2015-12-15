#include <QIcon>
#include <QMessageBox>

#include <retroshare/rsplugin.h>
#include <retroshare/rsversion.h>
#include <util/rsdir.h>

#include "WebUIPlugin.h"
#ifdef USE_OLD_WITTY_CODE
#include "WebUImain.h"
#endif
#ifdef WITH_GUI
#include <gui/RsWebUIConfig.h>
#endif

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
#ifdef WITH_GUI
RsWebUIConfig	*cfg_widget = new RsWebUIConfig ;

	return cfg_widget ;
#else
    return 0;
#endif
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
    httpd = 0;
    ctrlmodule = 0;
    wallhandler = 0;
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
    //std::string dataDir = "rssocialnet_v1";
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

#ifdef USE_OLD_WITTY_CODE
	std::cerr << "Starting the WebUI" << std::endl;
    RSWebUI::start(interfaces) ;
#endif
#if 0
    httpd = new resource_api::ApiServerMHD("./", 9090);
    httpd->getApiServer().loadMainModules(interfaces);

    ctrlmodule = new resource_api::RsControlModule(0, 0, httpd->getApiServer().getStateTokenServer(),
                                                   &httpd->getApiServer(), false);
    httpd->getApiServer().addResourceHandler("control", dynamic_cast<resource_api::ResourceRouter*>(ctrlmodule), &resource_api::RsControlModule::handleRequest);

    wallhandler = new resource_api::WallHandler(wall, interfaces.mIdentity);
    httpd->getApiServer().addResourceHandler("wall", dynamic_cast<resource_api::ResourceRouter*>(wallhandler),
                                                           &resource_api::WallHandler::handleRequest);

    httpd->start();
#endif
}

void WebUIPlugin::stop()
{   
#if 0
    httpd->stop();
    delete httpd;
    httpd = 0;

    delete ctrlmodule;
    ctrlmodule = 0;
    delete wallhandler;
    wallhandler = 0;
#endif
    // shutdown in reverse order
#ifdef USE_OLD_WITTY_CODE
	std::cerr << "Stopping the WebUI" << std::endl;
	RSWebUI::stop();
#endif

    wall_ns->join();
    wall->join();

    // wall_ns, wall_ds owned by wall (RsGenExchange)
    // delete wall_ns;
    delete wall;
    // delete wall_ds;
}

resource_api::ResourceRouter* WebUIPlugin::new_resource_api_handler(
        const RsPlugInInterfaces& interfaces,
        resource_api::StateTokenServer *sts,
        std::string &entrypoint) const
{
    std::cerr << "rssocialnet plugin: creating new WallHandler" << std::endl;
    entrypoint = "rssocialnet";
    return new resource_api::WallHandler(sts, wall, interfaces.mIdentity);
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

