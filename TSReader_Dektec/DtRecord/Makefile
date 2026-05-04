# ========================================================================================
# Makefile for the DtRecord (LINUX)
# ========================================================================================


# ========================================================================================
# Set include directories. Make sure the include path to the DTAPI is correct

DTAPIINCLUDE = ../DTAPI/Include
INCLUDEDIR = /usr/local/include -I$(DTAPIINCLUDE)

# ========================================================================================
# Compile flags

CPPFLAGS = -g -O2 -I$(INCLUDEDIR) -fexceptions

# ========================================================================================
# TARGET/Source definitions

SRCDIR = ./
TARGET = DtRecord
OBJS = $(TARGET).o
SRC = $(SRCDIR)DtRecord.cpp

# ========================================================================================
# Make all option. Make sure the path to DTPAI.o is set correct

DTAPILIB = ../DTAPI/DTAPI.o
LDLIBS += $(DTAPILIB)

all: $(TARGET)
$(TARGET): $(SRC:.cpp=.o)
		$(CXX) $(CPPFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# ========================================================================================
# Clean all option

clean:
	rm -f $(SRCDIR)*.o $(SRCDIR)*~ $(SRCDIR)core $(SRCDIR).depend
	rm -f *.o *~ core .depend $(TARGET)


