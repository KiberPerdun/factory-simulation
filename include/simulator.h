//
// Created by KiberPerdun on 11.05.2026.
//

#ifndef FACTORY_SIMULATION_SIMULATOR_H
#define FACTORY_SIMULATION_SIMULATOR_H

#include <cstdint>
#include <deque>
#include <queue>
#include <string>
#include <vector>

enum class MsgType
{
  FINISH = 1,
  READY = 2,
  WAIT = 3,
  START = 4,
  STOP = 5
};

struct LogEntry
{
  int64_t time;
  MsgType type;
  std::string text;
  int64_t order;

  bool
  operator< (const LogEntry &other) const
  {
    if (time != other.time)
      return time < other.time;

    if (type != other.type)
      return type < other.type;

    return order < other.order;
  }
};

struct Product
{
  int64_t id;
  int64_t cur_type;
  int64_t start;
};

struct Machine
{
  int64_t id;
  int64_t busy_time = 0;
  /*
   * Идея в том что бы в момент получения данных из входного файла по каждому
   * станку сформировать сколько в общем будет обрабатываться очередь деталей
   * которая передана, это позволит нам избавиться от необходимости пересчета
   * очереди деталей каждый раз при извлечении и добавлении деталей
   */
  int64_t queue_work_time = 0;
  std::deque<int64_t> queue;
};

enum class EventType
{
  FINISH = 0,
  START = 1
};

struct Event
{
  int64_t time;
  EventType type;
  int64_t machine_id;
  int64_t item_id;

  bool
  operator> (const Event &other) const
  {
    if (time != other.time)
      return time > other.time;

    if (type != other.type)
      return type > other.type;

    return machine_id > other.machine_id;
  }
};

class FactorySimulator
{
  private:
      int64_t M, N;
  std::vector<std::vector<int64_t>> times;
  std::vector<Machine> workers;
  std::vector<Product> items;
  std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events;
  std::vector<LogEntry> logs;
  int64_t log_counter = 0;

  int64_t cur_time = 0;
  int64_t remain_items = 0;
  void parse_input (std::string &filename);
  void handle_start (const Event &e);
  void handle_finish (const Event &e);
  int64_t select_best_machine (int64_t item_id);

  public:
  void run (std::string &filename);
};

#endif // FACTORY_SIMULATION_SIMULATOR_H
