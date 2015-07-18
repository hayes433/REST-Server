#include "tax_endpoint.h"

int process_taxes(char* JSON, char* response)
{
	Tax_Info* tax_info = initialize_tax_info();
	Taxes* taxes = initialize_taxes();
	int flag = 1;

	// Serialize Tax_Info
	flag &= JSON_to_tax_info(tax_info, JSON);
	// Calculate Taxes
	flag &= calculate_taxes(tax_info, taxes);
	// Deserialize Taxes
	flag &= taxes_to_JSON(taxes, response);

	// Free allocated memory
	free_taxes(taxes);
	free_tax_info(tax_info);

	return flag;
}

int JSON_to_tax_info(Tax_Info* tax_info, char* json)
{
	// If json or tax_info are null or if json is empty return error
	if(!json || !*json || !tax_info)
	{
		return 0;
	}

	// Define local variables
	int i = 0;
	int index = 0;
	char c = 0;
	char* fields[5] = {"\"gross_income\"", "\"free_income\"", "\"deductions\"", "\"allowances\"", "\"married\""};
	char* token;
	char field[STRINGSIZE];
	char value[STRINGSIZE];

	// Strip the braces from the json string
	json = strtok(json, "{");
	json = strtok(json, "}");

	// Get first element
	token = strtok(json, ",");
	while(token && i < 5)
	{
		// Reset while loop variables
		index = 0;		
		c = *token;
		// Get all characters until the ':'
		while(c && c != ':')
		{
			// Store field name one char at a time
			*(field + index) = c;
			c = *(token + ++index);
		}
		// Check for malformed JSON
		if(!c)
		{
			return 0;
		}
		// Terminate string
		*(field + index) = 0;
		// Copy everything after the ':'
		strcpy(value, token + ++index);
		// Check to see if it is valid field else return error
		if(strcmp(field, fields[i]) == 0)
		{
			if(i < 4)
			{
				// Set appropriate field to value as an int
				*(get_field_tax_info(tax_info, i)) = (int) strtol(value, (char **) NULL, 10);
			}
			else
			{
				// Set married field to 1 if "false" 2 if "true" or return error
				if(strcmp(value, "true") == 0)
				{
					tax_info->married = 1;
				}
				else if(strcmp(value, "false") == 0)
				{
					tax_info->married = 0;
				}
				else
				{
					return 0;
				}
			}
		}
		else
		{
			return 0;
		}
		// Get next field:value pair
		token = strtok(NULL, ",");
		// increment pointer
		i++;
	}
	// Return success
	return 1;
}
int tax_info_to_JSON(Tax_Info* tax_info, char* json)
{
	// return failure if tax_info or json are null
	if(!tax_info || !json)
	{
		return 0;
	}

	char val[15];

	// concat gross_income to JSON
	strcpy(json, "{\"gross_income\":");
	sprintf(val, "%d", tax_info->gross_income);
	strcat(json, val);
	strcat(json, ",");

	// concat free_income to JSON
	strcat(json, "\"free_income\":");
	sprintf(val, "%d", tax_info->free_income);
	strcat(json, val);
	strcat(json, ",");

	// concat deductions to JSON
	strcat(json, "\"deductions\":");
	sprintf(val, "%d", tax_info->deductions);
	strcat(json, val);
	strcat(json, ",");

	// concat allowances to JSON
	strcat(json, "\"allowances\":");
	sprintf(val, "%d", tax_info->allowances);
	strcat(json, val);

	strcat(json, "}");
	return 1;
}
int taxes_to_JSON(Taxes* taxes, char* json)
{
	// return failure if taxes is null
	if(!taxes || !json)
	{
		return 0;
	}

	char val[15];

	// concat taxes_owed to JSON
	strcpy(json, "{\"taxes_owed\":");
	sprintf(val, "%d", taxes->taxes_owed);
	strcat(json, val);
	strcat(json, ",");

	// concat percent_of_gross_income to JSON
	strcat(json, "\"percent_of_gross_income\":");
	sprintf(val, "%03.1f", taxes->percent_of_gross_income);
	strcat(json, val);
	strcat(json, ",");

	// concat percent_of_free_income to JSON
	strcat(json, "\"percent_of_free_income\":");
	sprintf(val, "%03.1f", taxes->percent_of_free_income);
	strcat(json, val);

	strcat(json, "}");
	return 1;
}
int calculate_taxes(Tax_Info* tax_info, Taxes* taxes)
{
	// Check for Null structs and return error
	if(!tax_info || !taxes)
	{
		return 0;
	}

	int taxable = 0;
	float tax = 0.0;
	int married = tax_info->married;
	if (tax_info->deductions)
	{
		taxable = tax_info->gross_income - tax_info->deductions;
	}
	else
	{
		taxable = tax_info->gross_income - tax_info->allowances * 9000;
	}
	if(taxable > 406000 + (married * 50000))
	{
		tax += (taxable - (406000 + (married * 50000))) * 0.396;
		taxable = (406000 + (married * 50000));
	}
	if(taxable > 405000)
	{
		tax += (min(406000 + (married * 50000), taxable) - 405000) * 0.35;
		taxable = 405000;
	}
	if(taxable > 186000 + (married * 40000))
	{
		tax += (min(405000, taxable) - (186000 + (married * 50000))) * 0.33;
		taxable = 186000 + (married * 50000);
	}
	if(taxable > 89000 + (married * 50000))
	{
		tax += (min(186000 + (married * 40000), taxable) - (89000 + (married * 50000))) * 0.28;
		taxable = 89000 + (married * 50000);
	}
	if(taxable > 36000 + (married * 36000))
	{
		tax += (min(89000 + (married * 50000), taxable) - (36000 + (married * 36000))) * 0.25;
		taxable = 36000 + (married * 36000);
	}
	if(taxable > 9000 + (married * 9000))
	{
		tax += (min(36000 + (married * 36000), taxable) - (9000 + (married * 9000))) * 0.15;
		taxable = 9000 + (married * 9000);
	}
	if(taxable > 0)
	{
		tax += (min(9000 + (married * 9000), taxable)) * 0.1;
	}
	taxes->taxes_owed = (int) tax;
	taxes->percent_of_gross_income = tax / tax_info->gross_income * 100;
	taxes->percent_of_free_income = tax / tax_info->free_income * 100;
	return 1;
}
Tax_Info* initialize_tax_info()
{
	Tax_Info* tax_info = malloc(sizeof(Tax_Info));
	if(tax_info)
	{
		tax_info->gross_income = 0;
		tax_info->free_income = 0;
		tax_info->deductions = 0;
		tax_info->allowances = 0;
		tax_info->married = 1;
		return tax_info;
	}
	return 0;
}
Taxes* initialize_taxes()
{
	Taxes* taxes = malloc(sizeof(Taxes));
	if(taxes)
	{
		taxes->taxes_owed = 0;
		taxes->percent_of_gross_income = 0;
		taxes->percent_of_free_income = 0;
		return taxes;
	}
	return 0;
}
void free_tax_info(Tax_Info* tax_info)
{
	free(tax_info);
}
void free_taxes(Taxes* taxes)
{
	free(taxes);
}
int* get_field_tax_info(Tax_Info* tax_info, int item)
{
	// tax info is null
	if (!tax_info)
		return 0;

	switch(item)
	{
		case 0: 
			return &(tax_info->gross_income);
		case 1:
			return &(tax_info->free_income);
		case 2:
			return &(tax_info->deductions);
		case 3:
			return &(tax_info->allowances);
		case 4:
			return &(tax_info->married);
		default:
			return 0;
	}
}