#include <malloc.h>
#include <sys/types.h>
#include "base/at_exit.h"
#include "base/logging.h"
#include "base/message_loop/message_pump_type.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"

int main(int argc, const char* argv[]) {
  logging::LoggingSettings settings;
  base::AtExitManager at_exit_manager;
  base::SingleThreadTaskExecutor io_task_executor(base::MessagePumpType::IO);

  base::RunLoop().Run();
  return EXIT_SUCCESS;
}
