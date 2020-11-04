#include <iostream>
#include <string>
#include <map>
#include <atomic>
#include <functional>
#include <any>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
#include <thread>
#include <future>
#include <set>
#include <chrono>

#include "MetricRegistry.hpp"
#include "MetricMonitor.hpp"

using namespace std;

MetricMonitor::MetricMonitor(const std::string& influxdbUri, const std::string& databaseName,
                             int portNumber, const std::string& appName, const std::string& hostName,
                             const long unsigned int numThreads, int rateLimiter)
{
  should_run=true;
  rate=rateLimiter;
  number_of_threads=numThreads;
  host_name=hostName ;
  application_name=appName;
  metric_publish = MetricPublish();
  metric_publish.setPublisher(portNumber, databaseName, influxdbUri);
}

MetricMonitor::~MetricMonitor()
{
}

template <typename T> void 
MetricMonitor::registerMetric(const std::string& metricName, std::reference_wrapper<T> myMetric)
{
  MetricRegistry::getInstance().registerMetric<T>(metricName,  myMetric);
}

void
MetricMonitor::unregisterMetric(const std::string& metricName)
{  
  MetricRegistry::getInstance().unregisterMetric(metricName);
}

template <typename T> void
MetricMonitor::getValueOfMetric(const std::string& metricName)
{
  MetricRegistry::getInstance().getValueOfMetric<T>(metricName);
}

double
MetricMonitor::getValue(std::shared_ptr<MetricRefInterface> ref)
{
  if((*ref).getTypeName()==typeid(std::atomic<float>).name()) {
    std::reference_wrapper<std::atomic<float>> value = 
      dynamic_cast<MetricRef<std::atomic<float>>&>(*ref).getValue(); 
    return (double) value.get();
  } else if((*ref).getTypeName()==typeid(std::atomic<int>).name()) { 
    std::reference_wrapper<std::atomic<int>> value =
      dynamic_cast<MetricRef<std::atomic<int>>&>(*ref).getValue(); 
    return (double) value.get();
  } else if((*ref).getTypeName()==typeid(std::atomic<double>).name()) { 
    std::reference_wrapper<std::atomic<double>> value =
      dynamic_cast<MetricRef<std::atomic<double>>&>(*ref).getValue(); 
    return (double) value.get();
  } else if((*ref).getTypeName()==typeid(std::atomic<size_t>).name()) {
    std::reference_wrapper<std::atomic<size_t>> value =
      dynamic_cast<MetricRef<std::atomic<size_t>>&>(*ref).getValue(); 
    return (double) value.get();
  }
}

void
MetricMonitor::publishMetrics(std::map<std::string, std::shared_ptr<MetricRefInterface>> metrics)
{
  map<string, shared_ptr<MetricRef<std::any>>> *castVersion =
    reinterpret_cast<map<string, shared_ptr<MetricRef<std::any>>>*>(&metrics);
  
  /*for(auto&&kv : *castVersion) 
  {
    cout << kv.first << ": " <<  ", " << std::endl;
  }*/

  for(std::map<std::string, std::shared_ptr<MetricRefInterface>>::iterator itr =
    metrics.begin(), itr_end = metrics.end(); itr != itr_end; ++itr)
  {
    std::string metric_name= itr->first;
    double metric_value=0;
    // cout<< (*itr->second).getTypeName()<<'\n';
    metric_value=getValue(itr->second);
    std::cout<< "Metric name:" << metric_name << "\n";
    std::cout<< "Metric value:" << metric_value << "\n";
    metric_publish.publishMetric(metric_name, application_name, host_name,  metric_value);
    //metric_publish.publishMetricByHTTP_Request(metric_name, application_name, host_name,  metric_value);
    //metric_publish.ccm_publishMetric("testament", application_name, host_name,  metric_value);
  }
}

double
MetricMonitor::publishThread()
{    
  std::cout << "Going to sleep 1s...\n";
  std::this_thread::sleep_for(1s); 
  try {  
    auto start = std::chrono::high_resolution_clock::now();        
    //publish the metric
  std::cout << "The thread published the metrics " << "\n";
 // publish_metrics(registry.getMetrics()); //get all metrics and publish them
   publishMetrics(MetricRegistry::getInstance().getMetrics());
 

  auto end = std::chrono::high_resolution_clock::now();
  double thread_execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();   
  return thread_execution_time ;
  }
  catch (std::exception e) {
    std::cout << "Couldn't publish metrics: " << e.what() << '\n';
  }



}

void
MetricMonitor::stop() 
{
  should_run=false;
}



void
MetricMonitor::monitor() 
{
  for (uint64_t j=0; j< number_of_threads ;j++)
  {
    std::cout << "Creating new thread \n" ;
    threads.emplace_back(&MetricMonitor::publishThread, this);
  }

  for (auto& thread : threads) thread.join();

  std::cout<<"### Finished monitoring \n" ;
  std::cout << "\n================ Statistics ================\n";
  std::cout << "Number of threads: " << number_of_threads << "\n" ; 
  for (auto &f: threads)
  {
      //auto s = f.get();
     // std::cout << "Execution time [ms]: " << s << "\n";
     
  }

  std::cout << "\n============================================\n";

}









                                                                     