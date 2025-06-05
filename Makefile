all:
.SILENT:
.SECONDARY:
PRECMD=echo "  $@" ; mkdir -p "$(@D)" ;

ifneq (,$(strip $(filter clean,$(MAKECMDGOALS))))
clean:;rm -rf mid out
else

OPT_ENABLE:=stdlib graf text rom
PROJNAME:=deadweight
PROJRDNS:=com.aksommerville.egggame.deadweight
ENABLE_SERVER_AUDIO:=
BUILDABLE_DATA_TYPES:=
NON_SRC_FILES:=src/validator/%

ifndef EGG_SDK
  EGG_SDK:=/home/andy/proj/egg
endif

include $(EGG_SDK)/etc/tool/common.mk

VALIDATOR_CC:=gcc -c -MMD -O2 -Isrc -Werror -Wimplicit
VALIDATOR_LD:=gcc
VALIDATOR_LDPOST:=-lz
VALIDATOR_EXE:=out/validator
all:$(VALIDATOR_EXE)
VALIDATOR_CFILES:=$(filter src/validator/%.c,$(SRCFILES))
VALIDATOR_OFILES:=$(patsubst src/%.c,mid/validator/%.o,$(VALIDATOR_CFILES))
-include $(VALIDATOR_OFILES:.o=.d)
mid/validator/%.o:src/%.c;$(PRECMD) $(VALIDATOR_CC) -o$@ $<
$(VALIDATOR_EXE):$(VALIDATOR_OFILES);$(PRECMD) $(VALIDATOR_LD) -o$@ $(VALIDATOR_OFILES) $(VALIDATOR_LDPOST)
validate:$(VALIDATOR_EXE);$(VALIDATOR_EXE) --palette=etc/nes-palette.png src/data/image

endif
