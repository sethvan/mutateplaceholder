#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "OperationsDV1.h"

#define LINE_LENGTH 119

char cardNumber[15][12] = {"first", "second", "third", "fourth", "fifth", "sixth", "seventh",
						   "eighth", "ninth", "tenth", "eleventh", "twelfth", "thirteenth",
						   "fourteenth", "fifteenth"};

int get_int_value(Type type) {
	int intVal;
	do	{
		printf("\nHow many %ss are you considering? (maximum%d)\nNumber of %ss? = ", type.name, type.max, type.name);
		if ((scanf("%i", &intVal) != 1) || (intVal < type.min || intVal > type.max)) {
			printf("\nInvalid entry, please enter an integer between %d and %d.\n", type.min, type.max);
			while ((getchar()) != '\n');
		}
	} while (intVal < type.min || intVal > type.max);
	getchar();

	return intVal;
}

char *myGets(char *st, int n) {
	char *ret_val;
	if ((ret_val = fgets(st, n, stdin)) != NULL) {
		st[strcspn(st, "\n")] = 0;
	}

	return ret_val;
}

StringArray get_string_array(Type type) {
	StringArray sa;

	sa.size = get_int_value(type);

	sa.array = NULL;
	if (!(sa.array = malloc(sa.size * sizeof(char *)))) {
		printf("Failure to allocate memory!\n");
		exit(1);
	}

	printf("\nWe will now enter the names of your %ss, please keep them short as\n"
		   " the output spreadsheet will only display the first %d characters\n",
		   type.name, type.dislength);

	for (int i = 0; i < sa.size; ++i) {
		if (!(sa.array[i] = malloc(30))) {
			printf("Failure to allocate memory!");
			exit(1);
		}
		printf("\nWhat is your %s %s?:\n", cardNumber[i], type.name);
		myGets(sa.array[i], 30);
	}

	return sa;
}

StringArray sort_factor(StringArray factorArray) {
	char *temp = NULL;
	int mostImportant = 0;
	for (int i = 0; i < factorArray.size - 1; ++i) {
		if (i == 0)
			printf("\nAmong the factors listed below, which is the most important?\n");
		else
			printf("\nAnd which is the most important of these that remain?\n\n");

		for (int j = 1; j < factorArray.size - i + 1; ++j) {
			printf("%i. %s\n", j, factorArray.array[i + j - 1]);
		}

		do {
			printf("\nEnter the corresponding number:  ");
			if ((scanf("%i", &mostImportant) != 1) || (mostImportant < 1 || mostImportant > factorArray.size - i))
			{
				printf("\nInvalid entry, please enter an integer between 1 and %i.\n", factorArray.size - i);
				while ((getchar()) != '\n')
					;
			}
		} while (mostImportant < 1 || mostImportant > factorArray.size - i);
		getchar();

		mostImportant += i - 1;

		temp = factorArray.array[mostImportant];
		factorArray.array[mostImportant] = factorArray.array[i];
		factorArray.array[i] = temp;
	}
	return factorArray;
}

float *qtfy_factor(StringArray factorArray) {
	float *pfGrade = NULL;
	if (!(pfGrade = malloc(factorArray.size * sizeof(float)))) {
		printf("Failure to allocate memory!\n");
		exit(1);
	}

	for (int i = 0; i < factorArray.size; ++i) {
		do {
			printf("\nFrom 1-1000 how important is the factor of:\n\"%s\"?: ", factorArray.array[i]);
			if ((scanf("%f", &pfGrade[i]) != 1) || (pfGrade[i] < 1.0f || pfGrade[i] > 1000.0f)) {
				printf("\nInvalid entry, please enter a number between 1 and 1000.\n");
				while ((getchar()) != '\n');
			}
		} while (pfGrade[i] < 1.0f || pfGrade[i] > 1000.0f);
		printf("\n\n");
	}
	getchar();
	return pfGrade;
}

float *rate_compat(StringArray optionArray, StringArray factorArray) {
	int size = factorArray.size * optionArray.size;

	float *compatability = NULL;
	if (!(compatability = malloc(size * sizeof(float)))) {
		printf("Failure to allocate memory!\n");
		exit(1);
	}

	for (int i = 0; i < size; ++i) {
		do {
			printf("From 0-1000 how ideal is option \"%s\"\nfor factor \"%s\"?:",
				   optionArray.array[i % optionArray.size], factorArray.array[i / optionArray.size]);
			if ((scanf("%f", &compatability[i]) != 1) || (compatability[i] < 0 || compatability[i] > 1000)) {
				printf("\nInvalid entry, please enter an integer between 0 and 100.\n");
				while ((getchar()) != '\n');
			}
		} while (compatability[i] < 0 || compatability[i] > 1000);
		printf("\n\n");
	}
	return compatability;
}

float *tally_sums(StringArray optionArray, StringArray factorArray, float *compatability, float *pfGrade) {
	int fGradeSize = optionArray.size * factorArray.size;
	float *finalGrade = NULL, *finalGradeSum = NULL;

	if (!(finalGrade = malloc(fGradeSize * sizeof(float))) || !(finalGradeSum = calloc(optionArray.size, sizeof(float)))) {
		printf("Failure to allocate memory!\n");
		exit(1);
	}

	for (int i = 0; i < fGradeSize; ++i) {
		finalGrade[i] = compatability[i] * pfGrade[i / optionArray.size];
		finalGradeSum[i % optionArray.size] += finalGrade[i];
	}

	free(finalGrade);
	return finalGradeSum;
}

void display_output_spreadsheet(StringArray optionsArray, StringArray factorsArray, float *compatability, float *pfGrade)
{
	int line = 0;
	printf("\n\n\n%38s%-80s\n", " ", "          Compatabilities of the options with each corresponding factor");
	printf("%38s", "Factor     ");
	for (line = 0; line < 81; line++)
		putchar('-');
	printf("\n%-26s%s", "Priority of factors", "|Importance |");
	for (int i = 0; i < optionsArray.size; ++i) {
		printf("%-15.15s|", optionsArray.array[i]);
	}

	if (optionsArray.size < 5) {
		for (line = 0; line < 79 - (optionsArray.size * 16); line++)
			putchar(' ');
		putchar('|');
	}
	printf("\n");

	for (line = 0; line < LINE_LENGTH; line++)
		putchar('-');
	for (int i = 0; i < factorsArray.size; ++i) {
		printf("\n%2i.%-22.22s |   %4.0lf    |", i + 1, factorsArray.array[i], pfGrade[i]);
		for (int j = i * optionsArray.size; j < (i + 1) * optionsArray.size; ++j)
			printf("      %3.0lf      |", compatability[j]);
	}
	printf("\n\n");
}

void display_option_totals(StringArray optionsArray, float *finalGradeSum) {
	printf("\nScore totals in favor of each option:\n\n");
	float best = 0.0f;
	int index = 0;

	for (int j = 0; j < optionsArray.size; ++j) {
		for (int i = 0; i < optionsArray.size; ++i) {
			if (finalGradeSum[i] > best) {
				best = finalGradeSum[i];
				index = i;
			}
		}
		printf("%-20s=  %.0f\n", optionsArray.array[index], finalGradeSum[index]);
		finalGradeSum[index] = 0.0f;
		best = 0.0f;
	}
	printf(("\n\n\n\n"));
}