#pragma once
#include "network.h"
bool gameCiaInstalled();
bool installGameCia(const std::string& path,std::string& error,const Progress& progress);
bool prepareGameCiaLaunch(std::string& error);
