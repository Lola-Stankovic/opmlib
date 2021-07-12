/**
 * @file InfoManager.hpp
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef OPMONLIB_INCLUDE_OPMONLIB_INFOMANAGER_HPP_
#define OPMONLIB_INCLUDE_OPMONLIB_INFOMANAGER_HPP_

#include "opmonlib/InfoProvider.hpp"
#include "opmonlib/OpmonService.hpp"

#include <nlohmann/json.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace dunedaq::opmonlib {

class InfoManager
{
public:
  static inline constexpr char s_parent_tag[]{ "__parent" }; // Call it "top"?

  explicit InfoManager(std::string service); // Constructor
  explicit InfoManager(dunedaq::opmonlib::OpmonService& service);
  void publish_info(int level);
  nlohmann::json gather_info(int level);
  void set_provider(opmonlib::InfoProvider& p);
  void start(uint32_t interval_sec, uint32_t level); // NOLINT(build/unsigned)
  void stop();

private:
  void run(uint32_t interval_sec, uint32_t level); // NOLINT(build/unsigned)

  mutable opmonlib::InfoProvider* m_ip = nullptr;
  std::shared_ptr<opmonlib::OpmonService> m_service;
  std::atomic<bool> m_running;
  std::thread m_thread;
};

} // namespace dunedaq::opmonlib

#endif // OPMONLIB_INCLUDE_OPMONLIB_INFOMANAGER_HPP_
