#include "api/worker/file_string.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace worker {
const char kEtcHostPath[] = "/etc/hosts";
const char kEtcHostsFormat[] =
    "127.0.0.1       localhost\n127.0.1.1       %d\n# The following lines are "
    "desirable for IPv6 capable hosts\n::1     ip6-localhost "
    "ip6-loopback\nfe00::0 ip6-localnet\nff00::0 ip6-mcastprefix\nff02::1 "
    "ip6-allnodes\nff02::2 ip6-allrouters\n";

const char kMPath[] = "mpath";  // merge path
const char kContainerId[] = "ContainerId";
const char kId[] = "Id";
const char kName[] = "Name";
const char kContainerRoot[] = "ContainerRoot";
const char kPids[] = "Pids";

const char kImage[] = "Image";
const char kCommand[] = "Command";
const char kCreated[] = "Creaeted";
const char kStatus[] = "Status";
const char kType[] = "Type";
const char kPorts[] = "Ports";

bool write_file(const char* file, const char* data, int size) {
  if (!file || !data)
    return false;
  int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC);
  if (fd == -1) {
    perror("open file");
    return false;
  }
  if (write(fd, data, size) == -1) {
    perror("write file");
    return false;
  }
  close(fd);
  return true;
}
}  // namespace worker