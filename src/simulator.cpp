//
// Created by KiberPerdun on 11.05.2026.
//

#include "simulator.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static bool
parse_line_to_ints (const std::string &line, std::vector<int64_t> &out)
{
  std::istringstream iss (line);
  std::string token;
  while (iss >> token)
    {
      if (token.empty ())
        return false;

      size_t start = 0;
      if (token[0] == '-')
        start = 1;

      if (start == token.size ())
        return false;

      for (size_t i = start; i < token.size (); ++i)
          if (!std::isdigit (token[i]))
            return false;

      try
        {
          out.push_back (std::stoll (token));
        }
      catch (...)
        {
          return false;
        }
    }
  return true;
}

static void
error_exit (const std::string &line)
{
  std::cout << line << "\n";
  std::exit (1);
}

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

  m.is_working = true;

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

  m.is_working = false;

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

      bool is_idle = !target_m.is_working && target_m.queue.empty ();

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
  std::ifstream file (filename);
  if (!file.is_open ())
    std::exit (1);

  std::string line;
  auto get_next_line = [&] ()
  {
    if (!std::getline (file, line))
      std::exit (1);

    if (!line.empty () && line.back () == '\r')
      line.pop_back ();

    return line;
  };

  std::vector<int64_t> nums;

  line = get_next_line ();
  if (!parse_line_to_ints (line, nums) || nums.size () != 2)
    error_exit (line);

  M = nums[0];
  N = nums[1];

  if (M < 1 || M > 100 || N < 1 || N > 100)
    error_exit (line);

  times.assign (M - 1, std::vector<int64_t> (N));
  workers.resize (N);

  for (int64_t i = 0; i < M - 1; ++i)
    {
      line = get_next_line ();
      nums.clear ();
      if (!parse_line_to_ints (line, nums)
          || nums.size () != static_cast<size_t> (N))
        error_exit (line);

      for (int64_t j = 0; j < N; ++j)
        {
          if (nums[j] < 0 || nums[j] > 10000)
            error_exit (line);

          times[i][j] = nums[j];
        }
    }

  int64_t product_id_counter = 0;
  int64_t total_q = 0;

  for (int64_t i = 0; i < N; ++i)
    {
      line = get_next_line ();
      nums.clear ();
      if (!parse_line_to_ints (line, nums) || nums.empty ())
        error_exit (line);

      int64_t q = nums[0];
      if (q < 0 || nums.size () != static_cast<size_t> (q + 1))
        error_exit (line);

      total_q += q;
      if (total_q > 100000)
        error_exit (line);

      workers[i].id = i;
      for (int64_t j = 0; j < q; ++j)
        {
          int64_t product_type = nums[j + 1];

          if (product_type < 0 || product_type > M - 2)
            error_exit (line);

          Product item = { product_id_counter++, product_type, i };
          items.push_back (item);
          workers[i].queue.push_back (item.id);
          workers[i].queue_work_time += times[product_type][i];
        }
    }
}