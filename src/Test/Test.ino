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

struct scpi_response* get_valve1(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_valve2(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_valve3(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_valve4(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_valve5(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_valve7(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_cooler(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* get_recirculator(struct scpi_parser_context* context, struct scpi_token* command);

struct scpi_response* set_valve1(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_valve2(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_valve3(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_valve4(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_valve5(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_valve7(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_cooler(struct scpi_parser_context* context, struct scpi_token* command);
struct scpi_response* set_recirculator(struct scpi_parser_context* context, struct scpi_token* command);

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

  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE1?", 7, "VALVE1?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE2?", 7, "VALVE2?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE3?", 7, "VALVE3?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE4?", 7, "VALVE4?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE5?", 7, "VALVE5?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE6?", 7, "VALVE6?", 7, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE7?", 7, "VALVE7?", 7, NULL);
  
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "RECIRCULATOR?", 13, "REC?", 4, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "RECIRCULATOR", 12, "REC", 3, NULL);
  
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "COOLER?", 7, "COOL?", 5, NULL);
  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "COOLER", 6, "COOL", 4, NULL);

  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "PRESSURE?", 9, "PRES?", 5, get_pressure);

  Serial1.begin(COM_BAUD_RATE);
}

void loop()
{
  char line_buffer[COM_BUFF_SIZE];
  unsigned char read_length;
  struct scpi_response* response;
  struct scpi_response* tmp_response;

  int resp_cnt;

  while(1)
  {
    /* Read in a line and execute it. */
    read_length = Serial1.readBytesUntil(COM_TERMINATOR, line_buffer, COM_BUFF_SIZE);
    if(read_length > 0)
    {
      response = scpi_execute(&ctx, line_buffer, read_length);

      /*debug
      Serial1.print("scpi_execute finished--");
      Serial1.print("--");
      end debug*/

      /* Count non-empty responses*/
      tmp_response = response;
      resp_cnt=0;
      while(tmp_response != NULL)
      {
        if(tmp_response->length>0)
        {
          resp_cnt++;
        }
        tmp_response = tmp_response->next;
      }

      if(resp_cnt>0)
      {
        /* Print response strings to the serial port*/
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
  
  scpi_free_tokens(command);

  resp = get_empty_response(13);
  strcpy(resp->str, "IDN response");
  resp->length--; // discard the EOS character
  
  return resp;
}

/*
 * Respond to :PRESSURE?
 */
struct scpi_response* get_pressure(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_response* resp;
  
  scpi_free_tokens(command);

  resp = get_empty_response(18);
  strcpy(resp->str, "PRESSURE response");
  resp->length--; // discard the EOS character
  
  return resp;
}

/*
 * Respond to VALVE?
 */
