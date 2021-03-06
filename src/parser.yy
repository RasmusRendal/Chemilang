%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.4.2"
%defines

%define api.token.constructor
%define api.value.type variant

%code requires {
  #include <string>
  #include <map>
  #include <vector>
  #include <exception>
  #include <assert.h>
  #include "module.h"
  #include "modulecomposition.h"
  #include "conditionalcomposition.h"
  #include "scalarcomposition.h"
  class driver;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
# include "driver.h"

template <class T>
void MergeVectors(std::vector<T> &v1, const std::vector<T> &v2) {
	for (const auto &i : v2) {
		v1.push_back(i);
	}
}

Composition *MakeComposition(driver &drv, const std::string &moduleName, std::vector<specie> inputs, std::vector<specie> outputs) {
	if (drv.modules.find(moduleName) == drv.modules.end()) {
		throw NoSuchModuleException(moduleName);
	}
	Module *module = &drv.modules.at(moduleName);
	return new ModuleComposition(module, inputs, outputs);
}

void InsertToSpecieMap(speciesRatios &ratio, std::pair<specie, int> &toInsert) {
	if (ratio.find(toInsert.first) == ratio.end()) {
		ratio.insert(toInsert);
	} else {
		ratio[toInsert.first] += toInsert.second;
	}
  }
}

%define api.token.prefix {TOK_}
%token
    END  0               "end of file"
    T_DMODULE            "module"
    T_DSCALE             "scale"
    T_DFUNCTION          "function"
    T_DPRIVATE           "private:"
    T_DINPUT             "input:"
    T_DOUTPUT            "output:"
    T_DREACTIONS         "reactions:"
    T_DCONCENTRATIONS    "concentrations:"
    T_DCOMPOSITIONS      "compositions:"
    T_DIF                "if"
    T_RIGHTARROW         "->"
    T_BIARROW            "<->"
    T_BRACKETSTART       "["
    T_BRACKETEND         "]"
    T_BRACESTART         "{"
    T_BRACEEND           "}"
    T_PARENSTART         "("
    T_PARENEND           ")"
    T_SET                ":="
    T_EQUALS             "="
    T_PLUS               "+"
    T_COMMA              ","
    T_END                ";"
;

%token <std::string>    T_NAME      "name"
%token <int>            T_NUMBER    "number"
%token <double>         T_DECIMAL    "decimal"

%nterm <std::vector<specie>> dSpecies
%nterm <std::vector<specie>> speciesArray
%nterm <speciesRatios> reactionSpeciesList
%nterm <std::pair<specie, int>> reactionSpecie
%nterm <double> reactionRate
%nterm <Composition*> composition
%nterm <std::vector<Composition*>> compositions

%%

%start modules;

modules  : module
         | modules module
         ;

module : T_DMODULE "name" "{" properties "}" { drv.currentModule.name = $2; drv.FinishParsingModule(); }
       | T_DFUNCTION "name" "{" properties "}" { drv.currentModule.name = $2; drv.FinishParsingFunction(); }

properties : property
		   | properties property
		   ;

property : "private:" dSpecies ";" { MergeVectors(drv.currentModule.privateSpecies, $2); }
		 | "output:" dSpecies ";" { MergeVectors(drv.currentModule.outputSpecies, $2); }
		 | "input:" dSpecies ";" { MergeVectors(drv.currentModule.inputSpecies, $2); }
		 | "reactions:" "{" reactions "}"
		 | "concentrations:" "{" concentrations "}"
		 | "compositions:" "{" compositions "}" {{ MergeVectors(drv.currentModule.compositions, $3); }}
		 ;

compositions: composition { std::vector<Composition*> vec; vec.push_back($1); $$ = vec; }
			| compositions composition { auto vec = $1; $1.push_back($2); $$ = $1; }
      ;

composition: speciesArray "=" "name" "(" speciesArray ")" ";" { $$ = MakeComposition(drv, $3, $5, $1); }
					 | speciesArray "=" "name" "(" ")" ";" { $$ = MakeComposition(drv, $3, std::vector<specie>(), $1); }
           | "if" "(" "name" ")" "{" compositions "}" { $$ = new ConditionalComposition($3, $6); }
           | "scale" "(" "number" ")" "{" compositions "}" { $$ = new ScalarComposition(($3), $6); }
           | "scale" "(" "decimal" ")" "{" compositions "}" { $$ = new ScalarComposition($3, $6); }
		       ;

reactions: reaction
		 | reactions reaction
		 ;

reaction: reactionSpeciesList "->" reactionSpeciesList ";"
            { reaction r = {$1, $3, 1}; drv.currentModule.reactions.push_back(r); }

        | reactionSpeciesList "->" "(" reactionRate ")" reactionSpeciesList ";"
            { reaction r = {$1, $6, $4}; drv.currentModule.reactions.push_back(r); }

        | reactionSpeciesList "<->" reactionSpeciesList ";" {
            reaction r = {$1, $3, 1}; drv.currentModule.reactions.push_back(r);
            reaction R = {$3, $1, 1}; drv.currentModule.reactions.push_back(R);}

        | reactionSpeciesList "<->" "(" reactionRate ")" reactionSpeciesList ";" {
            reaction r = {$1, $6, $4}; drv.currentModule.reactions.push_back(r);
            reaction R = {$6, $1, 1}; drv.currentModule.reactions.push_back(R);}

        | reactionSpeciesList "(" reactionRate ")" "<->" "(" reactionRate ")" reactionSpeciesList ";" {
            reaction r = {$1, $9, $7}; drv.currentModule.reactions.push_back(r);
            reaction R = {$9, $1, $3}; drv.currentModule.reactions.push_back(R); }

        | reactionSpeciesList "(" reactionRate ")" "<->" reactionSpeciesList ";" {
            reaction r = {$1, $6, 1}; drv.currentModule.reactions.push_back(r);
            reaction R = {$6, $1, $3}; drv.currentModule.reactions.push_back(R); }

reactionRate : "number" { $$ = static_cast<double>($1); }
             | "decimal" { $$ = $1; }

reactionSpeciesList: reactionSpecie { speciesRatios l; InsertToSpecieMap(l, $1); $$ = l; }
                    | reactionSpeciesList "+" reactionSpecie { speciesRatios l = $1; InsertToSpecieMap(l, $3); $$ = l; }
                    | "number" {if ($1 !=0) {yy::parser::error(@1, "Standalone number in reaction "); YYABORT; }
                            $$ = std::map<std::string, int>();}
                    ;

reactionSpecie: "name" { $$ = std::pair<specie, int>(std::move($1), std::move(1)); }
                | "number" "name" { $$ = std::pair<specie, int>(std::move($2), std::move($1)); }
                ;

dSpecies: "name" { std::vector<specie> v; v.push_back($1); $$ = v; }
		| "[" speciesArray "]" { $$ = $2; }
		;

speciesArray: "name" { std::vector<specie> v; v.push_back($1); $$ = v; }
			| speciesArray "," "name" { std::vector<specie> v = $1; v.push_back($3); $$ = v; }
			;

concentrations: concentration
			  | concentrations concentration
			  ;

concentration: "name" ":=" "number" ";" {drv.currentModule.concentrations.insert(std::make_pair($1, $3));}
			 ;

%%

void yy::parser::error (const location_type &l, const std::string &m)
{
  std::cerr << l << ": " << m << '\n';
}
