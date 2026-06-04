#Compiler and Linker
CXX          := clang++-22

#The Target Binary Program
TARGET      := compiler

#The Directories, Source, Includes, Objects, and Binary
SRCDIR      := src
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin
SRCEXT      := cpp
DEPEXT      := d
OBJEXT      := o

#Flags, Libraries and Includes
CFLAGS      := -stdlib=libc++ -std=c++23 -Wall -Wextra -Werror
LFLAGS      := -stdlib=libc++
LIB         := -lm
INC         := -I$(INCDIR)

SOURCES     := $(wildcard $(SRCDIR)/*.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Default Make
all: directories $(TARGET)

#Remake
remake: clean all

#Make the Directories
directories:
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)

#Full Clean, Objects and Binaries
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	$(CXX) $(LIB) $(LFLAGS) -o $(TARGETDIR)/$(TARGET) $^

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

#Non-File Targets
.PHONY: all remake clean