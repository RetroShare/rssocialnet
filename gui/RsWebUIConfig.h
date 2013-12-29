#ifndef _AUDIOINPUTCONFIG_H
#define _AUDIOINPUTCONFIG_H

#include <QWidget>

#include "retroshare-gui/configpage.h"
#include "ui_RsWebUIConfig.h"

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
		virtual QString helpText() const { return "Provides a web interface to control Retroshare with your web browser"; }

	private slots:
		void on_portChanged(int port);
		void on_enableSwitch(bool b);

	private:
		Ui::RsWebUIConfig ui;

		void loadSettings();
};

#endif
