VPATH = /root/distr/httpd-2.2.3/srclib/apr-util/test

INCLUDES = -I/root/distr/httpd-2.2.3/srclib/apr-util/include -I/root/distr/httpd-2.2.3/srclib/apr-util/include/private  -I/root/distr/httpd-2.2.3/srclib/apr/include  -I/root/distr/httpd-2.2.3/srclib/apr-util/xml/expat/lib

PROGRAMS = testall testdbm testdate testxml testrmm \
	   testreslist testqueue testxlate dbd
TARGETS = $(PROGRAMS)

APRUTIL_DOTTED_VERSION=1.2.7
APRUTIL_MAJOR_VERSION=1
TARGET_LIB_PATH = ../libaprutil-${APRUTIL_MAJOR_VERSION}.la

CLEAN_TARGETS = manyfile.bin testfile.txt data/sqlite*.db

# bring in rules.mk for standard functionality
include /root/distr/httpd-2.2.3/srclib/apr-util/build/rules.mk
PROGRAM_DEPENDENCIES = /root/distr/httpd-2.2.3/srclib/apr-util/xml/expat/lib/libexpat.la /root/distr/httpd-2.2.3/srclib/apr/libapr-1.la -lrt -lcrypt -lpthread -ldl
APRUTIL_LDFLAGS = -no-install -L/root/distr/httpd-2.2.3/srclib/apr-util/xml/expat/lib

all: $(PROGRAMS)

check: $(PROGRAMS)
	for prog in $(PROGRAMS); do \
		./$$prog ;\
		if test $$? = 255; then \
			echo "$$prog failed"; \
			break; \
		fi; \
	done

testdbm_OBJECTS = testdbm.lo
testdbm_LDADD =  $(TARGET_LIB_PATH)
testdbm: $(testdbm_OBJECTS) $(testdbm_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testdbm_OBJECTS) $(testdbm_LDADD) $(PROGRAM_DEPENDENCIES)

dbd_OBJECTS = dbd.lo
dbd_LDADD =  $(TARGET_LIB_PATH)
dbd: $(dbd_OBJECTS) $(dbd_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(dbd_OBJECTS) $(dbd_LDADD) $(PROGRAM_DEPENDENCIES)

testdbd_OBJECTS = testdbd.lo
testdbd_LDADD =  $(TARGET_LIB_PATH)
testdbd: $(testdbd_OBJECTS) $(testdbd_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testdbd_OBJECTS) $(testdbd_LDADD) $(PROGRAM_DEPENDENCIES)

testdate_OBJECTS = testdate.lo
testdate_LDADD =  $(TARGET_LIB_PATH)
testdate: $(testdate_OBJECTS) $(testdate_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testdate_OBJECTS) $(testdate_LDADD) $(PROGRAM_DEPENDENCIES)

testxml_OBJECTS = testxml.lo
testxml_LDADD =  $(TARGET_LIB_PATH)
testxml: $(testxml_OBJECTS) $(testxml_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testxml_OBJECTS) $(testxml_LDADD) $(PROGRAM_DEPENDENCIES)

testrmm_OBJECTS = testrmm.lo
testrmm_LDADD =  $(TARGET_LIB_PATH)
testrmm: $(testrmm_OBJECTS) $(testrmm_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testrmm_OBJECTS) $(testrmm_LDADD) $(PROGRAM_DEPENDENCIES)

testreslist_OBJECTS = testreslist.lo
testreslist_LDADD =  $(TARGET_LIB_PATH)
testreslist: $(testreslist_OBJECTS) $(testreslist_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testreslist_OBJECTS) $(testreslist_LDADD) $(PROGRAM_DEPENDENCIES)

testqueue_OBJECTS = testqueue.lo
testqueue_LDADD =  $(TARGET_LIB_PATH)
testqueue: $(testqueue_OBJECTS) $(testqueue_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testqueue_OBJECTS) $(testqueue_LDADD) $(PROGRAM_DEPENDENCIES)

testxlate_OBJECTS = testxlate.lo
testxlate_LDADD =  $(TARGET_LIB_PATH)
testxlate: $(testxlate_OBJECTS) $(testxlate_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testxlate_OBJECTS) $(testxlate_LDADD) $(PROGRAM_DEPENDENCIES)

testall_OBJECTS = teststrmatch.lo testuri.lo testuuid.lo abts.lo testutil.lo \
	testbuckets.lo testpass.lo testmd4.lo testmd5.lo testldap.lo testdbd.lo
testall_LDADD =  $(TARGET_LIB_PATH)
testall: $(testall_OBJECTS) $(testall_LDADD)
	$(LINK) $(APRUTIL_LDFLAGS) $(testall_OBJECTS) $(testall_LDADD) $(PROGRAM_DEPENDENCIES)

