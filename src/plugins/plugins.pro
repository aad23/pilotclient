load(common_pre)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += simulator
SUBDIRS += weatherdata

load(common_post)
