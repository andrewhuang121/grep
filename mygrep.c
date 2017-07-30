/*
 * File: mygrep.c
 * Student: Andrew Huang
 * -----------------------
 *
 * The 'mygrep' program implements a simplified version of the 'grep' command.
 */

#include <error.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024


#define OPEN_EMPHASIS "\e[7m"
#define CLOSE_EMPHASIS "\e[0m"

static int regex_match(const char *input, const char *pattern, const char ***endp);
//Searches through, returns a start pointer, and through indirection returns an end porter
static const char *search(const char *input, const char *pattern, const char **endp) //*** necessary because modification of pointer happens in regex_match
{
	for (int i = 0; input[i] != '\0'; i++) { //loops through the input word
	    if (*pattern == '^') {
            if (regex_match(input + i, pattern + 1, &endp) == 0) {//skips the ^, breaks if it doesn't work in the front
                return input + i;
            } else {
                break; //if it doesn't work for the fron when ^ is present, break
            }
        }
        if (regex_match(input + i, pattern, &endp) == 0) { //if pattern matches
			return input + i; //portion where the regex begins
        }
	}
	return NULL; //default
}
//returns -1 if not a match. returns 0 if a match.
static int regex_match(const char *input, const char *pattern, const char ***endp) {

    if (*pattern == '\0') { //reached end of pattern: match!
        **endp = input;//set the pointer to the end
        return 0;
    } else if (*input == '\0' && *pattern == '$') { //end of pattern, end of string. match!
        **endp = input;
        return 0;
    } else if (*(pattern + 1) == '*') { //checks if next is *
        if (*pattern == *input || *pattern == '.') { //if match
            if (*input == '\0') { //if patter has . but input is empty
                return regex_match(input, pattern + 2, endp); //try with skipping the *. catches lots of end cases
            } else if (regex_match(input + 1, pattern, endp) == 0) { //explore, if it works return 0
                return 0;
            } else {
                if(regex_match(input + 1, pattern + 2, endp) == 0) { //extensive exploration
                    return 0;
                } else {
                    return regex_match(input, pattern + 2, endp); //backtracking extensive exploration
                }
            }
        } else {
            return regex_match(input, pattern + 2, endp); //if it doesn't, skip
        }
    } else if (*pattern == '.') { //catch the metacharacter
        if (*input == '\0') { //same case mentioned above
            return -1;
        } else {
            return regex_match(input + 1, pattern + 1, endp); //advance! . works
        }
    } else if (*input != *pattern) { //no match. :(
        return -1;
    } else {
        return regex_match(input + 1, pattern + 1, endp); //this case is the equivalent of things matching. advance!
    }
}

//prints surrounding necessary regex with brackets
static void print_with_emphasis(const char *cur, const char *start, const char *end)
{
	int nbefore = start - cur; //pointer arithmetic
	int nmatched = end - start;
	printf("%.*s%s%.*s%s", nbefore, cur, OPEN_EMPHASIS, nmatched, start, CLOSE_EMPHASIS);//prints part of cur in front of start, [, pattern, and then ]
}
//prints the line, recursively searching for the pattern
static void print_match(const char *line, const char *pattern, const char *filename)
{
  
    const char *end = NULL;
    const char *start = search(line, pattern, &end);
	    if (start != NULL) {
		    if (filename != NULL) printf("%s: ", filename);
		    print_with_emphasis(line, start, end);
		    print_match(end, pattern, filename); // turns "end" into the new "line," then searches
            if (search(end, pattern, &end) == NULL) { //if there's nothing left, there's no patterns left, so then print out end.
                printf("%s\n", end);
            }
	    }

}
//File operations for grep. Left unchanged from starter code.
static void grep_file(FILE *fp, const char *pattern, const char *filename)
{
	char line[MAX_LINE_LEN];

	while (fgets(line, sizeof(line), fp) != NULL) {
		// truncate trailing newline if present (most lines will have one, last line might not)
		if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0'; //turn it into a single line string
		print_match(line, pattern, filename);
	}
}
//Main method, takes in arguments from command lined. Left unchanged from start code.
int main(int argc, char *argv[])
{
	if (argc < 2) error(1, 0, "Usage: grep PATTERN [FILE]...");
	const char *pattern = argv[1];

	if (argc == 2) 
		grep_file(stdin, pattern, NULL);
	else {
		for (int i = 2; i < argc; i++) {
	    	FILE *fp = fopen(argv[i], "r");
	    	if (fp == NULL) error(1, 0, "%s: no such file", argv[i]);
	    	grep_file(fp, pattern, argc > 3 ? argv[i] : NULL);
	    	fclose(fp);
	    }
    }
    return 0;
}

