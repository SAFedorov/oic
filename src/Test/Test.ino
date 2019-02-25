#include <scpiparser.h>
#include <Arduino.h>

#define COM_TERMINATOR '\n'
#define COM_BAUD_RATE 9600
#define COM_BUFF_SIZE 256


// Global variables
struct scpi_parser_context ctx;


// Function prototypes
struct scpi_response* identify(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_pressure(struct scpi_parser_context* context, struct scpi_token* command);


void setup()
{

  /* First, initialise the parser. */
  scpi_init(&ctx);

  /*
  * Set up the command tree. Since there are few commands 
  * 
  *  *IDN?          -> Identify
  *  :VALVE<i>      -> Valve<i> open/closed
  *  :RECirculator  -> Recirculator on/off
  *  :COOLer        -> Cryocooler on/off
  *  :PRESsure?     -> read pressure
  */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);

  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE1?", 7, "VALVE1?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE2?", 7, "VALVE2?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE3?", 7, "VALVE3?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE4?", 7, "VALVE4?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE5?", 7, "VALVE5?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE6?", 7, "VALVE6?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "VALVE7?", 7, "VALVE7?", 7, NULL);
  
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "RECIRCULATOR?", 13, "REC?", 4, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "RECIRCULATOR", 12, "REC", 3, NULL);
  
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "COOLER?", 7, "COOL?", 5, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "COOLER", 6, "COOL", 4, NULL);

  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "PRESSURE?", 9, "PRES?", 5, get_pressure);

  Serial1.begin(COM_BAUD_RATE);
}

void loop()
{
  char line_buffer[COM_BUFF_SIZE];
  unsigned char read_length;
  struct scpi_response* response;
  struct scpi_response* tmp_response;

  /*debug*/
  int debug_cnt;

  while(1)
  {
    /* Read in a line and execute it. */
    read_length = Serial1.readBytesUntil(COM_TERMINATOR, line_buffer, COM_BUFF_SIZE);
    if(read_length > 0)
    {
      response = scpi_execute(&ctx, line_buffer, read_length);

      /*debug
      Serial1.print("scpi_execute finished--");
      tmp_response = response;
      debug_cnt=0;
      while(tmp_response != NULL)
      {
        debug_cnt++;
        tmp_response = tmp_response->next;
      }
      Serial1.print("responses received:--");
      Serial1.print(debug_cnt);
      Serial1.print("--");
      end debug*/

      /* Print response string to the serial port*/
      tmp_response = response;
      while(tmp_response != NULL)
      {
        Serial1.write((const uint8_t*)tmp_response->str, tmp_response->length);
        if(tmp_response->next != NULL)
        {
          Serial1.print(';');
        }
        else
        {
          Serial1.print(COM_TERMINATOR);
        }
        tmp_response = tmp_response->next;
      }
      
      scpi_free_responses(response);
    }
  }
}


/*
 * Respond to *IDN?
 */
struct scpi_response* identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_response* resp;
  char resp_str[13] = "IDN response";
  
  scpi_free_tokens(command);

  resp = get_empty_response();
  resp->str = (char *)malloc(sizeof(resp_str));
  strcpy(resp->str, resp_str);
  resp->length = sizeof(resp_str)/sizeof(char);
  
  return resp;
}

/*
 * Respond to PRESSURE?
 */
struct scpi_response* get_pressure(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_response* resp;
  char resp_str[18] = "PRESSURE response";
  
  scpi_free_tokens(command);

  resp = get_empty_response();
  resp->str = (char *)malloc(sizeof(resp_str));
  strcpy(resp->str, resp_str);
  resp->length = sizeof(resp_str)/sizeof(char);
  
  return resp;
}
