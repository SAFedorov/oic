#include <scpiparser.h>
#include <Arduino.h>

#define COM_TERMINATOR  '\n'
#define COM_BAUD_RATE   9600
#define COM_BUFF_SIZE   256
#define COM_TIMEOUT     50 // ms


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
  *  :PRESsure?     -> read pressure
  *  :VALVe         -> set valve status 0/1
  */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, &identify);

  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "PRESSURE?", 9, "PRES?", 5, &get_pressure);

  scpi_register_command(ctx.command_tree, SCPI_CL_CHILD, "VALVE", 5, "VALV", 4, &set_valve);

  Serial1.setTimeout(COM_TIMEOUT);
  Serial1.begin(COM_BAUD_RATE);
}

void loop()
{
  char line_buffer[COM_BUFF_SIZE];
  unsigned char read_length;

  while(1)
  {
    /* Read in a line and execute it. */
    read_length = Serial1.readBytesUntil(COM_TERMINATOR, line_buffer, COM_BUFF_SIZE);
    scpi_execute(&ctx, line_buffer, read_length, &commf, COM_TERMINATOR);
  }
}

/*
 * Communicate using serial port
 */
void
commf(char* str, int length)
{
  Serial1.write((const uint8_t*)str, length);
}

/*
 * Respond to *IDN?
 */
struct scpi_response* 
identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_response* resp;

  resp = get_empty_response(13);
  strcpy(resp->str, "IDN response");
  resp->length--; // discard the EOS character
  
  return resp;
}

/*
 * Respond to :PRESSURE?
 */
struct scpi_response* 
get_pressure(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_response* resp;
  int p = 41;

  resp = get_empty_response(20);
  resp->length = sprintf(resp->str, "Pressure = %i", 41);
  
  return resp;
}

/*
 * Respond to :VALVE
 */
struct scpi_response* 
set_valve(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_response* resp;
  struct scpi_token* arg;
  struct scpi_numeric out_numeric;
  
  arg = command;
  while(arg != NULL && arg->type != SCPI_CT_ARG)
  {
    arg = arg->next;
  }
  
  resp = get_empty_response(20);
  
  if(arg != NULL)
  {
    out_numeric = scpi_parse_numeric(arg->value, arg->length, 0, 0, 1);
    resp->length=sprintf(resp->str, "numeric=%i", (int)out_numeric.value);
  }
  else
  {
    scpi_error error;
    
    resp->length=sprintf(resp->str, "%s", "no numeric");
    error.id = -1;
    error.description = (char*)malloc(29*sizeof(char));
    error.length = sprintf(error.description, "%s", "Command invalid: no numeric");
    
    scpi_queue_error(&ctx, error);
  }

  return resp;
}
