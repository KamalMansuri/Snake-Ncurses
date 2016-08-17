#include <stdio.h>
#include <stdlib.h>

int main(void) {
	FILE *fp;
	fp = fopen("high-scores.txt", "r+");

	char f_str[40], s_str[40], l_str[40];

	fgets(f_str, 40 , fp); fgets(s_str, 40, fp);

	fclose(fp);
	fp = fopen("high-scores.txt", "w");

	fprintf(fp, "%s", f_str);
	fprintf(fp, "%s\n", s_str);
	fprintf(fp, "%s\n", s_str);
	fclose(fp);
	return 0;
}