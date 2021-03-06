#include "scalarcomposition.h"

void ScalarComposition::ApplyComposition(std::string moduleName,
																				 int compositionNumber,
																				 std::map<specie, int> &concOut,
																				 std::vector<reaction> &reactionOut,
																				 std::vector<specie> &specieOut) {
	std::vector<reaction> preRates;
	for (Composition *subcomp : subCompositions) {
		subcomp->ApplyComposition(moduleName, compositionNumber, concOut, preRates,
															specieOut);
		compositionNumber++;
	}
	for (auto reaction : preRates) {
		reaction.rate *= scale;
		reactionOut.push_back(reaction);
	}
}
