#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <print>
#include <ranges>
#include <set>
#include <string>
#include <vector>

typedef int64_t CRC;
typedef int FileId;
typedef int BlockId;

CRC calc_crc(const std::vector<FileId>& block_mapping) {
  CRC crc{0};
  for (int blkId = 0; blkId < block_mapping.size(); ++blkId) {
    if (block_mapping[blkId] == -1) {
      continue;
    }
    crc += blkId * block_mapping[blkId];
  }
  return crc;
}

struct EmptyBlock {
  struct InBin {
    std::list<std::shared_ptr<EmptyBlock>> *lst;
    std::list<std::shared_ptr<EmptyBlock>>::iterator it;
    int size;
  };

  BlockId start;
  int size;
  std::list<InBin> bins;

  EmptyBlock(BlockId start, int size) : start(start), size(size), bins{} {};
};

constexpr size_t MAX_SPAN_SIZE = 10;

void part2(const std::string& input) {
  assert(input.size() % 2 == 1);

  std::vector<std::list<std::shared_ptr<EmptyBlock>>> emptyBins(MAX_SPAN_SIZE + 1);

  std::vector<std::tuple<FileId, BlockId, int>> files{};

  auto it = input.begin();
  FileId currentFileId{0};
  BlockId currentBlockId{0};
  int currentSpanSize{*it - '0'};
  files.push_back(std::make_tuple(currentFileId, currentBlockId, currentSpanSize));

  currentBlockId += currentSpanSize;
  ++it;
  ++currentFileId;

  while (it != input.end()) {
    currentSpanSize = *it - '0';
    assert(currentSpanSize >= 0);
    assert(currentSpanSize <= MAX_SPAN_SIZE);
    auto empty = std::make_shared<EmptyBlock>(currentBlockId, currentSpanSize);

    for (auto span_size = currentSpanSize; span_size > 0; --span_size) {
      auto& emptyBlockList = emptyBins.at(span_size);
      auto placeInBin = emptyBlockList.insert(emptyBlockList.end(), empty);
      empty->bins.push_back(EmptyBlock::InBin{&emptyBlockList, placeInBin, span_size});
    }
    currentBlockId += currentSpanSize;
    ++it;

    currentSpanSize = *it - '0';
    assert(currentSpanSize >= 0);
    assert(currentSpanSize <= MAX_SPAN_SIZE);
    files.push_back(
                    std::make_tuple(currentFileId, currentBlockId, currentSpanSize));
    currentBlockId += currentSpanSize;
    ++currentFileId;
    ++it;
  }
  const int total_blocks = std::get<2>(files.back()) + std::get<1>(files.back());

  for (auto& [file_id, first_block, span_size] : std::ranges::views::reverse(files)) {
    if (!emptyBins[span_size].empty()) {
      auto empty = emptyBins[span_size].front();
      if (empty->start < first_block) {
        // update file
        first_block = empty->start;

        // cleanup empty bins
        empty->start += span_size;
        empty->size -= span_size;
        auto inBinIt = empty->bins.begin();
        while (inBinIt != empty->bins.end()) {
          if (inBinIt->size > empty->size) {
            inBinIt->lst->erase(inBinIt->it);
            inBinIt = empty->bins.erase(inBinIt);
            continue;
          }
          ++inBinIt;
        }
      }
    }
  }

  std::vector<FileId> block_mapping(total_blocks, -1);
  for (auto [file_id, first_block, span_size] : files) {
    std::fill(block_mapping.begin() + first_block,
              block_mapping.begin() + first_block + span_size,
              file_id);
  }

  for (auto blockId : block_mapping) {
    if (blockId >= 0) {
      std::cout << static_cast<char>('0' + blockId % 10);
    } else {
      std::cout << ".";
    }
  }
  std::cout << "\n";
  std::println("p2 crc {}", calc_crc(block_mapping));
}


int main(int argc, char **argv) {
  std::string input;
  std::getline(std::cin, input);

  assert(input.size() % 2 == 1);

  std::vector<FileId> block_mapping_original{};

  FileId currentFile{0};
  auto it = input.begin();
  int currentFileSize = *(it++) - '0';
  while (currentFileSize > 0) {
    block_mapping_original.push_back(currentFile);
    --currentFileSize;
  }
  currentFile++;

  std::vector<std::vector<int>> empty_mapping(10);
  auto block_mapping{block_mapping_original};

  while (it < input.end()) {
    int emptySize = *it - '0';
    empty_mapping[emptySize].push_back(it - input.begin());
    while (emptySize > 0) {
      block_mapping.push_back(-1);
      --emptySize;
    }
    ++it;

    int fileSize = *(it++) - '0';
    while (fileSize > 0) {
      block_mapping.push_back(currentFile);
      --fileSize;
    }
    currentFile++;
  }

  auto emptyIt = block_mapping.begin();
  auto defragIt = block_mapping.rbegin();

  while (emptyIt < defragIt.base()) {
    if (*emptyIt >= 0) {
      emptyIt = std::find(emptyIt, block_mapping.end(), -1);
      continue;
    }
    if (*defragIt == -1) {
      defragIt = std::find_if(defragIt, block_mapping.rend(), [](auto b) { return b != -1; });
      continue;
    }
    *emptyIt = *defragIt;
    *defragIt = -1;
    ++emptyIt;
    ++defragIt;
  }
  std::println("{}", calc_crc(block_mapping));

  part2(input);
  return 0;
}
