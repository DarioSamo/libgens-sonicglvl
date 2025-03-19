#pragma once

// Hash function from Sonic Generations, which lets us convert strings to keys.
// This is used in every Hedgehog Engine game up to Frontiers, it's safe to assume most hashes are derived from this.

inline int hhStrHash(const char* value) {
	int hash = 0;
	while (*value) {
		hash = hash * 31 + *value;
		++value;
	}

	return hash & 0x7FFFFFFF;
}