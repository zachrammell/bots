#include <VNgine/helper.h>

namespace fs = std::filesystem;

std::size_t number_of_files_in_directory(fs::path const& path)
{
  using fp = bool (*)(const fs::path&);
  return std::count_if(fs::directory_iterator{ path }, fs::directory_iterator{}, fp(fs::is_regular_file));
}