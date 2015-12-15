#pragma once

#include <retroshare/rsplugin.h>
#include "p3wallservice.h"
#include <gxs/rsgxsnetservice.h>
#include <gxs/rsdataservice.h>

#include "api/ApiServerMHD.h"
#include "api/WallHandler.h"
#include "api/RsControlModule.h"

class PluginGUIHandler ;
class PluginNotifier ;

class WebUIPlugin: public RsPlugin
{
	public:
		WebUIPlugin() ;
		virtual ~WebUIPlugin() {}

		virtual ConfigPage     *qt_config_page()        const ;
		virtual QDialog        *qt_about_page()         const ;
		
		virtual QIcon *qt_icon() const;
		//virtual QTranslator    *qt_translator(QApplication *app, const QString& languageCode, const QString& externalDir) const;

        virtual void getPluginVersion(int& major,int& minor,int& build, int &svn_rev) const ;
		virtual void setPlugInHandler(RsPluginHandler *pgHandler);

		virtual std::string configurationFileName() const { return "webuiplugin.cfg" ; }

		virtual std::string getShortPluginDescription() const ;
		virtual std::string getPluginName() const;
		virtual void setInterfaces(RsPlugInInterfaces& interfaces);
		virtual void stop();

        //virtual RsPQIService   *rs_pqi_service() 		const	{ return  ; }
        virtual p3Service       *p3_service() 		const	{ return wall_ns; }
        virtual p3Config       *p3_config() 		const	{ return wall_ns; }
        virtual uint16_t        rs_service_id() 	   const	{ return RsWall::RS_SERVICE_TYPE_WALL; }

        virtual resource_api::ResourceRouter* new_resource_api_handler(const RsPlugInInterfaces& ifaces, resource_api::StateTokenServer* sts, std::string &entrypoint) const;

	private:
		mutable RsPluginHandler *mPlugInHandler;
        mutable RsPlugInInterfaces plugin_interfaces;

        mutable RsGeneralDataService *wall_ds;
        mutable RsGxsNetService *wall_ns;
        mutable RsWall::p3WallService *wall;

        mutable resource_api::ApiServerMHD* httpd;
        mutable resource_api::RsControlModule* ctrlmodule;
        mutable resource_api::WallHandler* wallhandler;

		//mutable ConfigPage *config_page ;
		//mutable QIcon *mIcon;

		//PluginNotifier *mPluginNotifier ;
		//PluginGUIHandler *mPluginGUIHandler ;
};

