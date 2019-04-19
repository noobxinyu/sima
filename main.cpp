#include <iostream>

#include "include/csv.hpp"
#include "include/ini.hpp"
#include "include/datetime.hpp"
#include "include/filesystem.hpp"

#include "include/instrument.hpp"

std::map<std::string, Instrument> data;
std::map<std::string, int> ti_map;

void evaluate_y(Instrument::Matrix& matrix, int di, int y_type, int interval) {
  std::vector<double> y(28800);
  for(int ti = 0; ti < 28800; ti++) {
    if (ti + interval >= 28800 || matrix[di][ti].mid() == 0)
    {
      matrix[di][ti].y[y_type] = 0;
    } else {
      matrix[di][ti].y[y_type] = matrix[di][ti].position * double(matrix[di][ti + interval].mid() - matrix[di][ti].mid()) / matrix[di][ti].mid();
    }
  }
}

void worker() {
  auto& instrument = data["if1903"];
  auto di = 0;
  auto ti = 0;
  /*auto& instrument = data["if1903"];

  auto di = 1;
  auto ti = 5;
  instrument.matrix[di][ti].position =
      instrument.matrix[di][ti].bids[0].second > instrument.matrix[di][ti].asks[0].second ? 1 : 0;*/

  evaluate_y(instrument.matrix, di, 0, 120); // 1min
  evaluate_y(instrument.matrix, di, 1, 600); // 5min
  evaluate_y(instrument.matrix, di, 2, 1200); // 10min
}

int main() {
  // read files
  utils::config::Parser parser("config.ini");
  auto& top = parser.top();

  auto date_start = std::stoi(top.value("start", "20181206"));
  auto date_end = std::stoi(top.value("end", "20181206"));
  auto path = top.value("path", "/home/xinyu/index_future/%Y%m%d.csv");

  auto date_current = date_start;
  auto di = 0;
  while (date_current <= date_end) {
    auto file_path = utils::datetime::strftime(path, date_current);
    std::cout << file_path << std::endl;
    if (utils::filesystem::exist(file_path)) {
      io::CSVReader<27> reader(file_path);
      reader.read_header(io::ignore_extra_column,
                         "localTime", "exchTime", "tickerName", "lastPrice", "openInterest",
                         "turnover", "volume",
                         "askPrice1", "askPrice2", "askPrice3", "askPrice4", "askPrice5",
                         "askVolume1", "askVolume2", "askVolume3", "askVolume4", "askVolume5",
                         "bidPrice1", "bidPrice2", "bidPrice3", "bidPrice4", "bidPrice5",
                         "bidVolume1", "bidVolume2", "bidVolume3", "bidVolume4", "bidVolume5");

      double bidPrice1;
      double bidPrice2;
      double bidPrice3;
      double bidPrice4;
      double bidPrice5;
      double bidVolume1;
      double bidVolume2;
      double bidVolume3;
      double bidVolume4;
      double bidVolume5;
      double askPrice1;
      double askPrice2;
      double askPrice3;
      double askPrice4;
      double askPrice5;
      double askVolume1;
      double askVolume2;
      double askVolume3;
      double askVolume4;
      double askVolume5;

      std::string tickerName;
      int64_t localTime; // hhmmssMMMMMM
      int64_t exchTime;
      double lastPrice;
      double openInterest;
      double turnover;
      double volume;

      auto base = 34200000L;

      while (reader.read_row(localTime, exchTime, tickerName, lastPrice, openInterest, turnover, volume,
                             askPrice1, askPrice2, askPrice3, askPrice4, askPrice5,
                             askVolume1, askVolume2, askVolume3, askVolume4, askVolume5,
                             bidPrice1, bidPrice2, bidPrice3, bidPrice4, bidPrice5,
                             bidVolume1, bidVolume2, bidVolume3, bidVolume4, bidVolume5)) {

        auto& instrument = data[tickerName];
        auto& last_ti = ti_map[tickerName];
        instrument.symbol = tickerName;
        Instrument::Snapshot snapshot;
        if (instrument.matrix.size() < di + 1) {
          instrument.matrix.push_back(std::vector<Instrument::Snapshot>(28800));
          last_ti = 0;
        }

        snapshot.local_time = localTime;
        snapshot.exch_time = exchTime;
        snapshot.last = lastPrice * 10000LL;
        snapshot.open_interest = openInterest;
        snapshot.volume = volume;
        snapshot.value = turnover * 10000LL;
        snapshot.bids[0] = std::make_pair(bidPrice1 * 10000L, bidVolume1);
        snapshot.bids[1] = std::make_pair(bidPrice2 * 10000L, bidVolume2);
        snapshot.bids[2] = std::make_pair(bidPrice3 * 10000L, bidVolume3);
        snapshot.bids[3] = std::make_pair(bidPrice4 * 10000L, bidVolume4);
        snapshot.bids[4] = std::make_pair(bidPrice5 * 10000L, bidVolume5);
        snapshot.asks[0] = std::make_pair(askPrice1 * 10000L, askVolume1);
        snapshot.asks[1] = std::make_pair(askPrice2 * 10000L, askVolume2);
        snapshot.asks[2] = std::make_pair(askPrice3 * 10000L, askVolume3);
        snapshot.asks[3] = std::make_pair(askPrice4 * 10000L, askVolume4);
        snapshot.asks[4] = std::make_pair(askPrice5 * 10000L, askVolume5);

        auto hh = snapshot.exch_time / 10000000000;
        auto mm = snapshot.exch_time / 100000000 % 100;
        auto ss = snapshot.exch_time / 1000000 % 100;
        auto mmm = snapshot.exch_time / 1000 % 1000;
        auto tm = hh * 3600000 + mm * 60000 + ss * 1000 + mmm;
        bool valid = false;
        if (tm >= base) {
          if (tm >= 41400000) {
            tm -= 5400000;
            if (tm >= 41400000 && tm <= 48600000) {
              valid = true;
            }
          } else {
            valid = true;
          }
        }

        if (valid) {
          auto ti = (tm - base) / 500;
          auto& series = instrument.matrix.back();
          series[ti] = snapshot;
          // fill empty
          for (auto i = last_ti + 1; i < ti; ++i) {
            series[i] = series[last_ti];
          }

          last_ti = ti;
        }
      }

      ++di;
    }

    date_current = utils::datetime::next_week_day(date_current, 1);

    worker();
  }

  return 0;
}