# 運動攝影機影片檔一鍵上時間字幕

## License

GPL v3

## Introduction

你的運動攝影機沒有時間戳記，感到很困擾嗎？

沒關係，只要你的影片檔案保留正確的最後修改資料，就可以一鍵上時間字幕

## Requests

- Linux OS

- cmake

- make

- g++

- opencv c++ library

## Usage

```bash
cmake .
make
./srt_subtitle <your_video_path> <another_video_path> ...
```

可以用 wildcard `*`，例如 `*.MP4` 來一次性輸入多個影片檔路徑



