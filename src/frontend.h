#include "driver.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

class Frontend {
public:
	driver *drv;
	std::stringstream stream;
	void GenerateStringStream();
	void WriteFile();
	std::string outputFileName = "out.crn";

private:
	std::ofstream fileStream;
};
