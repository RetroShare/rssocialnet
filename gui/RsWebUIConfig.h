#pragma once

#include <stdint.h>
#include <QWidget>

#include "retroshare-gui/configpage.h"
#include "ui_RsWebUIConfig.h"
#ifdef USE_OLD_WITTY_CODE
#include "WebUImain.h"
#endif

class RsWebUIConfig : public ConfigPage 
{
	Q_OBJECT

	public:
		/** Default Constructor */
		RsWebUIConfig(QWidget * parent = 0, Qt::WindowFlags flags = 0);
		/** Default Destructor */
		~RsWebUIConfig();

		/** Saves the changes on this page */
		virtual bool save(QString &errmsg);
		/** Loads the settings for this page */
		virtual void load();

		virtual QPixmap iconPixmap() const { return QPixmap(":/images/emblem-web.png") ; }
		virtual QString pageName() const { return tr("Web UI") ; }
		virtual QString helpText() const ;

	private slots:
		void on_IPmaskChanged(const QString& mask);
		void on_enableSwitch(bool b);

	private:
		Ui::RsWebUIConfig ui;

		void loadSettings();
#ifdef USE_OLD_WITTY_CODE
		std::vector<RSWebUI::IPRange> _current_ip_list ;
		std::vector<RSWebUI::IPRange> _old_ip_list ;
#endif
		uint16_t _old_port ;
		bool _ip_list_changed ;
};
