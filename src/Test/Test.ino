#include <scpiparser.h>
#include <Arduino.h>

struct scpi_parser_context ctx;

float frequency;

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t get_frequency(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t set_frequency(struct scpi_parser_context* context, struct scpi_token* command);


void setup()
{

  /* First, initialise the parser. */
  scpi_init(&ctx);

  /*
   * After initialising the parser, we set up the command tree.  Ours is
   *
   *  *IDN?         -> identify
   *  :SOURCE
   *    :FREQuency  -> set_frequency
   *    :FREQuency? -> get_frequency
   */
  scpi_register_command(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", 5, "*IDN?", 5, identify);

  Serial.begin(9600);
}

void loop()
{
  char line_buffer[256];
  unsigned char read_length;

  while(1)
  {
    /* Read in a line and execute it. */
    read_length = Serial.readBytesUntil('\n', line_buffer, 256);
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

  Serial.println("OIC,Signal Generator,1,10");
  return SCPI_SUCCESS;
}
