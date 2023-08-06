#pragma once

#include "client/crashpad_client.h"

class Crashpad {
  private:
	std::map<std::string, std::string> _annotations;
	std::unique_ptr<crashpad::CrashpadClient> clientHandler;

	std::string getExecutableDir();

  public:
	Crashpad(const std::string &serverPath = "", const std::string &serverProxy = "",
			 const std::string &crashpadHandlerPath = "", const std::map<std::string, std::string> &annotations = {},
			 const std::vector<base::FilePath> &attachments = {});
};
