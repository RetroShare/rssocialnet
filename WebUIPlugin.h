#pragma once

#include <retroshare/rsplugin.h>

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

		virtual void getPluginVersion(int& major,int& minor,int& svn_rev) const ;
		virtual void setPlugInHandler(RsPluginHandler *pgHandler);

		virtual std::string configurationFileName() const { return "webuiplugin.cfg" ; }

		virtual std::string getShortPluginDescription() const ;
		virtual std::string getPluginName() const;
		virtual void setInterfaces(RsPlugInInterfaces& interfaces);
		virtual void stop();

	private:
		mutable RsPluginHandler *mPlugInHandler;
		mutable RsPlugInInterfaces plugin_interfaces ;

		//mutable ConfigPage *config_page ;
		//mutable QIcon *mIcon;

		//PluginNotifier *mPluginNotifier ;
		//PluginGUIHandler *mPluginGUIHandler ;
};

