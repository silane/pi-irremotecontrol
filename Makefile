CXXFLAGS = -Wall -std=c++11
LIBS = -lwiringPi

.PHONY: all
all: ir-send ir-record

ir-send: ir_send.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

ir-record: ir_record.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	$(RM) ir-send ir-record
