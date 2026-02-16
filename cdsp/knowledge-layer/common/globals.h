#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H
#include <string>

const std::string &getProjectRoot();
bool setProjectRoot(const std::string &root);
const std::string &getJsonRpcVersion();
void setPathToUseCases(const std::string &path);
const std::string &getPathToUseCases();

#endif  // GLOBALS_H