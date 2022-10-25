#include "console.h"
#include "cmd_sys.h"
#include "esp_console.h"

static const int CONSOLE_MAX_COMMAND_LINE_LENGTH = 256;

void console_run()
{
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  /* Prompt to be printed before each line.
   * This can be customized, made dynamic, etc.
   */
  repl_config.prompt = ">";
  repl_config.max_cmdline_length = CONSOLE_MAX_COMMAND_LINE_LENGTH;

  esp_console_register_help_command();
  register_system();

  esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
