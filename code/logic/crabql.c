/*
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 * 
 * This file is part of the Fossil Logic project, which aims to develop high-
 * performance, cross-platform applications and libraries. The code contained
 * herein is subject to the terms and conditions defined in the project license.
 * 
 * Author: Michael Gene Brockus (Dreamer)
 * 
 * Copyright (C) 2024 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/crabdb/crabql.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

// List of offensive words and phrases (super hard to mainting thisw list as GitHub Copilot doesnt wanna help with this part of the SOAP API)
static const char *offensive_words[] = {
    "curse1",
    "curse2",
    "racist_phrase1",
    "racist_phrase2", // for demo and unit testing we keep these four keywords

    // English offensive words and phrases
    "2g1c", "2 girls 1 cup", "acrotomophilia", "alabama hot pocket", "alaskan pipeline", "anal", "anilingus", "anus", "apeshit", "arsehole", "ass", "asshole", "assmunch", "auto erotic", "autoerotic", "babeland",
    "baby batter", "baby juice", "ball gag", "ball gravy", "ball kicking", "ball licking", "ball sack", "ball sucking", "bangbros", "bareback", "barely legal", "barenaked", "bastard", "bastardo", "bastinado", "bbw",
    "bdsm", "beaner", "beaners", "beaver cleaver", "beaver lips", "bestiality", "big black", "big breasts", "big knockers", "big tits", "bimbos", "birdlock", "bitch", "bitches", "black cock", "blonde action", "blonde on blonde action",
    "blowjob", "blow job", "blow your load", "blue waffle", "blumpkin", "bollocks", "bondage", "boner", "boob", "boobs", "booty call", "brown showers", "brunette action", "bukkake", "bulldyke", "bullet vibe", "bullshit",
    "bung hole", "bunghole", "busty", "butt", "buttcheeks", "butthole", "camel toe", "camgirl", "camslut", "camwhore", "carpet muncher", "carpetmuncher", "chocolate rosebuds", "circlejerk", "cleveland steamer", "clit",
    "clitoris", "clover clamps", "clusterfuck", "cock", "cocks", "coprolagnia", "coprophilia", "cornhole", "coon", "coons", "creampie", "cum", "cumming", "cunnilingus", "cunt", "darkie", "date rape", "daterape",
    "deep throat", "deepthroat", "dendrophilia", "dick", "dildo", "dingleberry", "dingleberries", "dirty pillows", "dirty sanchez", "doggie style", "doggiestyle", "doggy style", "doggystyle", "dog style", "dolcett",
    "domination", "dominatrix", "dommes", "donkey punch", "double dong", "double penetration", "dp action", "dry hump", "dvda", "eat my ass", "ecchi", "ejaculation", "erotic", "erotism", "escort", "eunuch", "faggot",
    "fecal", "felch", "fellatio", "feltch", "female squirting", "femdom", "figging", "fingerbang", "fingering", "fisting", "foot fetish", "footjob", "frotting", "fuck", "fuck buttons", "fuckin", "fucking", "fucktards",
    "fudge packer", "fudgepacker", "futanari", "gang bang", "gay sex", "genitals", "giant cock", "girl on", "girl on top", "girls gone wild", "goatcx", "goatse", "god damn", "gokkun", "golden shower", "goodpoop",
    "goo girl", "goregasm", "grope", "group sex", "g-spot", "guro", "hand job", "handjob", "hard core", "hardcore", "hentai", "homoerotic", "honkey", "hooker", "hot carl", "hot chick", "how to kill", "how to murder",
    "huge fat", "humping", "incest", "intercourse", "jack off", "jail bait", "jailbait", "jelly donut", "jerk off", "jigaboo", "jiggaboo", "jiggerboo", "jizz", "juggs", "kike", "kinbaku", "kinkster", "kinky", "knobbing",
    "leather restraint", "leather straight jacket", "lemon party", "lolita", "lovemaking", "make me come", "male squirting", "masturbate", "menage a trois", "milf", "missionary position", "motherfucker", "mound of venus",
    "mr hands", "muff diver", "muffdiving", "nambla", "nawashi", "negro", "neonazi", "nigga", "nigger", "nig nog", "nimphomania", "nipple", "nipples", "nsfw images", "nude", "nudity", "nympho", "nymphomania", "octopussy",
    "omorashi", "one cup two girls", "one guy one jar", "orgasm", "orgy", "paedophile", "paki", "panties", "panty", "pedobear", "pedophile", "pegging", "penis", "phone sex", "piece of shit", "pissing", "piss pig", "pisspig",
    "playboy", "pleasure chest", "pole smoker", "ponyplay", "poof", "poon", "poontang", "punany", "poop chute", "poopchute", "porn", "porno", "pornography", "prince albert piercing", "pthc", "pubes", "pussy", "queaf", "queef",
    "quim", "raghead", "raging boner", "rape", "raping", "rapist", "rectum", "reverse cowgirl", "rimjob", "rimming", "rosy palm", "rosy palm and her 5 sisters", "rusty trombone", "sadism", "santorum", "scat", "schlong",
    "scissoring", "semen", "sex", "sexo", "sexy", "shaved beaver", "shaved pussy", "shemale", "shibari", "shit", "shitblimp", "shitty", "shota", "shrimping", "skeet", "slanteye", "slut", "s&m", "smut", "snatch", "snowballing",
    "sodomize", "sodomy", "spic", "splooge", "splooge moose", "spooge", "spread legs", "spunk", "strap on", "strapon", "strappado", "strip club", "style doggy", "suck", "sucks", "suicide girls", "sultry women", "swastika",
    "swinger", "tainted love", "taste my", "tea bagging", "threesome", "throating", "tied up", "tight white", "tit", "tits", "titties", "titty", "tongue in a", "topless", "tosser", "towelhead", "tranny", "tribadism",
    "tub girl", "tubgirl", "tushy", "twat", "twink", "twinkie", "two girls one cup", "undressing", "upskirt", "urethra play", "urophilia", "vagina", "venus mound", "vibrator", "violet wand", "vorarephilia", "voyeur", "vulva",
    "wank", "wetback", "wet dream", "white power", "wrapping men", "wrinkled starfish", "xx", "xxx", "yaoi", "yellow showers", "yiffy", "zoophilia"

    // Support for other languages can be added via PR to this repository
};

bool contains_banned_words(const char* query) {
    if (!query) return false;
    for (int i = 0; offensive_words[i] != NULL; i++) {
        if (strstr(query, offensive_words[i]) != NULL) {
            return true;  // Banned word found
        }
    }
    return false;  // No banned words found
}

/* Search Operations */

fossil_crabql_result_collection_t* fossil_crabql_init_result_collection(size_t capacity) {
    fossil_crabql_result_collection_t* collection = (fossil_crabql_result_collection_t*)malloc(sizeof(fossil_crabql_result_collection_t));
    if (!collection) {
        return NULL;  // Memory allocation failed
    }
    
    collection->results = (fossil_crabql_result_t*)malloc(capacity * sizeof(fossil_crabql_result_t));
    if (!collection->results) {
        free(collection);  // Clean up previously allocated memory
        return NULL;  // Memory allocation failed
    }
    
    collection->count = 0;
    collection->capacity = capacity;
    return collection;
}

void fossil_crabql_destroy_result_collection(fossil_crabql_result_collection_t* collection) {
    if (collection) {
        free(collection->results);  // Free the array of results
        free(collection);            // Free the collection structure
    }
}

bool fossil_crabql_search_by_key(fossil_crabdb_t* db, const char* key, fossil_crabql_result_t* result) {
    if (contains_banned_words(key)) {
        return false;  // Key contains banned words, abort search
    }

    const char* value = fossil_crabdb_get_value_by_key(db, key);
    if (value) {
        strncpy(result->key, key, FOSSIL_CRABQL_KEY_SIZE);
        strncpy(result->value, value, FOSSIL_CRABQL_VAL_SIZE);
        return true;  // Key was found
    }
    return false;  // Key not found
}

bool fossil_crabql_search_by_value(fossil_crabdb_t* db, const char* value, fossil_crabql_result_collection_t* collection) {
    if (contains_banned_words(value)) {
        return false;  // Value contains banned words, abort search
    }

    size_t found_count = fossil_crabdb_find_keys_by_value(db, value, collection->results, collection->capacity);
    if (found_count > 0) {
        collection->count = found_count;  // Update the count of results found
        return true;  // Matching pairs found
    }
    return false;  // No matching pairs found
}

bool fossil_crabql_search_by_pattern(fossil_crabdb_t* db, const char* pattern, fossil_crabql_result_collection_t* collection) {
    if (contains_banned_words(pattern)) {
        return false;  // Pattern contains banned words, abort search
    }

    size_t found_count = fossil_crabdb_find_keys_by_pattern(db, pattern, collection->results, collection->capacity);
    if (found_count > 0) {
        collection->count = found_count;  // Update the count of results found
        return true;  // Matching pairs found
    }
    return false;  // No matching pairs found
}

bool fossil_crabql_search_by_prefix(fossil_crabdb_t* db, const char* prefix, fossil_crabql_result_collection_t* collection) {
    if (contains_banned_words(prefix)) {
        return false;  // Prefix contains banned words, abort search
    }

    size_t found_count = fossil_crabdb_find_keys_by_prefix(db, prefix, collection->results, collection->capacity);
    if (found_count > 0) {
        collection->count = found_count;  // Update the count of results found
        return true;  // Matching pairs found
    }
    return false;  // No matching pairs found
}

bool fossil_crabql_search_by_range(fossil_crabdb_t* db, const char* start_key, const char* end_key, fossil_crabql_result_collection_t* collection) {
    if (contains_banned_words(start_key) || contains_banned_words(end_key)) {
        return false;  // Range contains banned words, abort search
    }

    size_t found_count = fossil_crabdb_find_keys_in_range(db, start_key, end_key, collection->results, collection->capacity);
    if (found_count > 0) {
        collection->count = found_count;  // Update the count of results found
        return true;  // Matching pairs found
    }
    return false;  // No matching pairs found
}

bool fossil_crabql_resize_result_collection(fossil_crabql_result_collection_t* collection, size_t new_capacity) {
    if (new_capacity <= collection->capacity) {
        return false;  // New capacity must be larger
    }
    
    fossil_crabql_result_t* new_results = (fossil_crabql_result_t*)realloc(collection->results, new_capacity * sizeof(fossil_crabql_result_t));
    if (!new_results) {
        return false;  // Memory allocation failed
    }
    
    collection->results = new_results;  // Update the results pointer
    collection->capacity = new_capacity;  // Update the capacity
    return true;  // Resizing successful
}
