#SUBDIRS=AddOn-BeIDE ClassStructure Extra HandlerAddOn SharedAddOn
SUBDIRS= ClassStructure SharedAddOn AddOn-BeIDE HandlerAddOn

default all clean:
	for d in $(SUBDIRS); do \
		make -C $$d $@ || exit 1; \
	done


