#include <vector>
#include <string>
#include <filesystem>

using std::vector;
using std::string;

int mount_host_root();
int mount_cgroup_fs();
int mount_dev_fs();
int mount_proc_fs();
int mount_host_path(vector<string> mount_paths, std::filesystem::path rootfs_path);
