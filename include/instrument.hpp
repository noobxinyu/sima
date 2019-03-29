//
// Created by xinyu on 19-3-29.
//

#ifndef SIMA_INSTRUMENT_HPP
#define SIMA_INSTRUMENT_HPP

class Instrument
{
public:
  struct Snapshot{
    int64_t local_time = 0;
    int64_t exch_time = 0;
    int64_t last = 0;
    int64_t volume = 0;
    int64_t value = 0;
    int64_t open_interest = 0;
    std::array<std::pair<int64_t, int32_t>, 5> bids;
    std::array<std::pair<int64_t, int32_t>, 5> asks;

    int64_t position = 0;
  };

  using Series = std::vector<Snapshot>;
  using Matrix = std::vector<Series>;

public:
  Instrument(){}
  ~Instrument(){}

public:
  std::string symbol;

  Matrix matrix;
};

#endif //SIMA_INSTRUMENT_HPP
