#include "sidekic.hpp"
#include <fstream>
#include <crails/utils/string.hpp>

using namespace std;
using namespace Crails;

void Sidekic::async_task(const string& name, Data params)
{
  string filename = tasks_path + '/' +
    generate_random_string("abcdefghijklmnopqrstwxyz0123456789", 10);
  ofstream file(filename.c_str());

  if (file.is_open())
  {
    params["sidekic"]["type"] = name;
    file << params.to_json();
    file.close();
  }
  else
    throw boost_ext::runtime_error("sidekic cannot create file " + filename);
}

void Sidekic::schedule_task(time_t timestamp, const string& name, Data params)
{
  params["sidekic"]["run_at"] = timestamp;
  async_task(name, params);
}
