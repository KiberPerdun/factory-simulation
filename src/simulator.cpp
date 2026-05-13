//
// Created by KiberPerdun on 11.05.2026.
//

#include "simulator.h"
#include <algorithm>
#include <iostream>

void
FactorySimulator::run (std::string &filename)
{
  parse_input (filename);

  for (int64_t i = 0; i < N; ++i)
    if (!workers[i].queue.empty ())
      events.push ({ 0, EventType::START, i, workers[i].queue.front () });

  for (; !events.empty () && remain_items < items.size ();)
    {
      Event e = events.top ();
      events.pop ();
      cur_time = e.time;

      switch (e.type)
        {
        case EventType::START:
          handle_start (e);
          break;
        case EventType::FINISH:
          handle_finish (e);
          break;
        }
    }

  logs.push_back ({ cur_time, MsgType::STOP,
                    "stop " + std::to_string (cur_time), log_counter++ });

  std::sort (logs.begin (), logs.end ());

  for (const auto &log : logs)
    {
      std::cout << log.text << "\n";
    }
}

void
FactorySimulator::handle_start (const Event &e)
{
  Machine &m = workers[e.machine_id];
  Product &item = items[e.item_id];

  m.queue.pop_front ();
  m.queue_work_time -= times[item.cur_type][m.id];

  std::string msg
      = "start " + std::to_string (cur_time) + " " + std::to_string (item.id)
        + " " + std::to_string (item.cur_type) + " " + std::to_string (m.id);
  logs.push_back ({ cur_time, MsgType::START, msg, log_counter++ });

  int64_t duration = times[item.cur_type][m.id];
  m.busy_time = cur_time + duration;

  events.push ({ m.busy_time, EventType::FINISH, m.id, item.id });
}

void
FactorySimulator::handle_finish (const Event &e)
{
  Machine &m = workers[e.machine_id];
  Product &item = items[e.item_id];

  std::string f_msg
      = "finish " + std::to_string (cur_time) + " " + std::to_string (item.id)
        + " " + std::to_string (item.cur_type) + " " + std::to_string (m.id);
  logs.push_back ({ cur_time, MsgType::FINISH, f_msg, log_counter++ });

  item.cur_type++;

  if (item.cur_type == M - 1)
    {
      std::string r_msg = "ready " + std::to_string (cur_time) + " "
                          + std::to_string (item.id) + " "
                          + std::to_string (m.id);
      logs.push_back ({ cur_time, MsgType::READY, r_msg, log_counter++ });
      remain_items++;
    }
  else
    {
      int64_t best_j = select_best_machine (item.id);
      Machine &target_m = workers[best_j];

      bool is_idle
          = (target_m.busy_time <= cur_time && target_m.queue.empty ());

      if (!is_idle)
        {
          std::string w_msg = "wait " + std::to_string (cur_time) + " "
                              + std::to_string (item.id) + " "
                              + std::to_string (item.cur_type) + " "
                              + std::to_string (target_m.id) + " "
                              + std::to_string (target_m.queue.size ());
          logs.push_back ({ cur_time, MsgType::WAIT, w_msg, log_counter++ });
        }

      target_m.queue.push_back (item.id);
      target_m.queue_work_time += times[item.cur_type][target_m.id];

      if (is_idle && target_m.id != m.id)
        {
          events.push ({ cur_time, EventType::START, target_m.id, item.id });
        }
    }

  if (!m.queue.empty ())
    {
      events.push ({ cur_time, EventType::START, m.id, m.queue.front () });
    }
}

int64_t
FactorySimulator::select_best_machine (int64_t item_id)
{
  int64_t best_machine = -1;
  int64_t min_wait_time = INT64_MAX;

  for (int64_t i = 0; i < N; ++i)
    {
      int64_t wait_time = workers[i].queue_work_time;

      if (wait_time < min_wait_time)
        {
          min_wait_time = wait_time;
          best_machine = i;
        }
    }

  return best_machine;
}

void
FactorySimulator::parse_input (std::string &filename)
{
  std::cin >> M >> N;

  times.assign (M - 1, std::vector<int64_t> (N));
  workers.resize (N);

  for (int64_t i = 0; i < M - 1; ++i)
    for (int64_t j = 0; j < N; ++j)
      std::cin >> times[i][j];

  int64_t product_id_counter = 0;
  for (int64_t i = 0; i < N; ++i)
    {
      workers[i].id = i;
      int64_t q;
      std::cin >> q;
      for (int64_t j = 0; j < q; ++j)
        {
          int64_t product_type;
          std::cin >> product_type;
          Product item = { product_id_counter++, product_type, i };
          items.push_back (item);
          workers[i].queue.push_back (item.id);
          workers[i].queue_work_time += times[product_type][i];
        }
    }
}
