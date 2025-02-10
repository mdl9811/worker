#include "api/rtc/channel.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/task/sequenced_task_runner.h"

namespace worker {
Channel::Channel() = default;
Channel::~Channel() = default;
void Channel::Destruct(const Channel* processor) {
  delete processor;
}

void Channel::Dispose() {
  base::SequencedTaskRunner::GetCurrentDefault()->PostTask(
      FROM_HERE, base::BindOnce(Destruct, base::Unretained(this)));
}

}  // namespace worker