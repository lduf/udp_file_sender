#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include "includes/utils.h"


/**
 * @brief This function generates a random int between the specified min and the specified max.
 * @param min The minimum value of the random number.
 * @param max The maximum value of the random number.
 * 
 * @return The random number.
 */
int random_int(int min, int max) {
    return rand() % (max - min + 1) + min;
}

/**
 * @brief This function compares the given string with the given regex.
 * 
 * @param string 
 * @param rgx 
 * @return int : returns 1 if the string matches the regex, 0 otherwise.
 */

int compareString(char string[], char rgx[]){
	printf("regex used : %s\n", rgx);
	regex_t regex;
	int reti;
	char msgbuf[100];

	int res = 0;

	/* Compile regular expression */
	reti = regcomp(&regex, rgx, REG_EXTENDED);
	if (reti) {
	    fprintf(stderr, "Could not compile regex\n");
	    exit(1);
	}

	/* Execute regular expression */
	reti = regexec(&regex, string, 0, NULL, 0);
	if (!reti) {
	    res = 1;
	}
	else if (reti == REG_NOMATCH) {
	    res = 0;
	}
	else {
	    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
	    fprintf(stderr, "Regex match failed: %s\n", msgbuf);
	    exit(1);
	}

	/* Free memory allocated to the pattern buffer by regcomp() */
	regfree(&regex);
	return res;
}