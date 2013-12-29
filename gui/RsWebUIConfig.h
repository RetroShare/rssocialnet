#ifndef _AUDIOINPUTCONFIG_H
#define _AUDIOINPUTCONFIG_H

#include <stdint.h>
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
		virtual QString helpText() const ;

	private slots:
		void on_IPmaskChanged(const QString& mask);
		void on_enableSwitch(bool b);

	private:
		Ui::RsWebUIConfig ui;

		void loadSettings();

		uint32_t _current_mask ;
		uint32_t _old_mask ;
		uint16_t _old_port ;
};

#endif
