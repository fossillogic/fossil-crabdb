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

int fossil_crabdb_query_by_type(fossil_crabdb_t* db, fossil_crabdb_type_t type, char* result_buffer, size_t buffer_size) {
    if (!db || !result_buffer) return -1;
    if (buffer_size < MIN_BUFFER_SIZE) return -2;
    if (type < FOSSIL_CRABDB_TYPE_MIN || type > FOSSIL_CRABDB_TYPE_MAX) return -3;

    memset(result_buffer, 0, buffer_size);
    size_t written = 0;
    fossil_crabdb_node_t* current = db->head;
    bool found = false;

    while (current) {
        if (current->type == type) {
            int required_size = snprintf(NULL, 0, "%s:%s\n", current->key, current->value);
            if (written + required_size >= buffer_size) return -4;

            written += snprintf(result_buffer + written, buffer_size - written, "%s:%s\n", current->key, current->value);
            found = true;
        }
        current = current->next;
    }

    return found ? 0 : -5;  // -5 if no matches were found
}

bool fossil_crabdb_query_range(fossil_crabdb_t* db, const char* key, const char* min_value, const char* max_value, char* result_buffer, size_t buffer_size) {
    if (!db || !key || !min_value || !max_value || !result_buffer) return false;

    size_t current_size = 0;
    fossil_crabdb_node_t* node = db->head;

    while (node) {
        if (strncmp(node->key, key, strlen(key)) == 0 && strcmp(node->value, min_value) >= 0 && strcmp(node->value, max_value) <= 0) {
            int written = snprintf(result_buffer + current_size, buffer_size - current_size, "%s: %s\n", node->key, node->value);
            if (written < 0 || (size_t)written >= buffer_size - current_size) return false;

            current_size += written;
        }
        node = node->next;
    }
    return current_size > 0;
}

bool fossil_crabdb_full_text_search(fossil_crabdb_t* db, const char* search_text, char* result_buffer, size_t buffer_size) {
    if (!db || !search_text || !result_buffer) return false;
    if (contains_banned_words(search_text)) {
        fprintf(stderr, "Error: Search contains restricted words.\n");
        return false;
    }

    size_t current_size = 0;
    fossil_crabdb_node_t* node = db->head;

    while (node) {
        if (node->type == FOSSIL_CRABDB_TYPE_STRING && strstr(node->value, search_text)) {
            int written = snprintf(result_buffer + current_size, buffer_size - current_size, "%s: %s\n", node->key, node->value);
            if (written < 0 || (size_t)written >= buffer_size - current_size) return false;

            current_size += written;
        }
        node = node->next;
    }
    return current_size > 0;
}

bool fossil_crabdb_query_by_time(fossil_crabdb_t* db, time_t time_criteria, bool newer_than, char* result_buffer, size_t buffer_size) {
    if (!db || !result_buffer) return false;

    size_t current_size = 0;
    fossil_crabdb_node_t* node = db->head;

    while (node) {
        time_t node_time = node->timestamp;
        if ((newer_than && node_time > time_criteria) || (!newer_than && node_time < time_criteria)) {
            int written = snprintf(result_buffer + current_size, buffer_size - current_size, "%s\n", node->key);
            if (written < 0 || (size_t)written >= buffer_size - current_size) return false;

            current_size += written;
        }
        node = node->next;
    }
    return current_size > 0;
}
