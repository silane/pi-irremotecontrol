CXXFLAGS = -Wall -lwiringPi -std=c++11

.PHONY: all
all: ir-send ir-record

ir-send: ir_send.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

ir-record: ir_record.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) ir-send ir-record
