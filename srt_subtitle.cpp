#include <sys/stat.h>
#include <sys/types.h>

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

/**************************************
1                                   A
00:00:00,000 --> 00:00:01,000       B
2019-08-22 22:41:41                 C
***************************************/

/**************************************
gmtime(): convert time_t to tm as UTC time,
mktime(): convert tm struct to time_t
int tm_sec      秒數(0~61)
int tm_min      分鐘(0~59)
int tm_hour     小時(0~23)
int tm_mday     日期(1~31)
int tm_mon      月份(0~11，從1月算起)
int tm_year     年份(從1900年算起)
int tm_wday     星期幾(日→0、一→1、二→2、以此類推)
int tm_yday     一年中的第幾天
int tm_isdst    夏令時間旗標
timestamp = 0 -> 1970/01/01 00:00:00
-> 70/0/1 00:00:00
***************************************/

// This function will clip input filename into name only.
std::string get_absolute_path(std::string &filename) {
    std::string filepath;
    size_t found;

    filepath = std::filesystem::absolute(filename).string();  // Always get absolute path
    if (found = filepath.find_last_of('\\'), found != std::string::npos) {
        filename = filepath.substr(found + 1, filepath.size() - found - 1);
        filepath = filepath.substr(0, found + 1);
    }
    if (found = filepath.find_last_of('/'), found != std::string::npos) {
        filename = filepath.substr(found + 1, filepath.size() - found - 1);
        filepath = filepath.substr(0, found + 1);
    }
    return filepath;
}

time_t get_last_modification_time(const std::string &filedir, const std::string &filename) {
    std::string filepath;
    struct stat64 fileinfo;

    filepath = filedir + filename;
    stat64(filepath.c_str(), &fileinfo);
    return fileinfo.st_mtime;
}

double get_video_duration(const std::string &filedir, const std::string &filename) {
    std::string filepath;
    double fps, frame_cnt;

    filepath = filedir + filename;
    cv::VideoCapture video(filepath);
    if (!video.isOpened()) {
        std::cerr << "Error opening video stream or file" << std::endl;
        exit(EXIT_FAILURE);
    }
    fps = video.get(cv::CAP_PROP_FPS);
    frame_cnt = video.get(cv::CAP_PROP_FRAME_COUNT);
    video.release();
    cv::destroyAllWindows();
    return frame_cnt / fps;
}

std::string get_srt_name(const std::string &filename) {
    size_t found;

    found = filename.find_last_of('.');
    return filename.substr(0, found + 1) + "srt";
}

void write_srt(const std::string &filedir, const std::string &filename, const time_t mtime, const double duration, const time_t utc) {
    std::fstream fp;
    std::string srt_name;
    time_t timeline = 0;
    time_t timestamp = mtime + 3600 * utc - duration;
    struct tm tm_timeline, tm_timestamp;
    
    srt_name = get_srt_name(filename);
    fp.open(filedir + srt_name, std::ios::out | std::ios::trunc);
    if (!fp) {
        std::cerr << "Cannot open file " << srt_name << '\n';
        exit(EXIT_FAILURE);
    }
    gmtime_r(&timeline, &tm_timeline);
    gmtime_r(&timestamp, &tm_timestamp);
    for (time_t i = 1; i <= (time_t)duration; ++i) {
        fp << i << '\n';
        fp << std::setw(2) << std::setfill('0') << tm_timeline.tm_hour << ':'
           << std::setw(2) << std::setfill('0') << tm_timeline.tm_min << ':'
           << std::setw(2) << std::setfill('0') << tm_timeline.tm_sec << ",000 --> ";
        gmtime_r(&(++timeline), &tm_timeline);
        tm_timeline.tm_hour += (tm_timeline.tm_year - 70) * 8760 + (tm_timeline.tm_mon) * 744 + (tm_timeline.tm_mday - 1) * 24;
        fp << std::setw(2) << std::setfill('0') << tm_timeline.tm_hour << ':'
           << std::setw(2) << std::setfill('0') << tm_timeline.tm_min << ':'
           << std::setw(2) << std::setfill('0') << tm_timeline.tm_sec << ",000\n";
        fp << std::setw(4) << std::setfill('0') << tm_timestamp.tm_year + 1900 << '-'
           << std::setw(2) << std::setfill('0') << tm_timestamp.tm_mon + 1 << '-'
           << std::setw(2) << std::setfill('0') << tm_timestamp.tm_mday << ' '
           << std::setw(2) << std::setfill('0') << tm_timestamp.tm_hour << ':'
           << std::setw(2) << std::setfill('0') << tm_timestamp.tm_min << ':'
           << std::setw(2) << std::setfill('0') << tm_timestamp.tm_sec << "\n\n";
        gmtime_r(&(++timestamp), &tm_timestamp);
    }
    fp << (time_t)duration + 1 << '\n';
    fp << std::setw(2) << std::setfill('0') << tm_timeline.tm_hour << ':'
       << std::setw(2) << std::setfill('0') << tm_timeline.tm_min << ':'
       << std::setw(2) << std::setfill('0') << tm_timeline.tm_sec << ",000 --> ";
    fp << std::setw(2) << std::setfill('0') << tm_timeline.tm_hour << ':'
       << std::setw(2) << std::setfill('0') << tm_timeline.tm_min << ':'
       << std::setw(2) << std::setfill('0') << tm_timeline.tm_sec << ','
       << std::setw(3) << std::setfill('0') << (time_t)((duration - (double)(time_t)duration) * 1000) << '\n';
    fp << std::setw(4) << std::setfill('0') << tm_timestamp.tm_year + 1900 << '-'
       << std::setw(2) << std::setfill('0') << tm_timestamp.tm_mon + 1 << '-'
       << std::setw(2) << std::setfill('0') << tm_timestamp.tm_mday << ' '
       << std::setw(2) << std::setfill('0') << tm_timestamp.tm_hour << ':'
       << std::setw(2) << std::setfill('0') << tm_timestamp.tm_min << ':'
       << std::setw(2) << std::setfill('0') << tm_timestamp.tm_sec << "\n\n";
    
    fp.close();
}

int main(int argc, char **argv) {
    std::string filename;
    std::string filedir;
    std::string filepath;
    time_t mtime;
    double duration;

    if (argc == 1) {
        while (std::cout << "Please enter file name or file path.\n", std::getline(std::cin, filename)) {
            filedir = get_absolute_path(filename);
            filepath = filedir + filename;
            std::cout << "Reading: " << filedir << filename << std::endl;
            mtime = get_last_modification_time(filedir, filename);
            duration = get_video_duration(filedir, filename);
            write_srt(filedir, filename, mtime, duration, 8);
        }
    } else {
        for (int i = 1; i < argc; ++i) {
            filename = argv[i];
            filedir = get_absolute_path(filename);
            filepath = filedir + filename;
            std::cout << "Reading: " << filedir << filename << std::endl;
            mtime = get_last_modification_time(filedir, filename);
            duration = get_video_duration(filedir, filename);
            write_srt(filedir, filename, mtime, duration, 8);
        }
    }
    return 0;
}
