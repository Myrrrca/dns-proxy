ifeq ($(origin CC), default)
	CC = gcc
endif

TARGET = proxy
# CFLAGS ?= -O2
COMMONINC = -I./lib/errproc -I./lib/ini -I./src
OBJDIR_PREF = ./obj

override CFLAGS += $(COMMONINC)

CSRC = src/proxy.c src/config.c src/dns.c lib/ini/ini.c lib/errproc/errproc.c
COBJ = $(addprefix $(OBJDIR_PREF)/, $(CSRC:.c=.o))

.PHONY: all
all : $(TARGET)

$(TARGET) : $(COBJ) 
	$(CC) $^ -o $@ 

$(COBJ) : $(OBJDIR_PREF)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean :
	rm -rf $(TARGET) $(COBJ)



