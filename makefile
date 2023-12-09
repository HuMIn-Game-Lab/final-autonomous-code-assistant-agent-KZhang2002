# Makefile for final_autonomous_code_assistant_agent_KZhang2002

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20
LDFLAGS = -pthread

# Directories
INCLUDE_DIR = Code/lib/curl/include
LIB_DIR = Code/lib/curl/bin

# Source files
JOB_SYSTEM_SRC = \
    Code/JobSystem/JobWorkerThread.cpp \
    Code/JobSystem/JobSystem.cpp \
    Code/JobSystem/DefaultJobFactory.cpp \
    Code/JobSystem/CompileJob.cpp \
    Code/JobSystem/RenderJob.cpp \
    Code/JobSystem/ChatJob.cpp \
    Code/JobSystem/FlowScriptInterpreter.cpp

MAIN_SRC = \
    Code/main.cpp \
    Code/CustomJob.cpp \
    Code/CustomJobFactory.cpp

# Target names
TARGET_LIB = libJobSystem.so
TARGET_EXEC = final_autonomous_code_assistant_agent_KZhang2002

compile: $(TARGET_LIB) $(TARGET_EXEC)

$(TARGET_LIB): $(JOB_SYSTEM_SRC)
	$(CXX) -shared -o $@ $^ -I$(INCLUDE_DIR) -L$(LIB_DIR) -lcurl -fPIC

$(TARGET_EXEC): $(MAIN_SRC)
	$(CXX) -o $@ $^ -I$(INCLUDE_DIR) -ICode/JobSystem -I. -L. -lJobSystem -lcurl $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(TARGET_LIB) $(TARGET_EXEC)
