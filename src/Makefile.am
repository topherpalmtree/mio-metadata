bin_PROGRAMS = mio_meta_tool meta_publish
mio_meta_tool_SOURCES = meta_tool.c 
mio_meta_tool_CPPFLAGS = -Wall -I./src/ 
mio_meta_tool_LDADD = -lmio_agent -lmio_xml -lmio -lstrophe -lexpat -lssl -lpthread -luuid -lresolv
mio_meta_tool_ARFLAGS = rcs

meta_publish_SOURCES = meta_publish.c 
meta_publish_CPPFLAGS = -Wall -I./src/ 
meta_publish_LDADD = -lmio_agent -lmio_xml -lmio -lstrophe -lexpat -lssl -lpthread -luuid -lresolv
meta_publish_ARFLAGS = rcs
