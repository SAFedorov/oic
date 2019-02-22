#include <scpiparser.h>
#include <Arduino.h>

#define COM_TERMINATOR '\n'
#define COM_BAUD_RATE 9600
#define COM_BUFF_SIZE 256


// Global variables
struct scpi_parser_context ctx;


// Function prototypes
scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_pressure(struct scpi_parser_context* context, struct scpi_token* command);


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

  while(1)
  {
    /* Read in a line and execute it. */
    read_length = Serial1.readBytesUntil(COM_TERMINATOR, line_buffer, COM_BUFF_SIZE);
    if(read_length > 0)
    {
      scpi_execute_command(&ctx, line_buffer, read_length);
    }
  }
}


/*
 * Respond to *IDN?
 */
scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  scpi_free_tokens(command);

  Serial1.println("OIC,Test IDN");
  return SCPI_SUCCESS;
}

/*
 * Respond to PRESSURE?
 */
scpi_error_t get_pressure(struct scpi_parser_context* context, struct scpi_token* command)
{

  Serial1.print("get_pressure ");
  Serial1.println(110.123, 2);

  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}
