#include "lib/odb/application-odb.hxx"
#include <iostream>
#include <fstream>
#include <csignal>
#include <functional>
#include <chrono>
#include <crails/params.hpp>
#include <crails/utils/timer.hpp>
#include <crails/renderer.hpp>
#include <crails/logger.hpp>
#include <boost/filesystem.hpp>
#include "ctpl.h"
#include "app/sidekic/sidekic.hpp"

using namespace std;
using namespace boost::filesystem;
using namespace Crails;

extern map<string, function<void (Params&)> > sidetasks;

bool should_continue = true;

static void shutdown(int) { should_continue = false; }

static vector<string> get_tasks()
{
  vector<string> tasks;
  path p(Sidekic::tasks_path);
  directory_iterator it(p);
  directory_iterator end_it;

  for (; it != end_it ; ++it)
  {
    if (is_regular_file(it->path()))
      tasks.push_back(it->path().string());
  }
  return tasks;
}

static vector<string> get_commands(vector<string> tasks)
{
  vector<string> commands;

  for (auto filename : tasks)
  {
    std::ifstream file(filename.c_str());

    if (file.is_open())
    {
      string command, line;

      while (getline(file, line))
        command += line + '\n';
      file.close();
      remove(path(filename));
      commands.push_back(command);
    }
  }
  return commands;
}

static void run_tasks(ctpl::thread_pool& pool, vector<string> tasks)
{
  vector<string> commands = get_commands(tasks);
  auto timestamp = chrono::system_clock::to_time_t(chrono::system_clock::now());

  for (auto command : commands)
  {
    Params data; data.from_json(command);
    Data   sidedata = data["sidekic"];
    string type     = sidedata["type"];
    auto   runner   = sidetasks[type];

    if (sidedata["run_at"].exists() && sidedata["run_at"].as<time_t>() > timestamp)
      continue ;
    pool.push([&runner, &data, type](int id)
    {
      try
      {
        Utils::Timer timer;

        logger << Logger::Info
          << "# Running task of type " << type << " in " << id << Logger::endl;
        runner(data);
        logger << Logger::Info
          << "# Done (duration: " << timer.GetElapsedSeconds() << "s)"
          << Logger::endl;
      }
      catch (const std::exception& e)
      {
        logger << Logger::Error <<
          "[!] Catched exception: " << e.what() << Logger::endl;
      }
    });
  }
}

int main(int argc, char** argv)
{
  ctpl::thread_pool taskpool(2);

  Renderer::initialize();
  signal(SIGINT, &shutdown);
  do {
    run_tasks(taskpool, get_tasks());
    sleep(5);
  } while (should_continue == true);
}
