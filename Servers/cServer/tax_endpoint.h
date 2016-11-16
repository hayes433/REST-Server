/********************************* Import Libraries **************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/***************************** Import global variables ***********************************/
#include "globals.h"
/******************************** Define structures **************************************/
typedef struct Tax_Info 
{
    int grossIncome;
    int freeIncome;
    int deductions;
    int allowances;
    int married;
} Tax_Info;

typedef struct Taxes
{
    int taxes_owed;
    float percent_of_grossIncome;
    float percent_of_freeIncome;
} Taxes;
/**************************** Function Declarations **************************************/
int JSON_to_tax_info(Tax_Info*, char*);
int tax_info_to_JSON(Tax_Info*, char*);
int taxes_to_JSON(Taxes*, char*);
int calculate_taxes(Tax_Info*, Taxes*);
Tax_Info* initialize_tax_info();
Taxes* initialize_taxes();
void free_tax_info(Tax_Info*);
void free_taxes(Taxes*);
int* get_field_tax_info(Tax_Info*, int);
int process_taxes(char*, char*);
/******************************** Define Macros ******************************************/
#define min(x, y) x < y ? x : y