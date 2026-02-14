/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file townname.cpp %Town name generators. */

#include "stdafx.h"
#include "string_func.h"
#include "townname_type.h"
#include "town.h"
#include "strings_func.h"
#include "core/random_func.hpp"
#include "genworld.h"
#include "gfx_layout.h"
#include "strings_internal.h"

//#include "table/townname_original.h"

#include "safeguards.h"


/**
 * Initializes this struct from town data
 * @param t town for which we will be printing name later
 */
TownNameParams::TownNameParams(const Town *t) :
		grfid(t->townnamegrfid), // by default, use supplied data
		type(t->townnametype),
	    use_original_generator(false)
{
	if (t->townnamegrfid != 0 && GetGRFTownName(t->townnamegrfid) == nullptr) {
		/* Fallback to the first built in town name (English). */
		this->grfid = 0;
		this->type = SPECSTR_TOWNNAME_START;
		return;
	}
}


/**
 * Fills builder with specified town name.
 * @param builder       The string builder.
 * @param par           Town name parameters.
 * @param townnameparts 'Encoded' town name.
 */
static void GetGeneratorTownName(StringBuilder &builder, TownNameParams *par, uint32_t townnameparts)
{
	if (par->grfid == 0) {
		auto tmp_params = MakeParameters(townnameparts, par->use_original_generator);
		GetStringWithArgs(builder, par->type, tmp_params);
		return;
	}

	GRFTownNameGenerate(builder, par->grfid, par->type, townnameparts);
}

/**
 * Get the town name for the given parameters and parts.
 * @param par Town name parameters.
 * @param townnameparts 'Encoded' town name.
 * @return The town name.
 */
std::string GetGeneratorTownName(TownNameParams *par, uint32_t townnameparts)
{
	std::string result;
	StringBuilder builder(result);
	GetGeneratorTownName(builder, par, townnameparts);
	return result;
}

/**
 * Fills builder with town's name.
 * @param builder String builder.
 * @param t       The town to get the name from.
 */
void GetGeneratorTownName(StringBuilder &builder, const Town *t)
{
	TownNameParams par(t);
	GetGeneratorTownName(builder, &par, t->townnameparts);
}

/**
 * Get the name of the given town.
 * @param t The town to get the name for.
 * @return The town name.
 */
std::string GetGeneratorTownName(const Town *t)
{
	TownNameParams par(t);
	return GetGeneratorTownName(&par, t->townnameparts);
}


/**
 * Verifies the town name is valid and unique.
 * @param proposed_name is the generated or custom name to check
 * @return true if name is valid and unique
 */
bool VerifyTownName(std::string proposed_name)
{
	/* Check size and width */
	if (Utf8StringLength(proposed_name) >= MAX_LENGTH_TOWN_NAME_CHARS) return false;

	for (const Town *t : Town::Iterate()) {
		if (proposed_name == t->name) return false;
	}

	return true;
}


/**
 * Generates valid town name.
 * @param randomizer the source of random data for generating the name
 * @param townnameparts if a name is generated, it's stored there
 * @param town_names if a name is generated, check its uniqueness with the set
 * @return true iff a name was generated
 */
bool GenerateTownName(Randomizer &randomizer, std::string *town_name)
{
	TownNameParams par(_settings_game.game_creation.town_name);

	par.use_original_generator = false; // TODO: false is default but we could create a config paramter so users can switch between original and enhanced generators.

	/* Do not set i too low, since when we run out of names, we loop
	 * for #tries only one time anyway - then we stop generating more
	 * towns. Do not set it too high either, since looping through all
	 * the other towns may take considerable amount of time (10000 is
	 * too much). */
	for (int i = 1000; i != 0; i--) {
		uint32_t r = randomizer.Next();
		std::string name = GetGeneratorTownName(&par, r);
		if (!VerifyTownName(name)) continue;

		*town_name = name;
		return true;
	}

	return false;
}


/**
 * Generates town name from given seed.
 * @param builder string builder to write to
 * @param lang    town name language
 * @param seed    generation seed
 */
void GenerateTownNameString(StringBuilder &builder, size_t lang, uint32_t seed, bool use_original_generator)
{
	if (use_original_generator) {
		return GenerateOriginalTownNameString(builder, lang, seed);
	} else {
		return GenerateExtendedTownNameString(builder, lang, seed);
	}
}
