// globals.h
#ifndef ASTRAL_GLOBALS_H
#define ASTRAL_GLOBALS_H

#include <string>

extern bool DEBUG;
extern bool SILENCE;

extern const std::string astral_name = "The Astral Programming Language";
extern const std::string astral_version = "1.3.5";
extern const std::string astral_arg = "astral";

#include <unordered_map>
#include <stack>
#include <memory>
#include <string>
#include <mutex>
#include "logging/logger.h"
#include "concurrency/task.h"
#include "objects/method.h"
#include "objects/module.h"
#include "objects/class.h"
#include "stackframe.h"
#include "parsing/tokens.h"
#include "web/httplib.h"

extern Logger logger;
extern TaskManager task;
extern std::unordered_map<std::string, Method> methods;
extern std::unordered_map<std::string, Module> modules;
extern std::unordered_map<std::string, Class> classes;
extern std::unordered_map<std::string, std::string> astralArgs;
extern std::stack<std::shared_ptr<CallStackFrame>> callStack;
extern std::stack<k_stream> streamStack;
extern std::stack<std::string> moduleStack;
extern httplib::Server astralWebServer;
extern std::unordered_map<int, Method> astralWebServerHooks;
extern std::string astralWebServerHost;
extern k_int astralWebServerPort;

#endif