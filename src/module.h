#pragma once
#include "typedefs.h"
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class Module;
class Composition;

namespace precision {
std::string to_string(double d);
}

struct FunctionIncorrectReactionsException : public std::exception {
	std::string error;
	FunctionIncorrectReactionsException(std::string moduleName)
			: error("Mistake in function " + moduleName + ". Input species must be " +
							"present on each side of the reactionarrow") {}
	const char *what() const throw() {
		return error.c_str();
	}
};

struct SpecieNotDeclaredException : public std::exception {
	std::string error;
	SpecieNotDeclaredException(std::string speciesName, std::string moduleName)
			: error("The species " + speciesName + " was not declared in module " +
							moduleName) {}
	const char *what() const throw() {
		return error.c_str();
	}
};

struct InputSpecieConcException : public std::exception {
	std::string error;
	InputSpecieConcException(std::string speciesName, std::string moduleName)
			: error("Illegal use of input species " + speciesName + ", in module " +
							moduleName + ". Cannot set concentration of input species") {}
	const char *what() const throw() {
		return error.c_str();
	}
};

struct MapConcForSubModuleException : public std::exception {
	std::string error;
	MapConcForSubModuleException(std::string speciesName,
															 std::string moduleNameFrom,
															 std::string moduleNameTo)
			: error("Something went wrong, couldn't map specie: " + speciesName +
							" from module " + moduleNameFrom + " to module " + moduleNameTo +
							"'s concentrations") {}
	const char *what() const throw() {
		return error.c_str();
	}
};

struct NoSuchModuleException : public std::exception {
	std::string error;
	NoSuchModuleException(std::string moduleName)
			: error(
						"You tried to create a composition with module '" + moduleName +
						"', but no such module has been defined. Maybe it's defined below "
						"the current module?") {}
	const char *what() const throw() {
		return error.c_str();
	}
};

class Module {
public:
	Module() {}
	void Verify();
	void VerifyFunction();
	std::string OutputSpecieToString(const std::string &speciesName);
	std::string SpecieConcsTostring(const std::pair<std::string, int> &concs);
	std::string SpecieCoefToString(int coeff);
	std::string SpecieReactToString(const std::pair<std::string, int> &specie);
	std::string Compile();
	/**
	 * Remove all compositions from the vector, and add items to the object
	 *
	 * During compilations, compositions are added as items in the compositions
	 * vector. Then, after this is done, this function should be called, and each
	 * submodules reactions, compositions and private species should be added to
	 * the supermodule. This function does that action, and pops the modules off
	 * the vector
	 */
	void ApplyCompositions();
	std::string name;
	std::vector<specie> inputSpecies;
	std::vector<specie> outputSpecies;
	std::vector<specie> privateSpecies;
	std::map<specie, int> concentrations;
	std::vector<reaction> reactions;
	// TODO: This should be an unique pointer instead
	std::vector<Composition *> compositions;
};
