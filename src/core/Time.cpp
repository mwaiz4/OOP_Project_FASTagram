#include "core/Time.h"

#include <ctime>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
using namespace std;

static bool get_local_time(Time::seconds_type seconds, tm& result) {
    const time_t value = static_cast<time_t>(seconds);

#ifdef _WIN32
    return localtime_s(&result, &value) == 0;
#else
    return localtime_r(&value, &result) != nullptr;
#endif
}

Time::clock_fn Time::clock_source_ = nullptr;

Time::Time(seconds_type seconds)
    : seconds_(seconds) {
    if (seconds < 0) {
        throw invalid_argument("Time cannot be negative");
    }
}

Time Time::now() {
    if (clock_source_ != nullptr) {
        return Time(clock_source_());
    }

    const time_t current_time = time(nullptr);
    if (current_time == static_cast<time_t>(-1)) {
        throw runtime_error("Could not read the system clock");
    }

    const seconds_type seconds = static_cast<seconds_type>(current_time);
    if (seconds < 0) {
        throw runtime_error("The system clock returned a negative time");
    }

    return Time(seconds);
}

void Time::set_clock(clock_fn fn) noexcept {
    clock_source_ = fn;
}

Time::seconds_type Time::seconds() const noexcept {
    return seconds_;
}

string Time::label() const {
    const Time current = Time::now();
    const seconds_type diff = current - *this;

    if (diff < 60) {
        return "Just now";
    }
    if (diff < 3600) {
        return to_string(diff / 60) + " min ago";
    }
    if (diff < 86400) {
        return to_string(diff / 3600) + " h ago";
    }

    tm created_tm{};
    tm current_tm{};
    if (!get_local_time(seconds_, created_tm) ||
        !get_local_time(current.seconds(), current_tm)) {
        throw runtime_error("Could not convert time to local calendar date");
    }

    bool is_yesterday = false;
    if (current_tm.tm_yday > 0) {
        is_yesterday = created_tm.tm_year == current_tm.tm_year &&
                       created_tm.tm_yday == current_tm.tm_yday - 1;
    } else {
        // On January 1, yesterday is December 31 of the previous year.
        is_yesterday = created_tm.tm_year == current_tm.tm_year - 1 &&
                       created_tm.tm_mon == 11 && created_tm.tm_mday == 31;
    }

    if (is_yesterday) {
        return "Yesterday";
    }

    char date_buffer[32]{};
    if (strftime(date_buffer, sizeof(date_buffer), "%d %b", &created_tm) == 0) {
        throw runtime_error("Could not format time label");
    }

    string date_label(date_buffer);
    if (date_label.size() >= 2 && date_label[0] == '0') {
        date_label.erase(0, 1);
    }
    return date_label;
}

void Time::write(ostream& out) const {
    if (seconds_ > static_cast<seconds_type>(
                       (numeric_limits<int64_t>::max)())) {
        throw runtime_error("Time value does not fit the file format");
    }

    const int64_t value = static_cast<int64_t>(seconds_);
    out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    if (!out) {
        throw runtime_error("Failed to write Time");
    }
}

void Time::read(istream& in) {
    int64_t value = 0;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    if (!in) {
        throw runtime_error("Failed to read Time");
    }
    if (value < 0) {
        throw runtime_error("File contains a negative Time value");
    }

    seconds_ = static_cast<seconds_type>(value);
}

Time::seconds_type operator-(const Time& lhs, const Time& rhs) noexcept {
    return lhs.seconds_ - rhs.seconds_;
}

bool operator<(const Time& lhs, const Time& rhs) noexcept {
    return lhs.seconds_ < rhs.seconds_;
}

bool operator>(const Time& lhs, const Time& rhs) noexcept {
    return rhs < lhs;
}

bool operator==(const Time& lhs, const Time& rhs) noexcept {
    return lhs.seconds_ == rhs.seconds_;
}

bool operator!=(const Time& lhs, const Time& rhs) noexcept {
    return !(lhs == rhs);
}

ostream& operator<<(ostream& out, const Time& time) {
    out << time.seconds_;
    return out;
}
