/*
Copyright (c) 2013 Lachlan Gunn
Copyright (c) 2019 Sergey Fedorov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <Arduino.h>

#include "scpiparser.h"

#ifdef __cplusplus

  extern "C" {
  
#endif

/* Case-insensitive string comparison over defined length*/
static int
strcmpil(char* str1, char* str2, int length)
{
	int i;
	int isequal;
	
	isequal = 1;
	for(i=0; i<length; i++)
	{
		if(toupper(str1[i]) != toupper(str2[i]))
		{
			isequal = 0;
			break;
		}
	}
	
	return isequal;
}


static struct scpi_response*
system_error(struct scpi_parser_context* ctx, struct scpi_token* command)
{
	struct scpi_error* error;
	struct scpi_response* response;

	response = get_empty_response(0);
	response->error_code = SCPI_SUCCESS;

	error = scpi_pop_error(ctx);
	response->str = error->description;
	response->length = error->length;

	free((void*)error);

	return response;
}

void
scpi_init(struct scpi_parser_context* ctx)
{
	struct scpi_command* system;
	struct scpi_command* error;
	
	ctx->command_tree = (struct scpi_command*)malloc(sizeof(struct scpi_command));
	
	ctx->command_tree->long_name = NULL;
	ctx->command_tree->long_name_length = 0;
	
	ctx->command_tree->short_name = NULL;
	ctx->command_tree->short_name_length = 0;
	
	ctx->command_tree->callback = NULL;
	ctx->command_tree->next = NULL;
	ctx->command_tree->children = NULL;
	
	system = scpi_register_command(
				ctx->command_tree, SCPI_CL_CHILD, "SYSTEM", 6,
												  "SYST", 4, NULL);
												  
	error = scpi_register_command(
				system, SCPI_CL_CHILD, "ERROR", 5,
									   "ERR", 3, NULL);
									   
	scpi_register_command(
				system, SCPI_CL_CHILD, "ERROR?", 6,
									   "ERR?", 4, system_error);
	
	scpi_register_command(
				error, SCPI_CL_CHILD, "NEXT?", 5, "NEXT?", 5, system_error);
	
	ctx->error_queue_head = NULL;
	ctx->error_queue_tail = NULL;
}

struct scpi_token*
scpi_parse_string(char* str, size_t length)
{
	int i;
	
	struct scpi_token* head;
	struct scpi_token* tail;
	struct scpi_token* new_tail;
	
	int token_start;
	int token_length;
	
	head = NULL;
	tail = NULL;
	token_start = 0;
	
	for(i = 0; i < length; i++)
	{
		if(str[i] == ':' || str[i] == ' ' || i == length-1)
		{
			if(i == length-1)
			{
				/*including the current character*/
				token_length = i-token_start+1;
			}
			else
			{
				/*excluding the current character*/
				token_length = i-token_start;
			}
			
			new_tail = (struct scpi_token*)malloc(sizeof(struct scpi_token));
			new_tail->type = SCPI_CT_NAME;
			new_tail->value = str+token_start;
			new_tail->length = token_length;
			new_tail->next = NULL;
						
			if(tail == NULL)
			{
				head = new_tail;
			}
			else
			{
				tail->next = new_tail;
			}
			tail = new_tail;
			
			token_start = i+1;
			
			if(str[i] == ' ')
			{
				break;
			}
		}
	}
	
	token_start = -1;
	for(i++; i < length; i++)
	{
		if(token_start == -1 && !isspace(str[i]))
		{
			token_start = i;
		}
		
		if(str[i] == ',' || i == length-1)
		{
			new_tail = (struct scpi_token*)malloc(sizeof(*new_tail));
			new_tail->type = SCPI_CT_ARG;
			new_tail->value = str+token_start;
			new_tail->length = i-token_start;
			new_tail->next = NULL;
			
			if(i == length-1)
			{
				new_tail->length++;
			}
			
			tail->next = new_tail;
			tail = new_tail;
			
			token_start = -1;
		}
	}
	
	return head;
}

struct scpi_command*
scpi_register_command(struct scpi_command* parent, scpi_command_location_t location,
						char* long_name,  size_t long_name_length,
						char* short_name, size_t short_name_length,
						command_callback_t callback)
{
	
	struct scpi_command* current_command;
	
	if(location == SCPI_CL_CHILD)
	{
		current_command = parent->children;
	}
	else
	{
		current_command = parent;
	}
	
	if(current_command == NULL)
	{
		parent->children = (struct scpi_command*)malloc(sizeof(struct scpi_command));
		current_command = parent->children;
	}
	else
	{
		while(current_command->next != NULL)
		{
			current_command = current_command->next;
		}
		
		current_command->next = (struct scpi_command*)malloc(sizeof(struct scpi_command));
		current_command = current_command->next;
	}
	
	current_command->next = NULL;
	current_command->children = NULL;
	
	current_command->long_name = long_name;
	current_command->long_name_length = long_name_length;
	
	current_command->short_name = short_name;
	current_command->short_name_length = short_name_length;
	
	current_command->callback = callback;
	
	return current_command;
}

struct scpi_command*
scpi_find_command(struct scpi_parser_context* ctx,
					struct scpi_token* parsed_string)
{
	struct scpi_command* root;
	struct scpi_token* current_token;
	struct scpi_command* current_command;
	
	root = ctx->command_tree;
	current_token = parsed_string;
	current_command = root;

	
	while(current_token != NULL && current_token->type == SCPI_CT_NAME)
	{
	
		int found_token = 0;
		while(current_command != NULL)
		{
			
			if((current_token->length == current_command->long_name_length
					&& strcmpil(current_token->value, current_command->long_name, current_token->length))
				|| (current_token->length == current_command->short_name_length
					&& strcmpil(current_token->value, current_command->short_name, current_token->length)))
			{
				/* We have found the token. */
				current_token = current_token->next;
				
				if(current_token == NULL || current_token->type != SCPI_CT_NAME)
				{
					return current_command;
				}
				else
				{
					found_token = 1;
					current_command = current_command->children;
					break;
				}
			}
			else
			{
				current_command = current_command->next;
			}
		}
		
		if(!found_token)
		{
			return NULL;
		}
	}
	
	return NULL;
}

struct scpi_response*
get_empty_response(int length)
{
	struct scpi_response* response;
	
	response = (struct scpi_response*)malloc(sizeof(struct scpi_response));
	response->error_code = SCPI_SUCCESS;
	response->next = NULL;
	
	if(length == 0)
	{
		response->str = NULL;
		response->length = 0;	
	}
	else
	{
		response->str = (char *)malloc(length*sizeof(char));
		response->length = length;
	}
	
	return response;
}

struct scpi_response*
scpi_execute_command(struct scpi_parser_context* ctx, char* command_string, size_t length)
{
	struct scpi_command* command;
	struct scpi_token* parsed_command;
	struct scpi_response* response;
	
	parsed_command = scpi_parse_string(command_string, length);
	
	command = scpi_find_command(ctx, parsed_command);
	response = NULL;
	if(command == NULL)
	{
		response = get_empty_response(0);
		response->error_code = SCPI_COMMAND_NOT_FOUND;
	}
	else if(command->callback == NULL)
	{
		response = get_empty_response(0);
		response->error_code = SCPI_NO_CALLBACK;
	}
	else
	{
		response = command->callback(ctx, parsed_command);	
	}
	
	if(response == NULL)
	{
		response = get_empty_response(0);
		response->error_code = SCPI_NO_CALLBACK_RESPONSE;
	}

	scpi_free_tokens(parsed_command);
	
	return response;
}

void
scpi_execute(struct scpi_parser_context* ctx, char* command_string, size_t length, commf_t commf, char terminator)
{
	int i;
	int start_ind;
	int cmd_length;
	struct scpi_response* head;
	struct scpi_response* tail;
	struct scpi_response* current_response;

	if(length <= 0)
	{
		return;
	}

	/* Split the command string by ';' and execute command callbacks */
	head = NULL;
	start_ind = 0;

	for(i = 0; i < length; i++)
	{
		if(command_string[i] == ';' || i == length-1)
		{
			if(i == length-1 && command_string[i] != ';')
			{
				/*including the current character*/
				cmd_length = i-start_ind+1;
			}
			else
			{
				/*excluding the current character, which is ';'*/
				cmd_length = i-start_ind;
			}
			
			current_response = scpi_execute_command(ctx, &command_string[start_ind], cmd_length);

			if(head == NULL)
			{
				head = current_response;
				tail = head;
			}
			else
			{
				tail->next = current_response;
				tail = tail->next;
			}
			
			tail->next=NULL;
			start_ind = i+1;
		}
	}

	/* Count non-empty responses and send reply if cnt>0 */
	current_response = head;
	i=0;

	while(current_response != NULL)
	{
		if(current_response->length>0)
		{
			i++;
		}
		current_response = current_response->next;
	}

	if(i>0 && commf != NULL)
	{
		char cmd_sep = ';';

		/* Communicate response strings */
		current_response = head;
		while(current_response != NULL)
		{
			commf(current_response->str, current_response->length);
			if(current_response->next != NULL)
			{
				commf(&cmd_sep, 1);
			}
			else
			{
				commf(&terminator, 1);
			}
			current_response = current_response->next;
		}
	}
	
	scpi_free_responses(head);

	return;
}

void
scpi_free_some_tokens(struct scpi_token* start, struct scpi_token* end)
{
	struct scpi_token* prev;
	while(start != NULL && start != end)
	{
		prev = start;
		start = start->next;
		
		free((void*)prev);
	}
}

void
scpi_free_tokens(struct scpi_token* start)
{
	scpi_free_some_tokens(start, NULL);
}

void
scpi_free_responses(struct scpi_response* start)
{
	struct scpi_response* prev;
	while(start != NULL)
	{
		prev = start;
		start = start->next;
		
		free((void*)prev->str);
		free((void*)prev);
	}
}

struct scpi_numeric
scpi_parse_numeric(char* str, size_t length, float default_value, float min_value, float max_value)
{
	int i;
	float mantissa;
	int state;
	int sign;
	int point_position;
	int exponent;
	int exponent_sign;
	long exponent_multiplier;
	float value;
	char* unit_start;
	char* unit_end;
	struct scpi_numeric retval;
	
	exponent = 0;
	exponent_sign = 0;
	point_position = 0;
	sign = 0;
	state = 0;
	mantissa = 0;
	exponent_multiplier = 1;
	unit_start = NULL;
	unit_end = NULL;

	for(i = 0; i < length; i++)
	{
		if(state == 0)
		{
			/* Remove leading whitespace */			
			if(isspace(str[i]))
			{
				continue;
			}
                        else if(length-i >= 7 && str[i]   == 'D' && str[i+1] == 'E' && str[i+2] == 'F'
                                              && str[i+3] == 'A' && str[i+4] == 'U' && str[i+5] == 'L'
                                              && str[i+6] == 'T')
                        {
                                /* The user has asked for the default value. */
                                retval.value = default_value;
                                retval.unit = NULL;
                                retval.length = 0;
                                
                                return retval;
                        }
                        else if(length-i >= 3 && str[i] == 'M' && str[i+1] == 'A' && str[i+2] == 'X')
                        {
                                /* The user has asked for the maximum value. */
                                retval.value = max_value;
                                retval.unit = NULL;
                                retval.length = 0;
                                return retval;
                        }
                        else if(length-i >= 3 && str[i] == 'M' && str[i+1] == 'I' && str[i+2] == 'N')
                        {
                                /* The user has asked for the minimum value. */
                                retval.value = min_value;
                                retval.unit = NULL;
                                retval.length = 0;
                                return retval;
                        }
			else if(str[i] == '+' || str[i] == '-')
			{
				/* We have hit a +/- */
				state = 1;
			}
			else if(isdigit(str[i]))
			{
				/* We have reached the number itself. */
				state = 2;
			}
			else
			{
				state = -1;
				continue;
			}
		}
		
		if(state == 1)
		{
			/* Set the sign. */
			if(str[i] == '+')
			{
				sign = 0;
			}
			else
			{
				sign = 1;
			}
			
			state = 2;
			continue;
		}
		
		if(state == 2 || state == 3)
		{
			if(isdigit(str[i]))
			{
				/* Start accumulating digits. */
				mantissa = (10*mantissa) + (float)(str[i] - 0x30);
				
				if(state == 3)
				{
					/* We are past the decimal point, so reposition it. */
					point_position++;
				}
			}
			else if(str[i] == '.')
			{
				state = 3;
				continue;
			}
			else if(str[i] == 'e')
			{
				state = 4;
				continue;
			}
			else
			{
				state = 6;
			}
		}
		
		if(state == 4)
		{
			/* We are now looking at the exponent sign. */
			if(str[i] == '+' || str[i] == '-')
			{
				if(str[i] == '-')
				{
					exponent_sign = 1;
				}
			}
			else if(isdigit(str[i]))
			{
				state = 5;
			}
			else
			{
				state = -1;
			}
		}
		
		if(state == 5)
		{
			
			if(isdigit(str[i]))
			{
				exponent = (exponent*10) + (int)(str[i] - 0x30);
				continue;
			}
			else
			{
				state = 6;
			}
		}
		
		if(state == 6)
		{
		
			/* Remove spaces between the number and its units. */
			if(isspace(str[i]))
			{
				continue;
			}
			else
			{
				state = 7;
			}
		
		}
		
		if(state == 7)
		{
			/* The unit itself---first the multiplier. */
			switch(str[i])
			{
				case 'y':
					exponent -= 24;
                                        state = 8;
                                        continue;
					break;
				case 'z':
					exponent -= 21;
					state = 8;
                                        continue;
                                        break;
				case 'a':
					exponent -= 18;
					state = 8;
                                        continue;
                                        break;
				case 'f':
					exponent -= 15;
					state = 8;
                                        continue;
                                        break;
				case 'p':
					exponent -= 12;
					state = 8;
                                        continue;
                                        break;
				case 'n':
					exponent -= 9;
					state = 8;
                                        continue;
                                        break;
				case 'u':
					exponent -= 6;
					state = 8;
                                        continue;
                                        break;
				case 'm':
					exponent -= 3;
					state = 8;
                                        continue;
                                        break;
				case 'c':
					exponent -= 2;
					state = 8;
                                        continue;
                                        break;
				case 'd':
					exponent -= 1;
					state = 8;
                                        continue;
                                        break;
				case 'D':
					exponent += 1;
					state = 8;
                                        continue;
                                        break;
				case 'C':
					exponent += 2;
					state = 8;
                                        continue;
                                        break;
				case 'k':
					exponent += 3;
					state = 8;
                                        continue;
                                        break;
				case 'M':
					exponent += 6;
					state = 8;
                                        continue;
                                        break;
				case 'G':
					exponent += 9;
					state = 8;
                                        continue;
                                        break;
				case 'T':
					exponent += 12;
					state = 8;
                                        continue;
                                        break;
				case 'P':
					exponent += 15;
					state = 8;
                                        continue;
                                        break;
				case 'E':
					exponent += 18;
					state = 8;
                                        continue;
                                        break;
				case 'Z':
					exponent += 21;
					state = 8;
                                        continue;
                                        break;
				case 'Y':
					exponent += 24;
					state = 8;
                                        continue;
                                        break;
				
				default:
				
					if(isupper(str[i]))
					{
						state = 8;
					}
					else
					{
						state = -1;
                                                continue;
					}
			}

		}
		
		if(state == 8)
		{
			/* The unit proper. */
			if(isalpha(str[i]))
			{
				if(unit_start == NULL)
				{
					unit_start = str+i;
				}
				state = 9;
			}
			else
			{
                                if(unit_start != NULL)
                                {
				        unit_end = str+i;
                                }
                                else
                                {
                                        unit_end = NULL;
                                }
			
				state = -1;
			}
		}
	}

        if(unit_start != NULL && unit_end == NULL)
        {
          unit_end = str+length-1;
        }
	
	value = (float)mantissa;
	
	if(exponent_sign != 0)
	{
		exponent = -exponent;
	}
	
	exponent -= point_position;
	if(exponent > 0)
	{
		for(i = 0; i < exponent; i++)
		{
			exponent_multiplier *= 10;
		}
		
		value *= exponent_multiplier;
	}
	else
	{
		for(i = exponent; i < 0; i++)
		{
			exponent_multiplier *= 10;
		}
		
		value /= exponent_multiplier;
	}
	
	if(sign != 0)
	{
		value = -value;
	}
	
	retval.value  = value;
	retval.unit   = unit_start;
	
	if(unit_start == NULL)
	{
		retval.length = 0;
	}
	else
	{
		retval.length = unit_end - unit_start + 1;
	}
	
	
	return retval;
}

void
scpi_queue_error(struct scpi_parser_context* ctx, struct scpi_error error)
{
	struct scpi_error* new_error;
	
	new_error = (struct scpi_error*)malloc(sizeof(struct scpi_error));
	new_error->id = error.id;
	new_error->description = error.description;
	new_error->length = error.length;
	
	new_error->next = NULL;
	
	if(ctx->error_queue_head != NULL)
	{
		ctx->error_queue_tail->next = new_error;
	}
	else
	{
		ctx->error_queue_head = new_error;
	}
	
	ctx->error_queue_tail = new_error;
}

struct scpi_error*
scpi_pop_error(struct scpi_parser_context* ctx)
{
	if(ctx->error_queue_head == NULL)
	{
		struct scpi_error* success;
		
		success = (struct scpi_error*)malloc(sizeof(struct scpi_error));
		success->id = 0;
		success->description = (char *)malloc(9*sizeof(char));
		strcpy(success->description, "No error");
		success->length = 8; // discard the EOS character
		
		return success;
	}
	else
	{
		struct scpi_error* retval;

		retval = ctx->error_queue_head;
		ctx->error_queue_head = retval->next;
		
		if(ctx->error_queue_head == NULL)
		{
			ctx->error_queue_tail = NULL;
		}
		
		return retval;
	}
}

#ifdef __cplusplus

  }
  
#endif

