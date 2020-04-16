TARGET            =    AFE_Parser
OUT               =    out
BUILD             =    build
SOURCES           =    AFE_Parser
INCLUDES          =    include
CUSTOM_LIBS       =

DEFINES           =    __STDC_FORMAT_MACROS
ARCH              =    -march=native
FLAGS             =    -Wall -Wextra -pipe
CFLAGS            =    -std=gnu11
CXXFLAGS          =    -std=gnu++17
ASFLAGS           =
LDFLAGS           =
LINKS             =

RELEASE_DEFINES   =    $(DEFINES) NDEBUG=1
RELEASE_FLAGS     =    $(FLAGS) -O2 -ffunction-sections -fdata-sections -flto
RELEASE_CFLAGS    =    $(CFLAGS)
RELEASE_CXXFLAGS  =    $(CXXFLAGS)
RELEASE_ASFLAGS   =    $(ASFLAGS)
RELEASE_LDFLAGS   =    $(LDFLAGS) -Wl,--gc-sections -flto -fuse-linker-plugin -s

DEBUG_DEFINES     =    $(DEFINES) DEBUG=1
DEBUG_FLAGS       =    $(FLAGS) -O0 -g3 -ggdb
DEBUG_CFLAGS      =    $(CFLAGS)
DEBUG_CXXFLAGS    =    $(CXXFLAGS)
DEBUG_ASFLAGS     =    $(ASFLAGS) -g
DEBUG_LDFLAGS     =    $(LDFLAGS) -g -ggdb -Wl,-Map,$(BUILD)/$(TARGET)-dbg.map -fno-lto

PREFIX            =
CC                =    $(PREFIX)gcc
CXX               =    $(PREFIX)g++
AS                =    $(PREFIX)as
LD                =    $(PREFIX)g++

# -----------------------------------------------

# English error messages
export LANG=en

LIBS              =    $(CUSTOM_LIBS)

CFILES            =    $(shell find $(SOURCES) -name *.c)
CPPFILES          =    $(shell find $(SOURCES) -name *.cpp)
SFILES            =    $(shell find $(SOURCES) -name *.s -or -name *.S)

RELEASE_OFILES    =    $(CFILES:%=$(BUILD)/%-rel.o) $(CPPFILES:%=$(BUILD)/%-rel.o) $(SFILES:%=$(BUILD)/%-rel.o)
DEBUG_OFILES      =    $(CFILES:%=$(BUILD)/%-dbg.o) $(CPPFILES:%=$(BUILD)/%-dbg.o) $(SFILES:%=$(BUILD)/%-dbg.o)
DFILES            =    $(RELEASE_OFILES:.o=.d) $(DEBUG_OFILES:.o=.d)

LIBS_TARGET       =    $(shell find $(addsuffix /lib,$(CUSTOM_LIBS)) -name "*.a" 2>/dev/null)
RELEASE_TARGET    =    $(if $(OUT:=), $(OUT)/$(TARGET), .$(OUT)/$(TARGET))
DEBUG_TARGET      =    $(if $(OUT:=), $(OUT)/$(TARGET)-dbg, .$(OUT)/$(TARGET)-dbg)

REL_DEFINES_FLAGS =    $(addprefix -D,$(RELEASE_DEFINES))
DBG_DEFINES_FLAGS =    $(addprefix -D,$(DEBUG_DEFINES))

INCLUDE_FLAGS     =    $(addprefix -I$(CURDIR)/,$(INCLUDES)) $(foreach dir,$(CUSTOM_LIBS),-I$(CURDIR)/$(dir)/include) \
                       $(foreach dir,$(filter-out $(CUSTOM_LIBS),$(LIBS)),-I$(dir)/include)
LIB_FLAGS         =    $(foreach dir,$(LIBS),-L$(dir)/lib)

# -----------------------------------------------

.SUFFIXES:

.PHONY: all libs release debug run clean mrproper $(CUSTOM_LIBS)

all: release debug

libs: $(CUSTOM_LIBS)

$(CUSTOM_LIBS):
	@$(MAKE) -s --no-print-directory -C $@ -f Makefile

release: $(RELEASE_TARGET)

debug: $(DEBUG_TARGET)

run: debug
	@echo "Running" $(DEBUG_TARGET)
	@$(DEBUG_TARGET) roms/PONG.ch8

$(RELEASE_TARGET): $(RELEASE_OFILES) $(LIBS_TARGET) | libs
	@echo " LD  " $@
	@mkdir -p $(dir $@)
	@$(LD) $(ARCH) $(RELEASE_LDFLAGS) $(LIB_FLAGS) $(RELEASE_OFILES) -o $@ $(LINKS)
	@echo "Built" $(notdir $@)

$(DEBUG_TARGET): $(DEBUG_OFILES) $(LIBS_TARGET) | libs
	@echo " LD  " $@
	@mkdir -p $(dir $@)
	@$(LD) $(ARCH) $(DEBUG_LDFLAGS) $(LIB_FLAGS) $(DEBUG_OFILES) -o $@ $(LINKS)
	@echo "Built" $(notdir $@)

$(BUILD)/%.c-rel.o: %.c
	@echo " CC  " $@
	@mkdir -p $(dir $@)
	@$(CC) -MMD -MP $(ARCH) $(RELEASE_FLAGS) $(RELEASE_CFLAGS) $(REL_DEFINES_FLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.c-dbg.o: %.c
	@echo " CC  " $@
	@mkdir -p $(dir $@)
	@$(CC) -MMD -MP $(ARCH) $(DEBUG_FLAGS) $(DEBUG_CFLAGS) $(DBG_DEFINES_FLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.cpp-rel.o: %.cpp
	@echo " CXX " $@
	@mkdir -p $(dir $@)
	@$(CXX) -MMD -MP $(ARCH) $(RELEASE_FLAGS) $(RELEASE_CXXFLAGS) $(REL_DEFINES_FLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.cpp-dbg.o: %.cpp
	@echo " CXX " $@
	@mkdir -p $(dir $@)
	@$(CXX) -MMD -MP $(ARCH) $(DEBUG_FLAGS) $(DEBUG_CXXFLAGS) $(DBG_DEFINES_FLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.s-rel.o: %.s %.S
	@echo " AS  " $@
	@mkdir -p $(dir $@)
	@$(AS) -MMD -MP -x assembler-with-cpp $(ARCH) $(RELEASE_FLAGS) $(RELEASE_ASFLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

$(BUILD)/%.s-dbg.o: %.s %.S
	@echo " AS  " $@
	@mkdir -p $(dir $@)
	@$(AS) -MMD -MP -x assembler-with-cpp $(ARCH) $(DEBUG_FLAGS) $(DEBUG_ASFLAGS) $(INCLUDE_FLAGS) -c $(CURDIR)/$< -o $@

clean:
	@echo Cleaning...
	@rm -rf $(BUILD) $(OUT)

mrproper: clean
	@for dir in $(CUSTOM_LIBS); do $(MAKE) clean --no-print-directory -C $$dir -f Makefile; done

-include $(DFILES)
