#include <iostream>

#include "include/csv.hpp"
#include "include/ini.hpp"
#include "include/datetime.hpp"
#include "include/filesystem.hpp"

#include "include/instrument.hpp"

std::map<std::string, Instrument> data;

void worker() {
  auto& instrument = data["if1903"];

  auto di = 1;
  auto ti = 5;
  instrument.matrix[di][ti].position = instrument.matrix[di][ti].bids[0].second > instrument.matrix[di][ti].asks[0].second ? 1 : 0;
}

void evaluate() {
  
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

      while (reader.read_row(localTime, exchTime, tickerName, lastPrice, openInterest, turnover, volume,
                             askPrice1, askPrice2, askPrice3, askPrice4, askPrice5,
                             askVolume1, askVolume2, askVolume3, askVolume4, askVolume5,
                             bidPrice1, bidPrice2, bidPrice3, bidPrice4, bidPrice5,
                             bidVolume1, bidVolume2, bidVolume3, bidVolume4, bidVolume5)) {

        auto& instrument = data[tickerName];
        instrument.symbol = tickerName;
        Instrument::Snapshot snapshot;
        if (instrument.matrix.size() < di + 1) {
          instrument.matrix.push_back(std::vector<Instrument::Snapshot>());
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

        auto& di = instrument.matrix.back();
        if (di.size() > 0) {
          auto diff = (snapshot.exch_time - di.back().exch_time) / 500000 - 1;
          for (auto i = 0; i < diff; ++i) {
            di.push_back(di.back());
            di.back().exch_time += 500000;
          }
        }

        di.push_back(snapshot);
      }

      ++di;
    }

    date_current = utils::datetime::next_week_day(date_current, 1);

    worker();
    evaluate();
  }

  return 0;
}