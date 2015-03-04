TEMPLATE = subdirs

SUBDIRS +=	\
	resapi	\
	socialnet

resapi.file = resapi.pro
socialnet.file = socialnet.pro
socialnet.depends = resapi
