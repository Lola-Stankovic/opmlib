/**
 * @file InfoManager.cpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "opmonlib/InfoManager.hpp"

#include "opmonlib/InfoCollector.hpp"
#include "opmonlib/OpmonService.hpp"

#include <iostream>
#include <string>

using namespace dunedaq::opmonlib;
using namespace std;

InfoManager::InfoManager(std::string service)
{
  m_service = opmonlib::makeOpmonService(service);
  m_running.store(false);
}

void
InfoManager::publish_info(int level)
{

  nlohmann::json j = gather_info(level);

  m_service->publish(j);
}

nlohmann::json
InfoManager::gather_info(int level)
{

  nlohmann::json j_info, j_parent;
  dunedaq::opmonlib::InfoCollector ic;
  // FIXME: check against nullptr!
  m_ip->gather_stats(ic, level);
  j_info = ic.get_collected_infos();

  j_parent[s_parent_tag] = {};
  j_parent[s_parent_tag].swap(j_info[dunedaq::opmonlib::InfoCollector::s_children_tag]);

  return j_parent;
}

void
InfoManager::set_provider(opmonlib::InfoProvider& p)
{

  // Set the data member to point to selected InfoProvider
  m_ip = &p;
}

void
InfoManager::start(uint32_t interval_sec, uint32_t level) // NOLINT(build/unsigned)
{
  m_running.store(true);
  if (interval_sec > 0)
    m_thread = std::thread(&InfoManager::run, this, interval_sec, level);
}

void
InfoManager::run(uint32_t interval_sec, uint32_t level) // NOLINT(build/unsigned)
{
  uint32_t countdown = 10 * interval_sec; // NOLINT(build/unsigned)
  while (m_running.load()) {
    if (countdown > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      --countdown;
    } else {
      publish_info(level);
      countdown = 10 * interval_sec;
    }
  }
}

void
InfoManager::stop()
{
  m_running.store(false);
  m_thread.join();
}
